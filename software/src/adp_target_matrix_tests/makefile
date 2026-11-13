##------------------------------------------------------------------------------
## Purpose : C test Makefile for adp_target_matrix_tests (tc_adp_target_matrix)
##
## Targets vplan items MEGASOC-CONN-003 (ADP manager -> all 11 NIC-400 targets); ADP reads are checked by verif/tests/megasoc/check_adp_target_matrix.py.
## Structure follows the existing MegaSoC software tests (boot_smoke_tests,
## addr_decode_tests).
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
APP_SRC := adp_target_matrix_tests.c

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
