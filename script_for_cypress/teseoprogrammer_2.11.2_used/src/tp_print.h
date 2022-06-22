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

#if !defined( TP_PRINT_H )
#define TP_PRINT_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "typedefs.h"
#include "clibs.h"

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define TP_PRINT_INVALID    0U
#define TP_PRINT_QUIET      1U
#define TP_PRINT_STDMSG     2U
#define TP_PRINT_ERROR      3U
#define TP_PRINT_DEBUG      4U
#define TP_PRINT_STREAM     5U

#if 0
#define TP_PRINT(x)         _clibs_printf("Func %s: ",__FUNCTION__), tp_print_settype(TP_PRINT_STDMSG), tp_print x
#define TP_ERROR(x)         _clibs_printf("Func %s: ",__FUNCTION__), tp_print_settype(TP_PRINT_ERROR ), tp_print x
#define TP_INFO(x)          _clibs_printf("Func %s: ",__FUNCTION__), tp_print_settype(TP_PRINT_DEBUG ), tp_print x
#define TP_FATAL_ERROR(x)   _clibs_printf("Func %s: ",__FUNCTION__), _clibs_printf x
#define TP_PRINT_BUF(x,y,z) _clibs_printf("Func %s: ",__FUNCTION__), tp_print_buffer(x, y, z)
#define TP_STREAM(x)        _clibs_printf("Func %s: ",__FUNCTION__), _clibs_printf x

#else
#define TP_PRINT(x)         tp_print_settype(TP_PRINT_STDMSG), tp_print x
#define TP_ERROR(x)         tp_print_settype(TP_PRINT_ERROR ), tp_print x
#define TP_INFO(x)          tp_print_settype(TP_PRINT_DEBUG ), tp_print x
#define TP_STREAM(x)        tp_print_settype(TP_PRINT_STREAM ), tp_print x
#define TP_FATAL_ERROR(x)   _clibs_printf x
#define TP_PRINT_BUF(x,y,z) tp_print_buffer(x, y, z)
#endif

//#define TP_ENDL             "\r\n"
#define TP_ENDL             "\n"

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   exported variables
*****************************************************************************/

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

extern tp_status_t    tp_print_init     (tp_general_config_t *gen_cfg);
extern tVoid          tp_print          (const tPChar msg, ...);
extern tVoid          tp_print_settype  (const tU8 type);
extern tVoid          tp_print_shutdown (tVoid);
extern tVoid          tp_print_version  (tVoid);
extern tVoid          tp_print_buffer   (tPChar topic, tPChar buffer, tU16 size);

#endif /* TP_PRINT_H */
