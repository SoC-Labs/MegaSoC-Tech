/*
 * sdboot: load the EL3 shim, the device tree and the Linux Image from the FAT32 partition of the SD card into
 * DDR4 and jump to the shim.  MegaSoC HAPS-SX, 12-17 November 2026 (MGSOC-39 follow-on: boot Linux from the card).
 * Flashed as the production boot image it gives an unattended SD-card Linux boot (HAPS-SX build 64, 17-19 Nov 2026).
 *
 * Files (8.3 names, FatFs is built without long-name support):
 *   0:/SHIM.BIN     -> 0x8000_0000  (the EL3 shim, shim_g0.bin, < 64 KB)
 *   0:/MEGASOC.DTB  -> 0x8080_0000  (megasoc-haps-sx.dtb, < 1 MB)
 *   0:/IMAGE        -> 0x8100_0000  (arm64 Image with the initramfs inside, < 64 MB)
 * The load map is the one the monitor ROM's UART path uses (linux/README.md), so the shim is entered unchanged
 * with 'x0 = DTB' semantics handled by the shim itself.
 *
 * Reads go through FatFs -> diskio.c -> sdio_read() in 512-byte, sector-aligned requests, i.e. the driver's
 * single-block PIO path (reads only, so the multi-block DMA write defects MBT-72/MBT-73 are not in play).  Measured on
 * HAPS-SX builds 63/64 at the 5 MHz data clock: 26.7 ms per sector, 336 s for the 6.46 MB Image.  Built with
 * -DSDIO_PROFILE=1 the driver splits that per block (command start / busy wait / FIFO drain) and sdboot prints the
 * averages after each file, to find where the time goes; -DSDIO_DEBUG=0 silences the per-sector driver trace.
 *
 * Before the jump the three regions are cleaned+invalidated from the data cache (the app runs with the MMU and
 * caches on, DDR4 is Normal WB; the shim invalidates the caches, which would drop dirty lines).
 */
#include <sdiodrv.h>
#include "uart_stdout.h"
#include "system.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ff.h"
#include "diskio.h"

#define SHIM_ADDR   0x80000000UL
#define DTB_ADDR    0x80800000UL
#define IMAGE_ADDR  0x81000000UL
#define SHIM_MAX    (64u * 1024u)
#define DTB_MAX     (1024u * 1024u)
#define IMAGE_MAX   (64u * 1024u * 1024u)
#define CNT_HZ      20000000u

static FATFS g_fs;
static FIL   g_fil;

static uint64_t cnt(void) { uint64_t v; __asm volatile("mrs %0, cntpct_el0" : "=r"(v)); return v; }

static uint32_t sum32(uintptr_t a, uint32_t n) {
    uint32_t s = 0; const volatile uint32_t *p = (const volatile uint32_t *)a;
    for (uint32_t i = 0; i < n / 4; i++) s += p[i];
    return s;
}

static void dcache_clean_inval(uintptr_t a, uint32_t n) {
    uint64_t ctr; __asm volatile("mrs %0, ctr_el0" : "=r"(ctr));
    uintptr_t line = 4u << ((ctr >> 16) & 0xf);
    uintptr_t end = a + n;
    for (a &= ~(line - 1); a < end; a += line)
        __asm volatile("dc civac, %0" :: "r"(a) : "memory");
    __asm volatile("dsb sy" ::: "memory");
}

