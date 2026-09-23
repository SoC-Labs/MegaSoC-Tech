//-----------------------------------------------------------------------------
// MegaSoC Ethernet Subsystem Integration Test
//
// Verifies the ethmac_subsystem_apb integration at system level:
//   1. MDIO register access (PHY connectivity via ethmac driver)
//   2. MAC DMA loopback (TX -> RX path via internal loopback)
//   3. PTP HA1588 register access (scratch + RTC read-back)
//
// Requires: mdl_ethphy.v in testbench (PHY_ADDR=1, PHYID1=0x2000, PHYID2=0x1234)
//
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access
// license.
//-----------------------------------------------------------------------------

#include "uart_stdout.h"
#include "system.h"
#include "ethmac.h"
#include "ha1588.h"
#include <stdio.h>
#include <string.h>

// SoC address map — ethmac_subsystem_apb APB register space.
// These addresses match the NIC400 decode; update if the map changes.
#define ETHMAC_BASE     0x4000C000UL
#define HA1588_BASE     (ETHMAC_BASE + 0x1000UL)

// MDIO test constants
#define PHY_ADDR        1UL
#define EXPECTED_PHYID1 0x2000U
#define EXPECTED_PHYID2 0x1234U

// MAC address for loopback test (arbitrary, doesn't matter for loopback)
static const uint8_t test_mac[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };

// Test frame payload (46 bytes to meet minimum Ethernet payload)
static const uint8_t test_frame[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
    0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C
};

#define TEST_FRAME_LEN  sizeof(test_frame)
#define MII_TIMEOUT     65000U

//-----------------------------------------------------------------------------
// Test 1: MDIO Register Access
//-----------------------------------------------------------------------------
static uint32_t test_mdio(void)
{
    ethmac_t eth;
    uint32_t errors = 0;
    int val;

    printf("[MDIO] Initialising ethmac driver...\n");
    ethmac_init(&eth, ETHMAC_BASE, 1);

    // Configure MII clock divider (default is 0x64 = 100, give slower for sim)
    ethmac_mii_set_clkdiv(&eth, 0x04);

    // Read PHYID1 (reg 0x02) — mdl_ethphy returns 0x2000
    printf("[MDIO] Reading PHY ID registers...\n");
    val = ethmac_mii_read(&eth, PHY_ADDR, 0x02, MII_TIMEOUT);
    printf("[MDIO] PHYID1 = 0x%04X (expected 0x%04X)\n", val, EXPECTED_PHYID1);
    if (val != EXPECTED_PHYID1) {
        printf("[MDIO] FAIL: PHYID1 mismatch\n");
        errors++;
    }

    // Read PHYID2 (reg 0x03) — mdl_ethphy returns 0x1234
    val = ethmac_mii_read(&eth, PHY_ADDR, 0x03, MII_TIMEOUT);
    printf("[MDIO] PHYID2 = 0x%04X (expected 0x%04X)\n", val, EXPECTED_PHYID2);
    if (val != EXPECTED_PHYID2) {
        printf("[MDIO] FAIL: PHYID2 mismatch\n");
        errors++;
    }

    // Write-and-read-back BMCR (reg 0x00)
    // Read current value first
    val = ethmac_mii_read(&eth, PHY_ADDR, 0x00, MII_TIMEOUT);
    printf("[MDIO] BMCR current = 0x%04X\n", val);

    // Write a known value: set loopback bit (bit 14)
    uint32_t bmcr_wr = (uint32_t)val | (1UL << 14);
    if (ethmac_mii_write(&eth, PHY_ADDR, 0x00, bmcr_wr, MII_TIMEOUT) != 0) {
        printf("[MDIO] FAIL: MII write timeout\n");
        errors++;
    }

    // Read back
    val = ethmac_mii_read(&eth, PHY_ADDR, 0x00, MII_TIMEOUT);
    printf("[MDIO] BMCR after write = 0x%04X (expected 0x%04X)\n", val, bmcr_wr);
    if (val != (int)bmcr_wr) {
        printf("[MDIO] FAIL: BMCR read-back mismatch\n");
        errors++;
    }

    // Restore BMCR (clear loopback)
    ethmac_mii_write(&eth, PHY_ADDR, 0x00, bmcr_wr & ~(1UL << 14), MII_TIMEOUT);

    return errors;
}

