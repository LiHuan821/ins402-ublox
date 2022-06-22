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

#if !defined( TP_PARAMS_H )
#define TP_PARAMS_H

 /*****************************************************************************
	includes
 *****************************************************************************/

#include "tp_common.h"

 /*****************************************************************************
	defines and macros (scope: module-local)
 *****************************************************************************/

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   exported variables
*****************************************************************************/
extern const tp_parameters_t tp_common_parameters[];

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

extern tp_status_t    tp_params_parse(int argc, char* argv[], tp_general_config_t *gen_cfg, tPBool arg_parsed);
extern tp_status_t    tp_params_check(tp_general_config_t *gen_cfg);
extern tVoid          tp_params_usage(tPChar exename);

#endif /* TP_PARAMS_H */
