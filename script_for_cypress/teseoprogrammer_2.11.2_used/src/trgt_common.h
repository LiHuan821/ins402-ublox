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

#if !defined( TRGT_COMMON_H )
#define TRGT_COMMON_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "defines.h"
#include "typedefs.h"

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define TRGT_STATUS_OK              0U
#define TRGT_STATUS_ERROR           1U

#define TRGT_CFG_UNKNOWN            0U

#define TRGT_CMD_PROG               1U
#define TRGT_CMD_ERASE              2U
#define TRGT_CMD_SET                3U
#define TRGT_CMD_DUMP               4U
#define TRGT_CMD_FWUPG              5U

#define TRGT_FAMILY_TESEO2          1U
#define TRGT_FAMILY_TESEO3          2U
#define TRGT_FAMILY_TESEO5          3U

#define TRGT_MEM_SQI                1U
#define TRGT_MEM_NOR                2U
#define TRGT_MEM_RAM                3U

#define TRGT_DEBUG_OFF				0U
#define TRGT_DEBUG_ON				1U

#define TRGT_DEBUG_DUMP				0U
#define TRGT_DEBUG_SET				1U

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

typedef tUInt         trgt_status_t;                /* Return type for APIs */

typedef tUInt         trgt_command_t;               /* Command to be executed */

typedef tUInt         trgt_core_t;                  /* Target core */

typedef tUInt         trgt_dev_instance_t;          /* device instance */

/* Callbacks for xldr/fwupg to interface the core */
typedef trgt_status_t   (*trgt_dev_read_callback_t)     (tPChar out_buf, tSize chars, tUInt timeout, trgt_dev_instance_t inst);
typedef trgt_status_t   (*trgt_dev_write_callback_t)    (tPChar in_buf, tSize chars, trgt_dev_instance_t inst);

/* Callback to get time in ms */
typedef tInt            (*trgt_gettime_t)               (tVoid);

typedef struct trgt_io_manager_s
{
union {
		tPChar                      src_buf;
		tPChar                      out_buf;
	 };
union {
	tSize                       src_size;
	tSize                       out_size;
	  };

  FILE *						out_file;
  tPChar						public_rsa_key;	   /* RSA public key */
  tSize                         public_rsa_size;
  tPChar						private_rsa_key;   /* RSA private key */
  tSize                         private_rsa_size;

  tUInt                       src_crc32;

  trgt_dev_read_callback_t    dev_read_cb;
  trgt_dev_write_callback_t   dev_write_cb;
  trgt_dev_instance_t         instance_device;

  trgt_gettime_t              gettime;
} trgt_io_manager_t;

/*****************************************************************************
   exported variables
*****************************************************************************/

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

#endif /* TRGT_COMMON_H */