//-----------------------------------------------------------------------------
// Test 2: MAC DMA Loopback
//-----------------------------------------------------------------------------
static uint32_t test_dma_loopback(void)
{
    ethmac_t eth;
    uint32_t errors = 0;

    printf("[DMA] Initialising ethmac driver...\n");
    ethmac_init(&eth, ETHMAC_BASE, 1);

    // Set MAC address
    ethmac_set_mac_addr(&eth, test_mac);

    // Init buffer descriptors
    ethmac_init_bds(&eth);

    // Set promiscuous mode so loopback frames pass RX filter
    ethmac_set_promiscuous(&eth, 1);

    // Enable MAC loopback (TX echoed to RX internally)
    ethmac_set_loopback(&eth, 1);

    // Enable CRC append and pad
    eth->regs->MODER |= ETHMAC_MODER_CRCEN_Msk | ETHMAC_MODER_PAD_Msk;

    printf("[DMA] Setting up TX BD (frame len = %d bytes)...\n", (int)TEST_FRAME_LEN);

    // Setup TX BD: buffer address is the physical address of test_frame array.
    // In bare-metal identity mapping, the C pointer IS the physical address.
    ethmac_tx_bd_setup(&eth, 0,
                       (uint32_t)(uintptr_t)test_frame,
                       TEST_FRAME_LEN,
                       ETHMAC_TX_BD_CRC_Msk | ETHMAC_TX_BD_PAD_Msk,
                       1);  // last = 1 (wrap)

    printf("[DMA] Setting up RX BD...\n");

    // RX buffer: we use a separate RAM buffer to receive into.
    // The RX BD buffer address is where the MAC DMA writes received data.
    static uint8_t rx_buf[2048] __attribute__((aligned(4)));
    memset(rx_buf, 0, sizeof(rx_buf));

    ethmac_rx_bd_setup(&eth, 0,
                       (uint32_t)(uintptr_t)rx_buf,
                       0,   // no IRQ
                       1);  // last = 1 (wrap)

    // Enable MAC TX and RX
    printf("[DMA] Enabling MAC...\n");
    ethmac_enable(&eth);

    // Hand TX BD to MAC
    printf("[DMA] Starting TX...\n");
    ethmac_tx_bd_ready(&eth, 0);

    // Poll for TX completion
    uint32_t timeout = 5000000U;
    while (!ethmac_tx_bd_done(&eth, 0) && --timeout) { }
    if (timeout == 0) {
        printf("[DMA] FAIL: TX completion timeout\n");
        errors++;
        return errors;
    }
    printf("[DMA] TX complete\n");

    // Check RX
    timeout = 5000000U;
    while (!ethmac_rx_bd_received(&eth, 0) && --timeout) { }
    if (timeout == 0) {
        printf("[DMA] FAIL: RX completion timeout\n");
        errors++;
        return errors;
    }

    uint32_t rx_len = ethmac_rx_bd_length(&eth, 0);
    printf("[DMA] RX frame received: %d bytes\n", (int)rx_len);

    // The received frame includes the MAC address + type/length + payload.
    // For a 46-byte payload with Ethernet header (14 bytes), total = 60 bytes
    // (without CRC, which is stripped by MAC). With pad, minimum is 60 bytes.
    // Compare the payload portion (skip 14-byte Ethernet header).
    if (rx_len < TEST_FRAME_LEN) {
        printf("[DMA] FAIL: RX frame too short (%d < %d)\n",
               (int)rx_len, (int)TEST_FRAME_LEN);
        errors++;
    } else {
        // Compare the test frame data against received data (after Ethernet header)
        uint32_t cmp_offset = 14;  // skip DA(6) + SA(6) + Type(2)
        uint32_t match = 1;
        for (uint32_t i = 0; i < TEST_FRAME_LEN; i++) {
            if (rx_buf[cmp_offset + i] != test_frame[i]) {
                printf("[DMA] FAIL: byte[%d] TX=0x%02X RX=0x%02X\n",
                       (int)i, test_frame[i], rx_buf[cmp_offset + i]);
                match = 0;
                errors++;
                break;
            }
        }
        if (match) {
            printf("[DMA] PASS: TX/RX data match\n");
        }
    }

    // Disable MAC
    ethmac_disable(&eth);
    ethmac_set_loopback(&eth, 0);
    ethmac_set_promiscuous(&eth, 0);

    return errors;
}

