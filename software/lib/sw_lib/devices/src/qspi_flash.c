
#include "qspi_flash.h"

uint8_t SET_QIO_MODE_CMD = 0x38;

uint32_t SPI_READ_JEDIC(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI);
int SPI_STARTUP(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI);
void SPI_WAIT_BUSY(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI);
void SPI_RESET(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI);
void SET_QPI_MODE(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI);
void QPI_READ_WORDS_no_data(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI, uint32_t addr, uint8_t n_words, uint32_t n_dummy, uint8_t cont_read);
void QPI_SET_CONT_READ(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI);
void QPI_SET_CONT_READ_MICRON(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI);
void QPI_SET_AHB_MODE(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI);

void CACHE_STARTUP(CG092_TypeDef *CACHE);
int  WINBOND_QPI_STARTUP(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI);

void CACHE_STARTUP(CG092_TypeDef *CACHE){
    CACHE->CCR=0;
    CACHE->CCR=0x1; //DON'T USE PREFETCH
    while((CACHE->SR)&0x3 != 2){;}
}

int SPI_STARTUP(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI){
    uint32_t ID;
    // Check first if warm reset
    if(SL_AHB_QSPI->CTRL == 0){
        SPI_RESET(SL_AHB_QSPI);
        ID=SPI_READ_JEDIC(SL_AHB_QSPI);
        if((ID==0xFFFFFFFF)||(ID==0x00000000)){
            return 1;
        } else {
            if((ID&0xFF)==0xEF){
                // Winbond W25Qxx (MBT-47): validated on the HAPS-SX W25Q32FW, October 2026.
                // QE (SR2 bit 1) through the volatile status-register write, 38h to enter QPI, C0h with
                // P5:P4 = 11 (8 dummy clocks), then EBh with mode byte A0h for continuous read. The chip
                // counts the two mode-byte clocks within its eight, so the controller sends 6 dummy clocks.
                if(WINBOND_QPI_STARTUP(SL_AHB_QSPI) != 0){
                    return 1;
                }
            } else if((ID&0xFF)!=0xBF){
                SET_QIO_MODE_CMD = 0x35;
                SL_AHB_QSPI->AHB_CMD = (8<<SL_AHB_QSPI_SPI_CMD_N_Dummy_Pos) + 0x0B;
                SET_QPI_MODE(SL_AHB_QSPI);
                QPI_SET_CONT_READ_MICRON(SL_AHB_QSPI);
                QPI_READ_WORDS_no_data(SL_AHB_QSPI, 0x00000000, 1, 10, 0);
                SL_AHB_QSPI->CTRL |= (1<<25);
                QPI_READ_WORDS_no_data(SL_AHB_QSPI, 0x00001000, 1, 8, 1);
                QPI_SET_AHB_MODE(SL_AHB_QSPI);        
            } else{
                SL_AHB_QSPI->AHB_CMD = (4<<SL_AHB_QSPI_SPI_CMD_N_Dummy_Pos) + 0x0B;
                SET_QPI_MODE(SL_AHB_QSPI);
                QPI_READ_WORDS_no_data(SL_AHB_QSPI, 0x00000000, 1, 6, 0);
                QPI_SET_CONT_READ(SL_AHB_QSPI);
                QPI_READ_WORDS_no_data(SL_AHB_QSPI, 0x00001000, 1, 4, 1);
                QPI_SET_AHB_MODE(SL_AHB_QSPI);        
            }
            return 0;
        }
    }
    else {
        return 0;
    }
}

void QPI_SET_CONT_READ(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI){
    SL_AHB_QSPI->CTRL |= (1<<25);
}

