/*******************************************************************************
*                            (C) 2009 STMicroelectronics
*    Reproduction and Communication of this document is strictly prohibited
*      unless specifically authorized in writing by STMicroelectronics.
*-----------------------------------------------------------------------------
*                                  APG / CRM / SA&PD
*                   Software Development Group - SW platform & HW Specific
******************************************************************************/

/********************************************//**
 * \file XLoaderCLI.h
 * \brief This header provides all XLoader definitions.
 ***********************************************/

#if !defined( TP_UART_H )
#define TP_UART_H

/*****************************************************************************
   includes
*****************************************************************************/

#include <stdio.h>
#include "tp_common.h"
//#define dbg_printf(format,args...)   printf(format,args...)
#define dbg_printf(format,args...)

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   exported variables
*****************************************************************************/

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

extern tp_status_t    tp_uart_init              (tp_prgdev_uart_config_t *uart_cfg, tp_prgdev_instance_t portID);
extern tp_status_t    tp_uart_open              (tp_prgdev_instance_t inst);
extern tp_status_t    tp_uart_close             (tp_prgdev_instance_t inst);
extern tSize          tp_uart_read              (tPChar out_buf, const tSize chars, const tUInt timeout, tp_prgdev_instance_t inst);
extern tSize          tp_uart_write             (const tPChar in_buf, const tSize chars, tp_prgdev_instance_t inst);
extern tBool          tp_uart_getbaudrateidx    (const tUInt baudrate, tPU8 idx_ptr);
extern tp_status_t    tp_uart_setbaudrate       (const tUInt baudrate, tp_prgdev_instance_t inst);
extern tp_status_t    tp_uart_flush             (tp_prgdev_instance_t inst);
extern tp_status_t    tp_uart_flowcontrol       (boolean rts, boolean dtr, tUInt timing, tp_prgdev_instance_t inst);

#endif /* TP_UART_H */
