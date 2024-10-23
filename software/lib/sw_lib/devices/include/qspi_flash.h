#include <stdint.h>

extern void qspi_enable_cache();
extern void qspi_xip_enable();
extern int32_t SPI_READ_JEDIC();
extern void spi_reset();
extern void SET_QPI_MODE();
extern int32_t QPI_READ_JEDIC();


#define QSPI_CONFIG_BASEADDR (0x01000000UL)

/*------------- SL AHB QSPI --------------------------------------*/
/** @addtogroup SL AHB QSPI
  @{
*/
typedef struct
{
  volatile  uint32_t  QSPI_CONTROL;
  volatile  uint32_t  QSPI_STATUS;
  volatile  uint32_t  QSPI_CMD;
  volatile  uint32_t  QSPI_ADDR;
  volatile  uint32_t  QSPI_RDATA0;
  volatile  uint32_t  QSPI_RDATA1;
  volatile  uint32_t  QSPI_RDATA2;
  volatile  uint32_t  QSPI_RDATA3;
  volatile  uint32_t  QSPI_WDATA0;
  volatile  uint32_t  QSPI_WDATA1;
  volatile  uint32_t  QSPI_WDATA2;
  volatile  uint32_t  QSPI_WDATA3;
} AHB_QSPI_TypeDef;

#define SL_AHB_QSPI ((AHB_QSPI_TypeDef *) QSPI_CONFIG_BASEADDR)

#define QSPI_CACHE_CONFIG_ADDR (0x01001000UL)

/*------------- Arm Flash Cache --------------------------------------*/
/** @addtogroup Arm Flash Cache
  @{
*/
typedef struct
{
  volatile  uint32_t  CCR;
  volatile  uint32_t  SR;
  volatile  uint32_t  IRQMASK;
  volatile  uint32_t  IRQSTAT;
  volatile  uint32_t  HWPARAMS;
  volatile  uint32_t  CSHR;
  volatile  uint32_t  CSMR;
} Arm_Flash_Cache_TypeDef;

#define CACHE_CTRL ((Arm_Flash_Cache_TypeDef *) QSPI_CACHE_CONFIG_ADDR)

#define QSPI_DATA_BASEADDR (0x00400000UL)

typedef struct
{
  volatile uint32_t DATA[512];
} QSPI_CACHE_TypeDef;

#define QSPI_CACHE ((QSPI_CACHE_TypeDef *) QSPI_DATA_BASEADDR)
