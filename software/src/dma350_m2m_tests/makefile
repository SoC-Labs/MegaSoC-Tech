##------------------------------------------------------------------------------
## Purpose : C test Makefile for dma350_m2m_tests
##
## DMA-350 test, INC_DMA builds only (make ... INC_DMA=yes).
## vplan: MEGASOC-DMA350-004, MEGASOC-DMA350-005, MEGASOC-DMA350-006
## Checks: ROM/SRAM/DRAM memory-to-memory copies, 1 B to 16 KB, unaligned starts, element sizes, guard bytes
##
## Sweepable knobs (pass on the make command line):
##   TEST_VERBOSITY=1   verbose prints
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
APP_C_FLAGS := $(APP_TESTDEFINES) $(GLOBAL_TESTDEFINES) -DTTBR0_BASE=0x02100000
APP_C_INC_DIR := $(TEST_PATH)
APP_SRC := dma350_m2m_tests.c
# LPDDR4 builds (make ... INC_LPDDR4=yes): bring up the real DRAM with the
# shared driver before the test touches it; skip-training PHY init.
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