static int load_file(const char *name, uintptr_t dst, uint32_t max, uint32_t *len) {
    FRESULT fr = f_open(&g_fil, name, FA_READ);
    if (fr != FR_OK) { printf("sdboot: ERROR open %s (FR=%d)\n", name, (int)fr); return -1; }
    uint32_t size = (uint32_t)f_size(&g_fil);
    printf("sdboot: %s %u bytes -> %08lX\n", name, (unsigned)size, (unsigned long)dst);
    if (size == 0 || size > max) { printf("sdboot: ERROR size out of range (max %u)\n", (unsigned)max); f_close(&g_fil); return -1; }
#if SDIO_PROFILE
    sdio_prof_reset();
#endif
    uint64_t t0 = cnt();
    uint32_t done = 0, mark = 1u << 20;
    while (done < size) {
        UINT want = (size - done >= 512u) ? 512u : (size - done), br = 0;
        fr = f_read(&g_fil, (void *)(dst + done), want, &br);
        if (fr != FR_OK || br == 0) { printf("sdboot: ERROR read at %u (FR=%d, br=%u)\n", (unsigned)done, (int)fr, (unsigned)br); f_close(&g_fil); return -1; }
        done += br;
        if (done >= mark) { printf("  %u KB\n", (unsigned)(done >> 10)); mark += 1u << 20; }
    }
    f_close(&g_fil);
    uint32_t ms = (uint32_t)((cnt() - t0) / (CNT_HZ / 1000u));
    printf("sdboot: %s done in %u ms, sum32 %08lX\n", name, (unsigned)ms, (unsigned long)sum32(dst, done));
#if SDIO_PROFILE
    if (sdio_prof.n_blk) {
        uint32_t n = sdio_prof.n_blk;
        /* per-block averages in microseconds (counter at CNT_HZ = 20 MHz) */
        printf("sdboot: prof %u blocks, start-retries %u: avg start %u us, busy %u us (phase1 %u us/%u polls, phase2 %u us/%u polls), fifo %u us, block %u us, max block %u us\n",
               (unsigned)n, (unsigned)sdio_prof.n_start_retry,
               (unsigned)(sdio_prof.t_start / n / (CNT_HZ / 1000000u)),
               (unsigned)(sdio_prof.t_busy  / n / (CNT_HZ / 1000000u)),
               (unsigned)(sdio_prof.t_busy1 / n / (CNT_HZ / 1000000u)), (unsigned)(sdio_prof.polls_busy1 / n),
               (unsigned)(sdio_prof.t_busy2 / n / (CNT_HZ / 1000000u)), (unsigned)(sdio_prof.polls_busy2 / n),
               (unsigned)(sdio_prof.t_fifo  / n / (CNT_HZ / 1000000u)),
               (unsigned)(sdio_prof.t_blk   / n / (CNT_HZ / 1000000u)),
               (unsigned)(sdio_prof.max_blk / (CNT_HZ / 1000000u)));
    }
#endif

    *len = done;
    return 0;
}

int main(void) {
    uint32_t shim_len = 0, dtb_len = 0, image_len = 0;
    UartStdOutInit();
    printf("\nsdboot v2: MegaSoC SD-card Linux loader (17 Nov 2026, read-path profile)\n");

    DSTATUS st = disk_initialize(0);
    if (st & (STA_NOINIT | STA_NODISK)) { printf("sdboot: ERROR card init (DSTATUS=%u)\n", (unsigned)st); TEST_FAIL(); }
    FRESULT fr = f_mount(&g_fs, "", 1);
    if (fr != FR_OK) { printf("sdboot: ERROR f_mount (FR=%d)\n", (int)fr); TEST_FAIL(); }
    printf("sdboot: volume mounted, fs_type %u, %u sectors/cluster\n", (unsigned)g_fs.fs_type, (unsigned)g_fs.csize);
    printf("sdboot: SDIO PHY=%08lX CMD=%08lX\n", (unsigned long)*(volatile uint32_t *)0x01010010u, (unsigned long)*(volatile uint32_t *)0x01010000u);

    if (load_file("SHIM.BIN", SHIM_ADDR, SHIM_MAX, &shim_len)) TEST_FAIL();
    if (load_file("MEGASOC.DTB", DTB_ADDR, DTB_MAX, &dtb_len)) TEST_FAIL();
    if (load_file("IMAGE", IMAGE_ADDR, IMAGE_MAX, &image_len)) TEST_FAIL();

    const volatile uint8_t *dtb = (const volatile uint8_t *)DTB_ADDR;
    const volatile uint8_t *img = (const volatile uint8_t *)IMAGE_ADDR;
    if (dtb[0] != 0xd0 || dtb[1] != 0x0d || dtb[2] != 0xfe || dtb[3] != 0xed) {
        printf("sdboot: ERROR DTB magic %02x%02x%02x%02x (byte order?)\n", dtb[0], dtb[1], dtb[2], dtb[3]); TEST_FAIL();
    }
    if (img[0x38] != 'A' || img[0x39] != 'R' || img[0x3a] != 'M' || img[0x3b] != 0x64) {
        printf("sdboot: ERROR Image magic %02x%02x%02x%02x at +0x38\n", img[0x38], img[0x39], img[0x3a], img[0x3b]); TEST_FAIL();
    }
    printf("sdboot: DTB and Image magics OK\n");

    dcache_clean_inval(SHIM_ADDR, shim_len);
    dcache_clean_inval(DTB_ADDR, dtb_len);
    dcache_clean_inval(IMAGE_ADDR, image_len);
    __asm volatile("ic iallu; dsb sy; isb" ::: "memory");
    printf("sdboot: caches cleaned, jumping to the shim at %08lX\n\n", (unsigned long)SHIM_ADDR);
    ((void (*)(void))SHIM_ADDR)();
    TEST_FAIL();
    return 0;
}