void QPI_SET_CONT_READ_MICRON(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI){
    uint32_t CMD=0;
    // Write Enable
    SL_AHB_QSPI->SPI_CMD = 0x06;
    SL_AHB_QSPI->SPI_CMD = 0x106;
    SPI_WAIT_BUSY(SL_AHB_QSPI);

    // Write to volatile register
    SL_AHB_QSPI->WRITE_DATA[0]=0xF3;
    CMD = 0x81;
    CMD |= SL_AHB_QSPI_SPI_CMD_Wr_Enable_Msk;
    SL_AHB_QSPI->SPI_CMD = CMD;
    SL_AHB_QSPI->SPI_CMD |= SL_AHB_QSPI_SPI_CMD_Enable_Msk;
    SPI_WAIT_BUSY(SL_AHB_QSPI);

    // Write disable
    SL_AHB_QSPI->SPI_CMD = 0x04;
    SL_AHB_QSPI->SPI_CMD = 0x104;
    SPI_WAIT_BUSY(SL_AHB_QSPI);
}

void QPI_SET_AHB_MODE(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI){
    SL_AHB_QSPI->CTRL |= (1<<8);
}

void SPI_WAIT_BUSY(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI){
    while(((SL_AHB_QSPI->STATUS)&SL_AHB_QSPI_STATUS_Busy_Msk)!=1){;}
    while(((SL_AHB_QSPI->STATUS)&SL_AHB_QSPI_STATUS_Busy_Msk)!=0){;}
}

void SPI_RESET(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI){
    SL_AHB_QSPI->SPI_CMD = 0x66;
    SL_AHB_QSPI->SPI_CMD = 0x166;
    SPI_WAIT_BUSY(SL_AHB_QSPI);
    SL_AHB_QSPI->SPI_CMD = 0x99;
    SL_AHB_QSPI->SPI_CMD = 0x199;
    SPI_WAIT_BUSY(SL_AHB_QSPI);
}

uint32_t SPI_READ_JEDIC(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI){
    SL_AHB_QSPI->SPI_CMD = 0x0003009F;
    SL_AHB_QSPI->SPI_CMD = 0x0003039F;
    SPI_WAIT_BUSY(SL_AHB_QSPI);
    return SL_AHB_QSPI->READ_DATA[0];
}

void QPI_READ_WORDS_no_data(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI, uint32_t addr, uint8_t n_words, uint32_t n_dummy, uint8_t cont_read){
    uint32_t CMD = 0;

    SL_AHB_QSPI->SPI_ADDR = addr;

    if(cont_read==0){
        SL_AHB_QSPI->CTRL |= SL_AHB_QSPI_CTRL_Cont_Rd_Msk;
        SL_AHB_QSPI->CTRL |= (SL_AHB_QSPI_CTRL_Mode_Code_Msk & (0xA0<<16));
    }
    CMD |= (SL_AHB_QSPI_SPI_CMD_N_Dummy_Msk & (n_dummy<<12));
    CMD |= 0x0B;
    CMD |= SL_AHB_QSPI_SPI_CMD_Rd_Enable_Msk;
    CMD |= SL_AHB_QSPI_SPI_CMD_Addr_Enable_Msk;
    CMD |= (SL_AHB_QSPI_SPI_CMD_N_RW_Bytes_Msk & (n_words << 16));
    SL_AHB_QSPI->SPI_CMD = CMD;

    SL_AHB_QSPI->SPI_CMD |= SL_AHB_QSPI_SPI_CMD_Enable_Msk;

    SPI_WAIT_BUSY(SL_AHB_QSPI);

}

void SET_QPI_MODE(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI){
    SL_AHB_QSPI->SPI_CMD = SET_QIO_MODE_CMD;
    SL_AHB_QSPI->SPI_CMD = 0x100 + SET_QIO_MODE_CMD;
    SPI_WAIT_BUSY(SL_AHB_QSPI);
    SL_AHB_QSPI->CTRL |= SL_AHB_QSPI_CTRL_QIO_Msk;
}

