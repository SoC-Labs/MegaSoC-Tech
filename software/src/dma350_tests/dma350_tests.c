#include "uart_stdout.h"
#include <stdio.h>
#include "system.h"
#include "sys_memory_map.h"
#include "dma_350_command_lib.h"
#include "sys_intr_map.h"
#include "gic400.h"

#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define HW8_REG(ADDRESS)   (*((volatile unsigned char  *)(ADDRESS)))

#define COPY_ADDR_SRC         0x00800000UL
#define COPY_ADDR_DST         0x00800F00UL
#define COPY_ADDR_DST_long    0x80000000000UL
#define DATA_SIZE             64

// IRQ Handlers
static void DMA_CH0_IRQ();
static void DMA_CH1_IRQ();
static void DMA_CH2_IRQ();
static void DMA_CH3_IRQ();
void DMAClearChIrq(uint8_t ch);


AdaChannelSettingsType ch_settings = {
    .CHPRIO         = 0,
    .CLEARCMD       = 1,
    .REGRELOADTYPE  = RELOAD_DISABLED,
    .DONETYPE       = DONETYPE_EOF_CMD,
    .DONEPAUSEEN    = 0,
    .SRCMAXBURSTLEN = 15,
    .DESMAXBURSTLEN = 15
  };
AdaChannelSrcAttrType ch_srcattr = {
    .SRCMEMATTRLO  = 4,
    .SRCMEMATTRHI  = 4,
    .SRCSHAREATTR  = 0,
    .SRCNONSECATTR = 1,
    .SRCPRIVATTR   = 0
  };
AdaChannelDesAttrType ch_desattr = {
    .DESMEMATTRLO  = 4,
    .DESMEMATTRHI  = 4,
    .DESSHAREATTR  = 0,
    .DESNONSECATTR = 1,
    .DESPRIVATTR   = 0
  };
AdaChannelLinkAttrType ch_linkattr = {
    .LINKMEMATTRLO = 4,
    .LINKMEMATTRHI = 4,
    .LINKSHAREATTR = 0
  };
AdaBaseCommandType command_base = {
    .SRCADDR  = COPY_ADDR_SRC, // Read from M0 interface
    .DESADDR  = COPY_ADDR_DST, // Write to M0 interface
    .SRCXSIZE = DATA_SIZE,
    .DESXSIZE = DATA_SIZE,
    .TRANSIZE = BITS_32
  };
AdaBaseCommandType command_base_long = {
    .SRCADDR  = COPY_ADDR_SRC, // Read from M0 interface
    .DESADDR  = COPY_ADDR_DST_long, // Write to M0 interface
    .SRCXSIZE = DATA_SIZE,
    .DESXSIZE = DATA_SIZE,
    .TRANSIZE = BITS_32
  };

Ada1DIncrCommandType command_1d_incr = {
    .SRCXADDRINC = 1,           // Autoincrement by transaction size
    .DESXADDRINC = 1            // Autoincrement by transaction size
  };

  // Set the transfer types (2D and wrapping support)
  // The transaction type is 1D basic transfer
  AdaWrapCommandType command_1d_wrap = {
    .FILLVAL  = 0,
    .XTYPE    = OPTYPE_CONTINUE,
    .YTYPE    = OPTYPE_DISABLE
  };
AdaIrqEnType ch_irqs = {
    .INTREN_DONE     = 1,
    .INTREN_ERR      = 1,
    .INTREN_DISABLED = 0,
    .INTREN_STOPPED  = 0
  };



