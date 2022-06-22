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

#if !defined( CRC32_H )
#define CRC32_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "defines.h"
#include "typedefs.h"

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define CRC32_EVAL_BUF(d,c,b,l)       d = crc32_eval(c,b,l)
#define CRC32_EVAL_STR(d,c,s,l)       d = crc32_eval(c,s,l)
#define CRC32_EVAL_INT(d,c,i)         { tUInt val = (i); (d) = crc32_eval(c,&val,sizeof(val)); }

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   exported variables
*****************************************************************************/

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

extern tUInt crc32_eval(tUInt crc32val, const tVoid *void_ptr, const tSize len);

#endif /* CRC32_H */
