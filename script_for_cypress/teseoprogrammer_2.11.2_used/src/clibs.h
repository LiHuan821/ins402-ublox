/*******************************************************************************
 *                            (C) 2009 STMicroelectronics
 *    Reproduction and Communication of this document is strictly prohibited
 *      unless specifically authorized in writing by STMicroelectronics.
 *-----------------------------------------------------------------------------
 *                                  APG / CRM / SA&PD
 *                   Software Development Group - SW platform & HW Specific
 ******************************************************************************/

/********************************************//**
 * \file clibs.h
 * \brief This header provides retargetting APIs for C standard libs.
 ***********************************************/

#ifndef CLIBS_H
#define CLIBS_H

/*****************************************************************************
   includes
*****************************************************************************/

#if defined( CLIBS_RETARGETING )
#include "typedefs.h"
#else
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#if !defined( CLIBS_RETARGETING )
// climits.h retargeting
#define _clibs_INT_MAX()      INT_MAX

// ctypes.h retargeting
#define _clibs_tolower        tolower
#define _clibs_toupper        toupper

// stdio.h retargeting
#define _clibs_printf         printf
#define _clibs_sprintf        sprintf
#define _clibs_snprintf       snprintf
#define _clibs_sscanf         sscanf

// stdlib.h retargeting
#define _clibs_div_t          div_t
#define _clibs_abs            abs
#define _clibs_div            div
#define _clibs_malloc         malloc
#define _clibs_calloc         calloc
#define _clibs_free           free

// string.h retargeting
#define _clibs_memcmp         memcmp
#define _clibs_memcpy         memcpy
#define _clibs_memmove        memmove
#define _clibs_memset         memset
#define _clibs_strchr         strchr
#define _clibs_strcmp         strcmp
#define _clibs_strcpy         strcpy
#define _clibs_strncpy        strncpy
#define _clibs_strlen         strlen
#define _clibs_strncmp        strncmp
#endif

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

#if defined( CLIBS_RETARGETING )
typedef struct
{
  tInt quot;
  tInt rem;
} _clibs_div_t;
#endif

/*****************************************************************************
   exported variables
*****************************************************************************/

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

#if defined( CLIBS_RETARGETING )
extern  tInt         _clibs_INT_MAX  ( void);

// stdio.h retargeting
extern  tInt         _clibs_printf   ( const tChar *, ...);
extern  tInt         _clibs_sprintf  ( tChar *, const tChar *, ...);
extern  tInt         _clibs_snprintf(tChar *, tInt, const tChar *, ...);
extern  tInt         _clibs_sscanf   ( const tChar *, const tChar *, ...);

// stdlib.h retargeting
extern  tInt         _clibs_abs      ( tInt);
extern  _clibs_div_t _clibs_div      ( tInt, tInt);
extern  void *       _clibs_malloc   ( tSize);
extern  void         _clibs_free     ( void *);

// string.h retargeting
extern  tInt         _clibs_memcmp   ( const void *, const void *, tSize);
extern  void *       _clibs_memcpy   ( void *, const void *, tSize);
extern  void *       _clibs_memmove  ( void *, const void *, tSize);
extern  void *       _clibs_memset   ( void *, tInt, tSize);
extern  tChar *      _clibs_strchr   ( const tChar *, tInt);
extern  tChar *      _clibs_strcpy   ( tChar *, const tChar *);
extern  tChar *      _clibs_strncpy  ( tChar *, const tChar *, tSize);
extern  tSize        _clibs_strlen   ( const tChar *);
extern  tInt         _clibs_strncmp  ( const tChar *, const tChar *, tSize);
#endif

#endif /* CLIBS_H */