int main(){
    uint32_t ch_num;
    uint32_t trig_in_num;
    uint32_t trig_out_num;

    UartStdOutInit();

    printf("DMA 350 tests - SoCLabs MegaSoC\n");

    gic_initialise_intr(DMA350_CH0_INTR, 0, 1, 0);
    gic_install_handler(DMA350_CH0_INTR, &DMA_CH0_IRQ);
    gic_enable_interrupt(DMA350_CH0_INTR);

    gic_initialise_intr(DMA350_CH1_INTR, 0, 1, 0);
    gic_install_handler(DMA350_CH1_INTR, &DMA_CH1_IRQ);
    gic_enable_interrupt(DMA350_CH1_INTR);

    gic_initialise_intr(DMA350_CH2_INTR, 0, 1, 0);
    gic_install_handler(DMA350_CH2_INTR, &DMA_CH2_IRQ);
    gic_enable_interrupt(DMA350_CH2_INTR);

    gic_initialise_intr(DMA350_CH3_INTR, 0, 1, 0);
    gic_install_handler(DMA350_CH3_INTR, &DMA_CH3_IRQ);
    gic_enable_interrupt(DMA350_CH3_INTR);

    enable_irq();


    //Get the configuration information
    ch_num = AdaGetChNum(SECURE);
    trig_in_num = AdaGetTrigInNum(SECURE);
    trig_out_num = AdaGetTrigOutNum(SECURE);

    //Display the config parameters read
    printf("Number of DMA channels: %d \n", ch_num);
    printf("Number of DMA trigger inputs: %d \n", trig_in_num);
    printf("Number of DMA trigger outputs: %d \n", trig_out_num);

    AdaSecAllChStopReq();

    printf("TEST 1: DMA transfer to/from CPU side SRAM\n");
    for (uint32_t ch=0; ch < ch_num-1; ch++) {
      //
      // Write all settings to the DMA registers
      AdaChannelInit(ch_settings, ch_srcattr, ch_desattr, ch, SECURE);
      Ada1DIncrCommand(command_base, command_1d_incr, ch, SECURE);
      SetAdaWrapRegs(command_1d_wrap, ch, SECURE);
      AdaSetIntEn(ch_irqs, ch, SECURE);    
      // Start DMA operation and wait for done IRQ
      printf("DMA CH:%d Enabling\n",ch);
      AdaEnable(ch, SECURE);
      call_wfi();
      printf("DMA CH:%d transfer finished\n",ch);
    }
    printf("TEST 2: DMA transfer from CPU side SRAM to System side\n");
    for (uint32_t ch=0; ch < 2; ch++) {
      //
      // Write all settings to the DMA registers
      AdaChannelInit(ch_settings, ch_srcattr, ch_desattr, ch, SECURE);
      AdaLong1DCommand(command_base_long, ch, SECURE);
      SetAda1DIncrRegs(command_1d_incr, ch, SECURE);
      SetAdaWrapRegs(command_1d_wrap, ch, SECURE);
      AdaSetIntEn(ch_irqs, ch, SECURE);    
      // Start DMA operation and wait for done IRQ
      printf("DMA CH:%d Enabling\n",ch);
      AdaEnable(ch, SECURE);
      call_wfi();
      printf("DMA CH:%d transfer finished\n",ch);
    }

    UartEndSimulation();

}

void DMAClearChIrq(uint8_t ch) {
  // Check the source of the interrupt and clear interrupts
  AdaStatType ST = AdaReadStatus(ch, NON_SECURE);
  if (ST.STAT_DONE == 1) {
    AdaClearChDone(ch, NON_SECURE);
  } else if (ST.STAT_ERR == 1) {
    AdaClearChError(ch, NON_SECURE);
  } else if (ST.STAT_DISABLED == 1) {
    AdaClearChDisabled(ch, NON_SECURE);
  } else if (ST.STAT_STOPPED == 1) {
    AdaClearChStopped(ch, NON_SECURE);
  } else {
    printf("Unknown IRQ on CH%d!\n", ch);
  }
}

void DMA_CH0_IRQ(){
  disable_irq();
  DMAClearChIrq(0);
  printf("DMA CH0 IRQ\n");
  enable_irq();
}

void DMA_CH1_IRQ(){
  disable_irq();
  DMAClearChIrq(1);
  printf("DMA CH1 IRQ\n");
  enable_irq();
}

void DMA_CH2_IRQ(){
  disable_irq();
  DMAClearChIrq(2);
  printf("DMA CH2 IRQ\n");
  enable_irq();
}

void DMA_CH3_IRQ(){
  disable_irq();
  DMAClearChIrq(3);
  printf("DMA CH3 IRQ\n");
  enable_irq();
}
