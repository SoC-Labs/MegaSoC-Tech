//==============================================================================
// full_boot_sd_dram_tests -- vplan MEGASOC-BOOT-006 (tc_full_boot_sd_dram)
//
// Full boot flow, end to end:
//   1. Boot ROM runs from 0x0, sets up QSPI and jumps to flash at 0x0040_0000
//      (bootloader.c; same for every test).
//   2. This loader runs from QSPI flash (XIP).
//   3. It brings up DRAM (lpddr4_init() on INC_LPDDR4 builds) and the SD card.
//   4. It reads the boot header from SD sector SD_BOOT_IMAGE_SECTOR, checks it,
//      then reads the image into DRAM at the address the header gives.
//   5. It checks the image, does the cache maintenance needed before running
//      DMA-written code, and calls it in DRAM. The image (sd_image/
//      dram_payload.S) prints a hello-world line, fills in a result block and
//      returns.
//
// The bench preloads the image into the SD card model (UVM test
// sdio_full_boot_test, verif/testbench/logical/uvm_vip/sdio_full_boot_test.sv,
// from sd_image/sd_boot_image.hex), so the card holds the image before the CPU
// starts, as a real card would.
//
// Pass criteria (all must hold):
//   - header magic/version/check word valid, load range inside DRAM;
//   - payload checksum read back from DRAM matches the header;
//   - the payload returns BOOT_PAYLOAD_MAGIC, its result block holds the same
//     magic, sum 5050, and an entry PC equal to the DRAM load address (so it
//     really ran from DRAM, not from a stale copy elsewhere);
//   - the write-back pattern (sectors 0x3100-0x3102) is written without error.
//     The bench then checks the card memory byte for byte.
//
// SD byte order: unlike a write-then-read round trip, this test compares
// against data the card itself holds, so it checks byte order on all four
// SDIO data paths: header read (1 sector, PIO), image read (2 sectors, DMA),
// pattern write 1 sector (PIO) and 2 sectors (DMA, checked by the bench).
//
// Cache maintenance: DRAM is Normal write-back cacheable (page_table.s entry
// 2). The SDIO controller's DMA does not snoop the CPU caches, and sdiodrv.c's
// read path does no cache maintenance of its own, so this test cleans and
// invalidates the load range before and after the DMA, then invalidates the
// instruction cache before the jump.
//
// NOTE: printf() is c_print(), which formats into a fixed 160-byte stack
// buffer whenever the format contains '%'. Format strings are kept short.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "sdiodrv.h"
#ifdef INC_LPDDR4
#include "lpddr4.h"
#endif
#include <stdio.h>
#include <stdint.h>

static SDIO *SDIO_S = (SDIO *)SYS_SDIO_BASE;

// Must match sdio_full_boot_test.sv (where the bench puts the image) and
// sd_image/gen_sd_boot_image.py (header layout).
#define SD_BOOT_IMAGE_SECTOR   0x3000u
#define SD_SECTOR_BYTES        512u
#define BOOT_HDR_MAGIC         0x4942534Du   /* "MSBI" */
#define BOOT_HDR_VERSION       1u
#define BOOT_PAYLOAD_MAGIC     0xD7A3B007u
#define BOOT_PAYLOAD_SUM       5050u         /* 1 + 2 + ... + 100 */
#define BOOT_MAX_IMAGE_BYTES   0x10000u      /* 64 KB sanity limit */

// Write-path byte-order check: the loader writes a known pattern to the card
// and the bench (sdio_full_boot_test check_phase) compares the card memory.
// Sector WR_SECTOR is written alone (single block, PIO path), the next
// WR_DMA_SECTORS together (multi-block, controller DMA). Byte k of the
// 3-sector pattern is (k * 7 + 0x11) & 0xff. Must match sdio_full_boot_test.sv.
#define WR_SECTOR              0x3100u
#define WR_DMA_SECTORS         2u
#define WR_BUF_ADDR            0x0000000080050000ULL

#define DRAM_WIN_LO            0x0000000080000000ULL
#define DRAM_WIN_HI            0x00000000FFFFFFFFULL
#define CACHE_LINE             64u

