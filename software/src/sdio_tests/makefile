##------------------------------------------------------------------------------
## The confidential and proprietary information contained in this file may
## only be used by a person authorised under and to the extent permitted
## by a subsisting licensing agreement from Arm Limited or its affiliates.
##
##        (C) COPYRIGHT 2018-2021 Arm Limited or its affiliates.
##            ALL RIGHTS RESERVED
##
## This entire notice must be reproduced on all copies of this file
## and copies of this file may only be made by a person if such person is
## permitted to do so under the terms of a subsisting license agreement
## from Arm Limited or its affiliates.
##
##      Release Information : SSE710-r0p0-00rel0
##
##------------------------------------------------------------------------------
## Purpose : C test Makefile
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
# Simulation-specific driver policy overrides.
# These replace the PYNQ FPGA defaults defined in sdiodrv.c with values that
# are safe for the behavioural SD card model used in VCS simulation.  Do NOT
# apply these flags to an FPGA build — the FPGA defaults are required there.
#
#   SDIO_POST_WRITE_STATUS_CHECK=0
#     The behavioural SD model does not implement CMD13 (SEND_STATUS).
#     The FPGA default (1) issues CMD13 after every CMD24 write to flush the
#     AXI pipeline; in simulation CMDBUSY is never cleared by the model,
#     causing sdio_wait_while_busy() to time out and every write to fail.
#
#   SDIO_BUSY_TIMEOUT_CYCLES=5000u
#     Reduces the busy-wait budget from 50 M cycles (~1 s at 50 MHz on PYNQ)
#     to a simulation-friendly fail-fast value.  Without this, a stuck-busy
#     condition spins for millions of simulated nanoseconds before reporting.
#
#   PYNQ_SDIO_TARGET_CLK=SDIOCK_25MHZ
#     The current initialization path remains in the card's default-speed
#     access mode: it sends ACMD6 to select bus width, but deliberately skips
#     CMD6 SWITCH_FUNC high-speed negotiation.  Default-speed SD is limited to
#     25 MHz, so selecting the driver's 50 MHz FPGA default here violates the
#     protocol and makes the VIP report
#     card_data_transfer_mode_frequency_check on every clock edge.
APP_C_FLAGS := $(APP_TESTDEFINES) $(GLOBAL_TESTDEFINES) \
               -DTTBR0_BASE=0x02100000 \
               -DSDIO_BUSY_TIMEOUT_CYCLES=50000u \
               -DSDIO_POST_WRITE_STATUS_CHECK=1 \
               -DPYNQ_SDIO_ENABLE_4BIT=1 \
               -DPYNQ_SDIO_TARGET_CLK=SDIOCK_25MHZ \
               -DPYNQ_SDIO_TARGET_CLK_NAME=\"25MHz\"
APP_C_INC_DIR := $(TEST_PATH)
APP_SRC := sdio_tests.c

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
