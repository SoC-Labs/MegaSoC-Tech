##------------------------------------------------------------------------------
## Purpose : C test Makefile for gpio_irq_handler_tests
##           (tc_gpio_irq_handler)
##
## Targets vplan item MEGASOC-IRQ-002 (gpio_combined_irqs): pad edge -> real IRQ handler.
## Structure follows the existing MegaSoC software tests (gic_vector_table_delivery_tests).
##
## Sweepable knobs (pass on the make command line, propagated to sub-makes):
##   TEST_VERBOSITY=1        verbose per-access prints
##   IRQ_WAIT_MAX_ITERS=<n>  override the bounded WFI-wait iteration cap
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

ifneq ($(IRQ_WAIT_MAX_ITERS),)
  GLOBAL_TESTDEFINES  += -DIRQ_WAIT_MAX_ITERS=$(IRQ_WAIT_MAX_ITERS)UL
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
APP_SRC := gpio_irq_handler_tests.c

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