typedef struct {
  uint32_t magic;
  uint32_t version;
  uint32_t load_addr_lo;
  uint32_t load_addr_hi;
  uint32_t size;
  uint32_t entry_off;
  uint32_t checksum;
  uint32_t hdr_check;
} boot_hdr_t;

// Filled in by the payload; offsets are fixed by dram_payload.S
typedef struct {
  uint32_t magic;
  uint32_t sum;
  uint64_t entry_pc;
} boot_result_t;

typedef uint32_t (*boot_entry_t)(int (*print)(const char *, ...),
                                 volatile boot_result_t *result);

static uint8_t hdr_sector[SD_SECTOR_BYTES] __attribute__((aligned(CACHE_LINE)));
static volatile boot_result_t boot_result;

static void dcache_civac_range(uint64_t addr, uint64_t len)
{
  uint64_t a;
  for (a = addr & ~(uint64_t)(CACHE_LINE - 1); a < addr + len; a += CACHE_LINE)
    __asm__ volatile ("dc civac, %0" : : "r"(a) : "memory");
  __asm__ volatile ("dsb sy" ::: "memory");
}

static void icache_invalidate_all(void)
{
  __asm__ volatile ("dsb sy\n\t"
                    "ic iallu\n\t"
                    "dsb sy\n\t"
                    "isb" ::: "memory");
}

static uint64_t current_pc(void)
{
  uint64_t pc;
  __asm__ volatile ("adr %0, ." : "=r"(pc));
  return pc;
}

static int check_eq32(const char *what, uint32_t expected, uint32_t got)
{
  if (expected == got)
    return 0;
  printf("[MEGASOC-BOOT-006] %s: expected 0x%08x got 0x%08x\n", what, expected, got);
  return 1;
}

static int read_header(SDIODRV *dev, boot_hdr_t *hdr)
{
  const uint32_t *w = (const uint32_t *)hdr_sector;
  uint32_t sum = 0;
  int errors = 0;
  unsigned i;

  dcache_civac_range((uint64_t)(unsigned long)hdr_sector, SD_SECTOR_BYTES);
  if (sdio_read(dev, SD_BOOT_IMAGE_SECTOR, 1, (char *)hdr_sector) != 0) {
    printf("[MEGASOC-BOOT-006] header sdio_read failed\n");
    return 1;
  }
  dcache_civac_range((uint64_t)(unsigned long)hdr_sector, SD_SECTOR_BYTES);

  for (i = 0; i < 8; i++)
    ((uint32_t *)hdr)[i] = w[i];
  for (i = 0; i < 7; i++)
    sum += w[i];

  errors += check_eq32("header magic", BOOT_HDR_MAGIC, hdr->magic);
  errors += check_eq32("header version", BOOT_HDR_VERSION, hdr->version);
  errors += check_eq32("header check word", ~sum, hdr->hdr_check);
  return errors;
}

static int load_image(SDIODRV *dev, const boot_hdr_t *hdr, uint64_t load)
{
  unsigned sectors = (hdr->size + SD_SECTOR_BYTES - 1) / SD_SECTOR_BYTES;
  const volatile uint32_t *w = (const volatile uint32_t *)(unsigned long)load;
  uint32_t sum = 0;
  unsigned i;

  // Clean+invalidate first so no dirty line can later overwrite the DMA data,
  // and again after so the CPU reads what the DMA wrote.
  dcache_civac_range(load, (uint64_t)sectors * SD_SECTOR_BYTES);
  if (sdio_read(dev, SD_BOOT_IMAGE_SECTOR + 1, sectors, (char *)(unsigned long)load) != 0) {
    printf("[MEGASOC-BOOT-006] image sdio_read failed\n");
    return 1;
  }
  dcache_civac_range(load, (uint64_t)sectors * SD_SECTOR_BYTES);

  for (i = 0; i < hdr->size / 4; i++)
    sum += w[i];
  printf("[MEGASOC-BOOT-006] image: %u sectors copied to DRAM\n", sectors);
  return check_eq32("image checksum", hdr->checksum, sum);
}

