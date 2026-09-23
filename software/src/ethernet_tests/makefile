##------------------------------------------------------------------------------
## MegaSoC Ethernet Subsystem Integration Test — Makefile
##
## Follows the same pattern as hello_world/makefile.
## Includes the ethmac and ha1588 driver sources and headers.
##
## A joint work commissioned on behalf of SoC Labs, under Arm Academic Access
## license.
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

ETHMAC_SW_DIR := $(SOCLABS_MEGASOC_TECH_DIR)/logical/ethernet_mac_ahb/src/sw

APP_TESTDEFINES     :=

APP_SCAT := default
APP_LD_FLAGS :=
APP_AS_FLAGS := -DMASTER_CPU_REF=0x0 -DMASTER_CLUS_REF=0x0 -DTTBR0_BASE=0x02100000 -DVBAR_ADDR=0x02000000
APP_AS_INC_DIR :=
APP_C_FLAGS := $(APP_TESTDEFINES) $(GLOBAL_TESTDEFINES) -DTTBR0_BASE=0x02100000
APP_C_INC_DIR := $(TEST_PATH) $(ETHMAC_SW_DIR)

APP_SRC := ethernet_tests.c $(ETHMAC_SW_DIR)/ethmac.c $(ETHMAC_SW_DIR)/ha1588.c

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
