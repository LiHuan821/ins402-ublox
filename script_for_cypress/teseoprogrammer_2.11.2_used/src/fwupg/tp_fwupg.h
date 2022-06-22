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

#if !defined( TP_FWUPG_H )
#define TP_FWUPG_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "tp_common.h"
#include "fwupg/fwupg.h"

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/


/*****************************************************************************
   exported variables
*****************************************************************************/
extern const tp_parameters_t tp_upgrade_parameters[];

/*****************************************************************************
   exported function prototypes
*****************************************************************************/
extern tVoid        tp_fwupg_params_init    (fwupg_config_t *cfg);
extern tp_status_t  tp_fwupg_params_parse   (int argc, char* argv[], fwupg_config_t *cfg, tPBool arg_parsed);
extern tp_status_t  tp_fwupg_params_check   (fwupg_config_t *cfg);
extern tVoid        tp_fwupg_params_usage   (tVoid);

extern tp_status_t  tp_fwupg_exec           (fwupg_config_t *trgt_cfg, tp_prgdev_instance_t inst, trgt_io_manager_t *io_mgr, tp_prgdev_config_t *prg_cfg);

#endif /* TP_FWUPG_H */
