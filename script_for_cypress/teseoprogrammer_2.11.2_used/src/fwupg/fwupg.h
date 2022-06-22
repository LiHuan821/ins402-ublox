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

#if !defined( FWUPG_H )
#define FWUPG_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "trgt_common.h"

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/
typedef tUInt fwupg_instance_t;

typedef struct fwupg_imageinfo_s
{
	tU8   eraseNVM;
	tU8   programOnly;
	tU8   dump;
	tU8   baudRate;
	tUInt firmwareSize;
	tUInt firmwareCRC;
	tUInt nvmAddressOffset;
	tUInt nvmSize;
} fwupg_imageinfo_t;

typedef struct fwupg_manager_s
{
	trgt_io_manager_t *           io_mgr;
	fwupg_imageinfo_t             image_info;
	fwupg_instance_t              instance;
	tBool                         recovery;   
} fwupg_manager_t;


/* Target configuration */
typedef struct fwupg_config_s
{
  tUInt     br_nmea;
  tU8		br_fwupg_idx;
  tUInt     nvm_size;
  tUInt     nvm_offset;
  tBool     nvm_erase;
  tBool     program_only;
  tBool     recovery;
} fwupg_config_t;

/*****************************************************************************
   exported variables
*****************************************************************************/

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

extern trgt_status_t  fwupg_init                  (fwupg_instance_t  inst, fwupg_config_t *mgr, trgt_io_manager_t *io_mgr, fwupg_manager_t ** pfwupg_manager);
extern trgt_status_t  fwupg_send_fwupgcmd         (fwupg_manager_t * fwupg_manager);
extern trgt_status_t  fwupg_sync                  (fwupg_manager_t * fwupg_manager);
extern trgt_status_t  fwupg_start_comm            (fwupg_manager_t * fwupg_manager);
extern trgt_status_t  fwupg_send_imageinfo        (fwupg_manager_t * fwupg_manager);
extern trgt_status_t  fwupg_wait_changebaudrate   (fwupg_manager_t * fwupg_manager);
extern trgt_status_t  fwupg_wait_devinit          (fwupg_manager_t * fwupg_manager);
extern trgt_status_t  fwupg_wait_eraseprog        (fwupg_manager_t * fwupg_manager);
extern trgt_status_t  fwupg_wait_erasenvm         (fwupg_manager_t * fwupg_manager);
extern trgt_status_t  fwupg_send_buffer           (fwupg_manager_t * fwupg_manager);
extern trgt_status_t  fwupg_check_crc             (fwupg_manager_t * fwupg_manager);
extern tVoid          fwupg_shutdown              (fwupg_manager_t * fwupg_manager);

#endif /* FWUPG_H */
