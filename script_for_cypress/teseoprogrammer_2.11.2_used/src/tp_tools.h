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

#if !defined( TP_TOOLS_H )
#define TP_TOOLS_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "typedefs.h"

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define   TP_TOOLS_PARSE_FMT_BOOL     1U
#define   TP_TOOLS_PARSE_FMT_UINT     2U
#define   TP_TOOLS_PARSE_FMT_HEX      3U

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

typedef tUInt         tp_tools_parse_fmt_t;
typedef tInt          tp_tools_time_t;

/*****************************************************************************
   exported variables
*****************************************************************************/

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

/* String tools */
#if defined(__cplusplus)
extern "C"{
#endif

extern tVoid            tp_tools_tolower        (tPChar dest_str, const tPChar src_str);
extern tVoid            tp_tools_toupper        (tPChar dest_str, const tPChar src_str);
extern tPChar           tp_tools_lcstr_new      (const tPChar src_str);
extern tVoid            tp_tools_lcstr_delete   (tPChar memptr);
extern tp_status_t      tp_tools_parse_val      (const tPChar src_str, const tp_tools_parse_fmt_t src_format, tVoid *value_ptr);
extern tp_status_t      tp_tools_parse_valhex_valhex(const tPChar src_str, tUInt *value_ptr1, tUInt *value_ptr2);
extern tp_tools_time_t  tp_tools_gettime        (tVoid);
extern tVoid            tp_tools_delay          (const tInt ms);
extern tVoid            tp_tools_remove_endline (tPChar * line);
extern tUInt            tp_tools_split          (tPChar str, tPChar ** SplitStrings, tChar Separator);

#if defined(__cplusplus)
}
#endif

#endif /* TP_TOOLS_H */
