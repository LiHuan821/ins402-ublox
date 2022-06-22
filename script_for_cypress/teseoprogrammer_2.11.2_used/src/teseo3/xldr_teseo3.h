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

#if !defined( XLDR_TESEO3_H )
#define XLDR_TESEO3_H

/*****************************************************************************
   includes
*****************************************************************************/
#include "tp_common.h"
#include "trgt_common.h"

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define TRGT_VARIANT_STA8089FG        1U
#define TRGT_VARIANT_STA8089FGB       2U
#define TRGT_VARIANT_STA8089FGBD      3U
#define TRGT_VARIANT_STA8089G         4U
#define TRGT_VARIANT_STA8089GR        5U
#define TRGT_VARIANT_STA8089GA        6U
#define TRGT_VARIANT_STA8089GAY       7U
#define TRGT_VARIANT_STA8090EXG       8U
#define TRGT_VARIANT_STA8090EXGA      9U
#define TRGT_VARIANT_STA8090F4G       10U
#define TRGT_VARIANT_STA8090F4GB      11U
#define TRGT_VARIANT_STA8090F4GY      12U
#define TRGT_VARIANT_STA8090FG        13U
#define TRGT_VARIANT_STA8090FGB       14U
#define TRGT_VARIANT_STA8090FGBD      15U
#define TRGT_VARIANT_STA8090FGY       16U
#define TRGT_VARIANT_STA8090GA        17U
#define TRGT_VARIANT_STA8090GAT       18U
#define TRGT_VARIANT_STA8090WG        19U
#define TRGT_VARIANT_STA8090WGR       20U
#define TRGT_VARIANT_STA8089FGA       21U

#define TRGT_VARIANT_STA8090_NUMBER   21U

#define XLDR_TESEO3_FLASHERVER_OFFSET       0x60U

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

typedef tUInt xldr_teseo3_inst;

/* Target configuration */
typedef struct xldr_teseo3_config_s
{
  tUInt     variant;
  tUInt     mem;
  tUInt     destaddr;
  tUInt     desterasesize;
  tUInt     entry;
  tBool     nvm_erase;
  tUInt     nvm_offset;
  tUInt     nvm_size;
  tBool     program_only;
  tBool     subsector;
  tBool     restart;
  tUInt     baudrate;
  tUInt     baudrate_idx;
  tU8		debug;
  tU32		debugAction;
  tU32		debugAddress;
  tInt		debugSize;
  tU32		debugData;
  tPChar	PrivateRSAKey;
  tPChar	PublicRSAKey;
} xldr_teseo3_config_t;

typedef struct xldr_teseo3_binimg_info_s
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
} xldr_teseo3_binimg_info_t;

typedef struct xldr_teseo3_preamble_s
{
	tUInt identifier_msp;
	tUInt identifier_lsp;
	tUInt options;
	tUInt destinationAddress;
} xldr_teseo3_preamble_t;

typedef struct xldr_teseo3_bootloader_s
{
	tPChar  buf_ptr;
	tUInt   size;
	tUInt   entry;
	tUInt   crc32;
} xldr_teseo3_bootloader_t;

typedef struct xldr_teseo3_manager_s
{
	xldr_teseo3_inst          instance;
	trgt_io_manager_t *           io_mgr;
	xldr_teseo3_binimg_info_t     binimg_info;
	xldr_teseo3_preamble_t        preamble;
	xldr_teseo3_bootloader_t      bootloader;
} xldr_teseo3_manager_t;

/*****************************************************************************
   exported variables
*****************************************************************************/

extern unsigned char xldr_teseo3_bootloader[];
  
/*****************************************************************************
   exported function prototypes
*****************************************************************************/

extern trgt_status_t  xldr_teseo3_init            (trgt_command_t cmd, xldr_teseo3_inst inst, xldr_teseo3_config_t *mgr, trgt_io_manager_t *io_mgr, xldr_teseo3_manager_t ** xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_sync            (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_send_baudrate   (xldr_teseo3_manager_t * xldr_teseo3_manager,tUInt baud_rate);
extern trgt_status_t  xldr_teseo3_check_baudrate  (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_check_hostready (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_send_preamble   (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_send_bootloader (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_send_bininfo    (xldr_teseo3_manager_t * xldr_teseo3_manager, const tBool waitForAck);
extern trgt_status_t  xldr_teseo3_wait_devinit    (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_wait_eraseprog  (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_wait_erasenvm   (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_send_buffer     (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_check_crc       (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_dump            (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern trgt_status_t  xldr_teseo3_set             (xldr_teseo3_manager_t * xldr_teseo3_manager);
extern tVoid          xldr_teseo3_shutdown        (xldr_teseo3_manager_t * xldr_teseo3_manager);

#endif /* XLDR_TESEO3_H */
