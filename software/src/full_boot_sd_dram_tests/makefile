##------------------------------------------------------------------------------
## Purpose : C test Makefile for full_boot_sd_dram_tests
##           (tc_full_boot_sd_dram)
##
## Targets vplan item MEGASOC-BOOT-006: boot ROM -> QSPI flash -> DRAM + SDIO
## bring-up -> SD card image copied to DRAM -> image run from DRAM.
## Structure follows cpu_plus_sdio_dma_disjoint_tests.
##
## The SD card image is NOT built here: sd_image/gen_sd_boot_image.py builds
## sd_image/sd_boot_image.hex from sd_image/dram_payload.S (GNU binutils), and
## the UVM test sdio_full_boot_test preloads it into the card model. Run with
## UVM_TESTNAME=sdio_full_boot_test.
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

APP_TESTDEFINES     :=

APP_SCAT := default
APP_LD_FLAGS :=
APP_AS_FLAGS := -DMASTER_CPU_REF=0x0 -DMASTER_CLUS_REF=0x0 -DTTBR0_BASE=0x02100000 -DVBAR_ADDR=0x02000000
APP_AS_INC_DIR :=
# SDIO driver policy: same values as cpu_plus_sdio_dma_disjoint_tests (real,
# reactive SD card VIP under uvm_vip_tb).
APP_C_FLAGS := $(APP_TESTDEFINES) $(GLOBAL_TESTDEFINES) \
               -DTTBR0_BASE=0x02100000 \
               -DSDIO_BUSY_TIMEOUT_CYCLES=50000u \
               -DSDIO_POST_WRITE_STATUS_CHECK=1 \
               -DPYNQ_SDIO_ENABLE_4BIT=1 \
               -DPYNQ_SDIO_TARGET_CLK=SDIOCK_25MHZ \
               -DPYNQ_SDIO_TARGET_CLK_NAME=\"25MHz\"
APP_C_INC_DIR := $(TEST_PATH)
APP_SRC := full_boot_sd_dram_tests.c
# LPDDR4 builds (make ... INC_LPDDR4=yes): bring up the real DRAM with the
# shared driver before the image is copied; skip-training PHY init.
ifeq ($(INC_LPDDR4),yes)
APP_C_FLAGS   += -DINC_LPDDR4 -DLPDDR4_SKIP_TRAINING=1
APP_C_INC_DIR += $(LPDDR4_PROJECT_DIR)/sw/libs/phyinit
APP_SRC       += $(SOCLABS_MEGASOC_TECH_DIR)/software/lib/sw_lib/devices/src/lpddr4.c
endif

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
