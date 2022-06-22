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

#if !defined( XLDR_TESEO2_H )
#define XLDR_TESEO2_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "trgt_common.h"

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define TRGT_VARIANT_STA8088          1U

#define TRGT_VARIANT_STA8088_NUMBER   1U

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/
typedef tUInt xldr_teseo2_inst;

/* Target configuration */
typedef struct xldr_teseo2_config_s
{
  tUInt     variant;
  tUInt     mem;
  tUInt     destaddr;
  tUInt     entry;
  tBool     nvm_erase;
  tUInt     nvm_offset;
  tUInt     nvm_size;
  tBool     program_only;
  tBool     subsector;
  tBool     restart;
  tUInt     baudrate;
  tU8		debug;
  tU32		debugAction;
  tU32		debugAddress;
  tInt		debugSize;
  tU32		debugData;
} xldr_teseo2_config_t;

typedef struct xldr_teseo2_binimg_info_s
{
	tInt  fileSize;
	tU32  bootMode;
	tU32  crc32;
	tInt  destinationAddress;
	tInt  entryPoint;
	tInt  baudRate;
	tU8   eraseNVM;
	tU8   eraseOnly;
	tU8   programOnly;
	tU8   debug;
	tInt  nvmOffset;
	tInt  nvmEraseSize;
	tU32  debugAction;
	tU32  debugAddress;
	tInt  debugSize;
	tU32  debugData;
} xldr_teseo2_binimg_info_t;

typedef struct xldr_teseo2_preamble_s
{
	tUInt identifier_msp;
	tUInt identifier_lsp;
	tUInt options;

	tUInt destinationAddress;
} xldr_teseo2_preamble_t;

typedef struct xldr_teseo2_bootloader_s
{
	tPChar  buf_ptr;
	tUInt   size;
	tUInt   entry;
	tUInt   crc32;
} xldr_teseo2_bootloader_t;

typedef struct xldr_teseo2_manager_s
{
	xldr_teseo2_inst              instance;
	trgt_io_manager_t *           io_mgr;
	xldr_teseo2_binimg_info_t     binimg_info;
	xldr_teseo2_preamble_t        preamble;
	xldr_teseo2_bootloader_t      bootloader;
} xldr_teseo2_manager_t;

/*****************************************************************************
   exported variables
*****************************************************************************/

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

extern trgt_status_t  xldr_teseo2_init            (trgt_command_t cmd, xldr_teseo2_inst inst, xldr_teseo2_config_t *mgr, trgt_io_manager_t *io_mgr, xldr_teseo2_manager_t ** xldr_teseo2_manager);
extern trgt_status_t  xldr_teseo2_check_baudrate  (xldr_teseo2_manager_t *mgr);
extern trgt_status_t  xldr_teseo2_check_hostready (xldr_teseo2_manager_t *mgr);
extern trgt_status_t  xldr_teseo2_send_preamble   (xldr_teseo2_manager_t *mgr);
extern trgt_status_t  xldr_teseo2_send_bootloader (xldr_teseo2_manager_t *mgr);
extern trgt_status_t  xldr_teseo2_send_bininfo    (xldr_teseo2_manager_t *mgr,const tBool waitForAck);
extern trgt_status_t  xldr_teseo2_wait_devinit    (xldr_teseo2_manager_t *mgr);
extern trgt_status_t  xldr_teseo2_wait_eraseprog  (xldr_teseo2_manager_t *mgr);
extern trgt_status_t  xldr_teseo2_wait_erasenvm   (xldr_teseo2_manager_t *mgr);
extern trgt_status_t  xldr_teseo2_send_buffer     (xldr_teseo2_manager_t *mgr);
extern trgt_status_t  xldr_teseo2_check_crc       (xldr_teseo2_manager_t *mgr);
extern trgt_status_t  xldr_teseo2_dump            (xldr_teseo2_manager_t *mgr);
extern trgt_status_t  xldr_teseo2_set             (xldr_teseo2_manager_t *mgr);
extern tVoid          xldr_teseo2_shutdown        (xldr_teseo2_manager_t *mgr);

#endif /* XLDR_TESEO2_H */
