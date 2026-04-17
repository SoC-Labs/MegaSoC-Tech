#ifndef MEGASOC_RESETCTRL_H
#define MEGASOC_RESETCTRL_H

#include <stdint.h>
#include "sys_memory_map.h"

typedef struct {
    volatile uint32_t RESET_REQ;     // 0x00
    volatile uint32_t RESET_STATUS;  // 0x04
} MEGASOC_RESETCTRL_TypeDef;


#define RESET_REQ_SWRESET_Pos              0                                              
#define RESET_REQ_SWRESET_Msk            (0x01ul << RESET_REQ_SWRESET_Pos)

#define RESET_REQ_WDOG_Pos              1                                             
#define RESET_REQ_WDOG_Msk            (0x01ul << RESET_REQ_WDOG_Pos)

#define RESET_REQ_DBG_Pos              2                                              
#define RESET_REQ_DBG_Msk            (0x01ul << RESET_REQ_DBG_Pos)

#define RESET_REQ_LOCKUP_Pos              3                                             
#define RESET_REQ_LOCKUP_Msk            (0x01ul << RESET_REQ_LOCKUP_Pos)

#define RESET_STATUS_SWRESET_Pos              0                                              
#define RESET_STATUS_SWRESET_Msk            (0x01ul << RESET_STATUS_SWRESET_Pos)

#define RESET_STATUS_WDOG_Pos              1                                             
#define RESET_STATUS_WDOG_Msk            (0x01ul << RESET_STATUS_WDOG_Pos)

#define RESET_STATUS_DBG_Pos              2                                              
#define RESET_STATUS_DBG_Msk            (0x01ul << RESET_STATUS_DBG_Pos)

#define RESET_STATUS_LOCKUP_Pos              3                                             
#define RESET_STATUS_LOCKUP_Msk            (0x01ul << RESET_STATUS_LOCKUP_Pos)

#define RESET_STATUS_POR_Pos              4                                             
#define RESET_STATUS_POR_Msk            (0x01ul << RESET_STATUS_POR_Pos)

#define RESET_STATUS_ALL_Msk            (RESET_STATUS_SWRESET_Msk | RESET_STATUS_WDOG_Msk | RESET_STATUS_DBG_Msk | RESET_STATUS_LOCKUP_Msk | RESET_STATUS_POR_Msk)

#define MEGASOC_RESETCTRL           ((MEGASOC_RESETCTRL_TypeDef  *) SYS_RESETCTRL_BASE  )

#endif //MEGASOC_RESETCTRL_H
