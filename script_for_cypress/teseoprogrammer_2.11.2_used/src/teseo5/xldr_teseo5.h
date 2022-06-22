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

#if !defined( XLDR_TESEO5_H )
#define XLDR_TESEO5_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "trgt_common.h"

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define TRGT_VARIANT_UNKNOWN          0U
#define TRGT_VARIANT_STA8100_CUT1     1U
#define TRGT_VARIANT_STA8100_CUT2     2U
#define TRGT_VARIANT_STA8100_NUMBER   3U

#define XLDR_TESEO5_FLASHERVER_OFFSET       0x60U

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

typedef tUInt xldr_teseo5_inst;

/* Target configuration */
typedef struct xldr_teseo5_config_s
{
  tUInt     variant;
  tUInt     mem;
  tUInt     destaddr;
  tUInt     entry;
  tU8     nvm_erase;
  tUInt     nvm_offset;
  tUInt     nvm_size;
  tBool     program_only;
  tBool     subsector;
  tBool     restart;
  tUInt     baudrate;
  tUInt     baudrate_idx;
  tU8		  debug;
  tU32		debugAction;
  tU32		debugAddress;
  tInt		debugSize;
  tU32		debugData;
  tPChar	PrivateRSAKey;
  tPChar	PublicRSAKey;
} xldr_teseo5_config_t;

typedef struct xldr_teseo5_binimg_info_s
{
	tInt  fileSize;
	tU32  bootMode;
	tU32  crc32;
	tInt  destinationAddress;
	tInt  entryPoint;
	tU8   eraseNVM;
	tU8   eraseOnly;
	tU8   programOnly;
	tU8   subSector;
	tU8   sta8090fg;
	tU8   res1;
	tU8   res2;
	tU8   res3;
	tInt  nvmOffset;
	tInt  nvmEraseSize;
	tU32  debug;
	tU32  debugAction;
	tU32  debugAddress;
	tInt  debugSize;
	tU32  debugData;
} xldr_teseo5_binimg_info_t;

typedef struct xldr_teseo5_preamble_s
{
	tUInt identifier_msp;
	tUInt identifier_lsp;
	tUInt options;
	tUInt destinationAddress;
} xldr_teseo5_preamble_t;

typedef struct xldr_teseo5_bootloader_s
{
	tPChar  buf_ptr;
	tUInt   size;
	tUInt   entry;
	tUInt   crc32;
} xldr_teseo5_bootloader_t;

typedef struct xldr_teseo5_manager_s
{
	xldr_teseo5_inst              instance;
	trgt_io_manager_t *           io_mgr;
	xldr_teseo5_binimg_info_t     binimg_info;
	xldr_teseo5_preamble_t        preamble;
	xldr_teseo5_bootloader_t      bootloader;
} xldr_teseo5_manager_t;

/*****************************************************************************
   exported variables
*****************************************************************************/
extern unsigned char xldr_teseo5_bootloader_cut2[];

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

extern trgt_status_t  xldr_teseo5_init            (trgt_command_t cmd, xldr_teseo5_inst inst, xldr_teseo5_config_t *mgr, trgt_io_manager_t *io_mgr, xldr_teseo5_manager_t ** xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_sync            (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_send_baudrate   (xldr_teseo5_manager_t * xldr_teseo5_manager,tUInt baud_rate);
extern trgt_status_t  xldr_teseo5_check_baudrate  (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_check_hostready (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_send_preamble   (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_send_bootloader (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_send_bininfo    (xldr_teseo5_manager_t * xldr_teseo5_manager, const tBool waitForAck);
extern trgt_status_t  xldr_teseo5_wait_devinit    (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_wait_eraseprog  (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_wait_erasenvm   (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_send_buffer     (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_check_crc       (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_dump            (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern trgt_status_t  xldr_teseo5_set             (xldr_teseo5_manager_t * xldr_teseo5_manager);
extern tVoid          xldr_teseo5_shutdown        (xldr_teseo5_manager_t * xldr_teseo5_manager);

#endif /* XLDR_TESEO5_H */