//-----------------------------------------------------------------------------
// Test 3: PTP (HA1588) Register Access
//-----------------------------------------------------------------------------
static uint32_t test_ptp(void)
{
    ha1588_t ha;
    uint32_t errors = 0;

    printf("[PTP] Initialising HA1588 driver...\n");
    ha1588_init(&ha, HA1588_BASE);

    // Test 1: SCRATCH register write/read-back
    printf("[PTP] Testing SCRATCH register...\n");
    HA1588_REG(&ha, HA1588_SCRATCH) = 0xDEADBEEF;
    uint32_t scratch = HA1588_REG(&ha, HA1588_SCRATCH);
    printf("[PTP] SCRATCH = 0x%08X (expected 0xDEADBEEF)\n", scratch);
    if (scratch != 0xDEADBEEF) {
        printf("[PTP] FAIL: SCRATCH mismatch\n");
        errors++;
    }

    // Test 2: Set period (125 MHz = 8 ns per cycle)
    printf("[PTP] Setting RTC period to 8 ns (125 MHz)...\n");
    ha1588_rtc_set_period(&ha, 8, 0);

    // Test 3: Set time and read back
    printf("[PTP] Setting RTC time to 100.123456789...\n");
    ha1588_rtc_set_time(&ha, 0, 100, 123456789);

    ha1588_ts_t ts;
    ha1588_rtc_read_time(&ha, &ts);
    printf("[PTP] RTC time = %u.%u.%u\n", ts.sec_hi, ts.sec_lo, ts.nanoseconds);

    if (ts.sec_lo != 100) {
        printf("[PTP] FAIL: seconds mismatch (%u != 100)\n", ts.sec_lo);
        errors++;
    }
    // Nanoseconds may have advanced during the test, so check it's in range
    if (ts.nanoseconds < 123456000U || ts.nanoseconds > 123458000U) {
        printf("[PTP] FAIL: nanoseconds out of range (%u)\n", ts.nanoseconds);
        errors++;
    } else {
        printf("[PTP] PASS: RTC time read-back OK\n");
    }

    // Test 4: Reset RTC to zero
    printf("[PTP] Resetting RTC...\n");
    ha1588_rtc_reset(&ha);
    ha1588_rtc_read_time(&ha, &ts);
    printf("[PTP] After reset: %u.%u.%u\n", ts.sec_hi, ts.sec_lo, ts.nanoseconds);
    if (ts.sec_lo != 0 || ts.nanoseconds > 1000U) {
        printf("[PTP] FAIL: RTC reset did not zero counters\n");
        errors++;
    } else {
        printf("[PTP] PASS: RTC reset OK\n");
    }

    return errors;
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main(void)
{
    uint32_t total_errors = 0;

    UartStdOutInit();
    printf("\n=== MegaSoC Ethernet Subsystem Integration Test ===\n\n");

    // Test 1: MDIO
    printf("--- Test 1: MDIO Register Access ---\n");
    total_errors += test_mdio();
    printf("\n");

    // Test 2: DMA Loopback
    printf("--- Test 2: MAC DMA Loopback ---\n");
    total_errors += test_dma_loopback();
    printf("\n");

    // Test 3: PTP
    printf("--- Test 3: PTP (HA1588) Registers ---\n");
    total_errors += test_ptp();
    printf("\n");

    // Summary
    printf("=== Results: %d error(s) ===\n", (int)total_errors);
    if (total_errors == 0) {
        TEST_PASS();
    } else {
        TEST_FAIL();
    }

    return 0;
}
