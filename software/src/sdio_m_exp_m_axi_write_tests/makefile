##------------------------------------------------------------------------------
## sdio_m_exp_m_axi_write_tests (2026-11-12, NIC-400 close-out)
## Builds ../sdio_m_exp_m_reach_tests/sdio_m_exp_m_reach_tests.c with only the
## read-probe (sdio_read into EXP_M = AXI WRITE at the EXP_M boundary) and no
## post-abort recovery step (BUG-42). Closes cx_exp_m_absent_rw
## sdio_m_manager x write for MEGASOC-ILL-008. Runs on the SDIO VIP bench.
##------------------------------------------------------------------------------
##------------------------------------------------------------------------------
## Purpose : C test Makefile for sdio_m_exp_m_reach_tests
##           (tc_sdio_m_exp_m_reach)
##
## Targets vplan items MEGASOC-CONN-002 (sdio_m_target_matrix, EXP_M bin --
## the last of the 5 targets, ROM/FLASH/RAM/DRAM already closed by
## sdio_dma_read_rom_flash_tests / cpu_plus_sdio_dma_same_target_tests /
## cpu_plus_sdio_dma_disjoint_tests) and MEGASOC-ILL-008
## (exp_m_decerr_absent_target, sdio_m_manager half of the cross -- the
## a53_manager half is already closed by a53_target_matrix_tests).
## Structure follows sdio_dma_read_rom_flash_tests/makefile
## (SDIO_BUSY_TIMEOUT_CYCLES etc. proven-working values for the
## verif/testbench/logical/uvm_vip / uvm_vip_tb flow with a real, reactive
## svt_emmc_card_agent).
##
## Sweepable knobs (pass on the make command line, propagated to sub-makes):
##   TEST_VERBOSITY=1   verbose per-access prints
##------------------------------------------------------------------------------

TEST_PATH       := $(CURDIR)
TEST_ID         := $(notdir $(TEST_PATH))

export TEST_PATH
export TEST_ID

#############################################################################
#### Variables globally applied for all images                         ######
#############################################################################

ifeq ($(TEST_VERBOSITY), 1)
  GLOBAL_TESTDEFINES  := -DTEST_VERBOSITY_HIGH
endif

#############################################################################
############# Variables necessary for compiling image for Cortex A53 ########
#############################################################################

APP_TESTDEFINES     := -DEXP_M_PROBE_ONLY=2 -DEXP_M_SKIP_RECOVERY

APP_SCAT := default
APP_LD_FLAGS :=
APP_AS_FLAGS := -DMASTER_CPU_REF=0x0 -DMASTER_CLUS_REF=0x0 -DTTBR0_BASE=0x02100000 -DVBAR_ADDR=0x02000000
APP_AS_INC_DIR :=
# Simulation-specific driver policy overrides -- same values/rationale as
# sdio_dma_read_rom_flash_tests/makefile and
# cpu_plus_sdio_dma_disjoint_tests/makefile (sibling SDIO-DMA tests):
# SDIO_BUSY_TIMEOUT_CYCLES=50000u matches sdio_tests' proven-working value
# for verif/testbench/logical/uvm_vip (uvm_vip_tb), which has a real,
# reactive svt_emmc_card_agent -- a small busy-wait budget would truncate
# legitimate busy-wait stretches for real command/data completions. This
# same bounded timeout is also what safely terminates the DMA busy-wait
# after the deliberate EXP_M DECERR abort this test drives (see
# sdiodrv.c sdio_wait_while_busy_timeout()).
APP_C_FLAGS := $(APP_TESTDEFINES) $(GLOBAL_TESTDEFINES) \
               -DTTBR0_BASE=0x02100000 \
               -DSDIO_BUSY_TIMEOUT_CYCLES=50000u \
               -DSDIO_POST_WRITE_STATUS_CHECK=1 \
               -DPYNQ_SDIO_ENABLE_4BIT=1 \
               -DPYNQ_SDIO_TARGET_CLK=SDIOCK_25MHZ \
               -DPYNQ_SDIO_TARGET_CLK_NAME=\"25MHz\"
APP_C_INC_DIR := $(TEST_PATH)
APP_SRC := sdio_m_exp_m_axi_write_tests.c

export APP_SRC
export APP_C_FLAGS
export APP_C_INC_DIR
export APP_AS_FLAGS
export APP_AS_INC_DIR
export APP_LD_FLAGS
export APP_SCAT

###############################################################################
###############################################################################
###############################################################################

include $(SOCLABS_MEGASOC_TECH_DIR)/software/lib/common/Makefile.sim