// Winbond W25Qxx: QE, enter QPI, 8 dummy clocks, EBh continuous read, XIP window (MBT-47)
static uint8_t spi_read_byte(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI, uint8_t op){
    SL_AHB_QSPI->SPI_CMD = op | SL_AHB_QSPI_SPI_CMD_Rd_Enable_Msk;
    SL_AHB_QSPI->SPI_CMD = op | SL_AHB_QSPI_SPI_CMD_Rd_Enable_Msk | SL_AHB_QSPI_SPI_CMD_Enable_Msk;
    SPI_WAIT_BUSY(SL_AHB_QSPI);
    return (uint8_t)(SL_AHB_QSPI->READ_DATA[0] >> 24);   // a 1-byte read lands in bits 31:24
}
static void spi_write_byte(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI, uint8_t op, uint8_t val){
    SL_AHB_QSPI->WRITE_DATA[0] = val;
    SL_AHB_QSPI->SPI_CMD = op | SL_AHB_QSPI_SPI_CMD_Wr_Enable_Msk;
    SL_AHB_QSPI->SPI_CMD = op | SL_AHB_QSPI_SPI_CMD_Wr_Enable_Msk | SL_AHB_QSPI_SPI_CMD_Enable_Msk;
    SPI_WAIT_BUSY(SL_AHB_QSPI);
}
int WINBOND_QPI_STARTUP(SL_AHB_QSPI_TypeDef *SL_AHB_QSPI){
    uint32_t CMD;
    if((spi_read_byte(SL_AHB_QSPI, 0x35) & 0x02) == 0){
        SL_AHB_QSPI->SPI_CMD = 0x50;                      // Volatile SR Write Enable
        SL_AHB_QSPI->SPI_CMD = 0x150;
        SPI_WAIT_BUSY(SL_AHB_QSPI);
        spi_write_byte(SL_AHB_QSPI, 0x31, 0x02);          // SR2 = QE
        if((spi_read_byte(SL_AHB_QSPI, 0x35) & 0x02) == 0){
            return 1;                                     // QE did not set: no QPI possible
        }
    }
    SL_AHB_QSPI->SPI_CMD = 0x38;                          // Enter QPI
    SL_AHB_QSPI->SPI_CMD = 0x138;
    SPI_WAIT_BUSY(SL_AHB_QSPI);
    SL_AHB_QSPI->CTRL |= SL_AHB_QSPI_CTRL_QIO_Msk;
    spi_write_byte(SL_AHB_QSPI, 0xC0, 0x30);              // Set Read Parameters: 8 dummy clocks
    // EBh + 24-bit address + mode A0h (continuous read) + 6 dummy clocks, 16 bytes
    SL_AHB_QSPI->CTRL |= SL_AHB_QSPI_CTRL_Cont_Rd_Msk | (SL_AHB_QSPI_CTRL_Mode_Code_Msk & (0xA0<<16));
    SL_AHB_QSPI->SPI_ADDR = 0;
    CMD = 0xEB | SL_AHB_QSPI_SPI_CMD_Rd_Enable_Msk | SL_AHB_QSPI_SPI_CMD_Addr_Enable_Msk
        | (SL_AHB_QSPI_SPI_CMD_N_Dummy_Msk & (6<<12)) | (SL_AHB_QSPI_SPI_CMD_N_RW_Bytes_Msk & (15<<16));
    SL_AHB_QSPI->SPI_CMD = CMD;
    SL_AHB_QSPI->SPI_CMD = CMD | SL_AHB_QSPI_SPI_CMD_Enable_Msk;
    SPI_WAIT_BUSY(SL_AHB_QSPI);
    SL_AHB_QSPI->AHB_CMD = (6<<SL_AHB_QSPI_SPI_CMD_N_Dummy_Pos) + 0xEB;
    SL_AHB_QSPI->CTRL |= (1<<25);                         // No_CMD: the chip is in continuous read
    QPI_SET_AHB_MODE(SL_AHB_QSPI);
    return 0;
}