static int write_pattern(SDIODRV *dev)
{
  volatile uint8_t *buf = (volatile uint8_t *)(unsigned long)WR_BUF_ADDR;
  unsigned len = (1 + WR_DMA_SECTORS) * SD_SECTOR_BYTES;
  unsigned k;

  for (k = 0; k < len; k++)
    buf[k] = (uint8_t)(k * 7 + 0x11);
  // The controller DMA reads DRAM directly, so push the pattern out of the cache
  dcache_civac_range(WR_BUF_ADDR, len);

  if (sdio_write(dev, WR_SECTOR, 1, (const char *)buf) != 0) {
    printf("[MEGASOC-BOOT-006] single-block sdio_write failed\n");
    return 1;
  }
  if (sdio_write(dev, WR_SECTOR + 1, WR_DMA_SECTORS,
                 (const char *)(buf + SD_SECTOR_BYTES)) != 0) {
    printf("[MEGASOC-BOOT-006] multi-block sdio_write failed\n");
    return 1;
  }
  printf("[MEGASOC-BOOT-006] pattern written to SD sectors 0x3100-0x3102\n");
  return 0;
}

int main(void)
{
  int errors = 0;
  SDIODRV *dev;
  boot_hdr_t hdr;
  uint64_t load, pc;
  uint32_t ret;
  boot_entry_t entry;

  UartStdOutInit();

  printf("MegaSoC full boot test: ROM -> QSPI -> DRAM+SDIO -> SD image -> DRAM\n");
  printf("vplan: MEGASOC-BOOT-006\n");

  pc = current_pc();
  printf("[MEGASOC-BOOT-006] loader running at 0x%08x\n", (uint32_t)pc);

#ifdef INC_LPDDR4
  if (lpddr4_init() != 0) {
    printf("[LPDDR4] ERROR: initialisation failed\n");
    TEST_FAIL();
    return 1;
  }
  printf("[LPDDR4] DRAM ready\n");
#endif

  dev = sdio_init(SDIO_S);
  if (dev == NULL) {
    printf("[MEGASOC-BOOT-006] sdio_init() returned NULL\n");
    TEST_FAIL();
    return 1;
  }
  printf("[MEGASOC-BOOT-006] SD card ready\n");

  if (read_header(dev, &hdr) != 0) {
    printf("[MEGASOC-BOOT-006] bad boot header, is the SD image preloaded?\n");
    TEST_FAIL();
    return 1;
  }

  load = ((uint64_t)hdr.load_addr_hi << 32) | hdr.load_addr_lo;
  printf("[MEGASOC-BOOT-006] header: load 0x%08x size %u\n", hdr.load_addr_lo, hdr.size);
  if (load < DRAM_WIN_LO || hdr.size == 0 || hdr.size > BOOT_MAX_IMAGE_BYTES ||
      load + hdr.size - 1 > DRAM_WIN_HI || hdr.entry_off >= hdr.size) {
    printf("[MEGASOC-BOOT-006] header load range outside DRAM\n");
    TEST_FAIL();
    return 1;
  }

  if (load_image(dev, &hdr, load) != 0) {
    TEST_FAIL();
    return 1;
  }

  icache_invalidate_all();

  boot_result.magic = 0;
  boot_result.sum = 0;
  boot_result.entry_pc = 0;

  printf("[MEGASOC-BOOT-006] jumping to DRAM 0x%08x\n", (uint32_t)(load + hdr.entry_off));
  entry = (boot_entry_t)(unsigned long)(load + hdr.entry_off);
  ret = entry(c_print, &boot_result);
  printf("[MEGASOC-BOOT-006] back from DRAM\n");

  errors += check_eq32("payload return", BOOT_PAYLOAD_MAGIC, ret);
  errors += check_eq32("result magic", BOOT_PAYLOAD_MAGIC, boot_result.magic);
  errors += check_eq32("result sum", BOOT_PAYLOAD_SUM, boot_result.sum);
  errors += check_eq32("entry pc hi", (uint32_t)((load + hdr.entry_off) >> 32),
                       (uint32_t)(boot_result.entry_pc >> 32));
  errors += check_eq32("entry pc lo", (uint32_t)(load + hdr.entry_off),
                       (uint32_t)boot_result.entry_pc);

  errors += write_pattern(dev);

  printf("Total number of full boot errors: %d\n", errors);

  if (errors == 0) {
    TEST_PASS();
  } else {
    TEST_FAIL();
  }

  return errors;
}
