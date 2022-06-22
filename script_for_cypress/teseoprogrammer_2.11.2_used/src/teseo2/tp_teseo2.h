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

#if !defined( TP_TESEO2_H )
#define TP_TESEO2_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "tp_common.h"
#include "teseo2/xldr_teseo2.h"

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   exported variables
*****************************************************************************/
extern const tp_parameters_t tp_t2_parameters[];

/*****************************************************************************
   exported function prototypes
*****************************************************************************/
extern tVoid        tp_teseo2_params_init     (xldr_teseo2_config_t *cfg);
extern tp_status_t  tp_teseo2_params_parse    (int argc, char* argv[], xldr_teseo2_config_t *cfg, tPBool arg_parsed);
extern tp_status_t  tp_teseo2_params_check    (trgt_command_t cmd, xldr_teseo2_config_t *cfg);
extern tVoid        tp_teseo2_params_usage    (tVoid);

extern tp_status_t  tp_teseo2_exec            (trgt_command_t cmd, tp_prgdev_instance_t inst, xldr_teseo2_config_t *trgt_cfg, trgt_io_manager_t *io_mgr, tp_prgdev_config_t *prg_cfg);

#endif /* TP_TESEO2_H */
