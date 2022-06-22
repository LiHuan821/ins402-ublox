/*******************************************************************************
*                            (C) 2018 STMicroelectronics
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

#if !defined( TP_TESEO5_H )
#define TP_TESEO5_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "tp_common.h"
#include "teseo5/xldr_teseo5.h"

/*****************************************************************************
defines and macros (scope: module-local)
*****************************************************************************/

/*****************************************************************************
typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
exported variables
*****************************************************************************/
extern const tp_parameters_t tp_t5_parameters[];

/*****************************************************************************
   exported function prototypes
*****************************************************************************/
extern tVoid        tp_teseo5_params_init     (xldr_teseo5_config_t *cfg);
extern tp_status_t  tp_teseo5_params_parse    (int argc, char* argv[], xldr_teseo5_config_t *cfg, tPBool arg_parsed);
extern tp_status_t  tp_teseo5_params_check    (trgt_command_t cmd, xldr_teseo5_config_t *cfg);
extern tVoid        tp_teseo5_params_usage    (tVoid);

extern tp_status_t  tp_teseo5_exec            (trgt_command_t cmd, tp_prgdev_instance_t inst, xldr_teseo5_config_t *trgt_cfg, trgt_io_manager_t *io_mgr, tp_prgdev_config_t *prg_cfg);

#endif /* TP_TESEO5_H */
