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

#if !defined( TP_COMMON_H )
#define TP_COMMON_H

/*****************************************************************************
   includes
*****************************************************************************/

#include "defines.h"
#include "typedefs.h"
#include "macros.h"
#include "clibs.h"

#include "trgt_common.h"

/*
  Include all xldr/fwupg headers to define all possible targets and
  relative configuration structures
*/
#include "fwupg/fwupg.h"
#include "teseo2/xldr_teseo2.h"
#include "teseo3/xldr_teseo3.h"
#include "teseo5/xldr_teseo5.h"

#ifdef MULTI_COMS
#include "pthread.h"
#endif

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

/* These macros are used for C string concatenation, no need for U qualifier */
#define TP_VERSION_MAJOR                2
#define TP_VERSION_MINOR                11
#define TP_VERSION_MINOR_2              2
#define TP_VERSION_MINOR_3              0

#define TP_STATUS_OK                    0U
#define TP_STATUS_ERROR                 1U

#define TP_CFG_UNKNOWN                  0U

#define TP_PRGDEVID_UART                1U
#define TP_PRGDEVID_I2C                 2U
#define TP_PRGDEVID_SPI                 3U

#define TP_PRGDEV_UART_DEFBR_FWUPG      115200U
#define TP_PRGDEV_UART_DEFBR_TESEO2     115200U
#define TP_PRGDEV_UART_DEFBR_TESEO3     115200U
#define TP_PRGDEV_UART_DEFBR_TESEO5     115200U

#define TP_TRGT2TP_ERROR(x)             ((x) == TRGT_STATUS_OK ? TP_STATUS_OK : TP_STATUS_ERROR)
#define TP_TP2TRGT_ERROR(x)             ((x) == TP_STATUS_OK ? TRGT_STATUS_OK : TRGT_STATUS_ERROR)

#define TP_VERSION_STRING               MCR_MACROTOSTRING(TP_VERSION_MAJOR) "." MCR_MACROTOSTRING(TP_VERSION_MINOR) "." MCR_MACROTOSTRING(TP_VERSION_MINOR_2)

/* Macro for warnings */
#define WRNG_STRINGIFY(x) #x
#define WRNG_TOSTRING(x) WRNG_STRINGIFY(x)
#define WRNG( txt ) __FILE__"(" WRNG_TOSTRING(__LINE__) ") : Warning: " txt

#define PARA_UNKNOWN '*'
#define PARA_UNKNOWN_SYNTAX ""
#define PARA_UNKNOWN_COMMENT ""

#ifndef MULTI_COMS
#define TP_COM_INSTANCE_MAX 1
#define LOCK_MANAGER   
#define UNLOCK_MANAGER 
#define LOCK_UART      
#define UNLOCK_UART  
#define LOCK_PRINT     
#define UNLOCK_PRINT 
#else
#define TP_COM_INSTANCE_MAX 6
extern pthread_mutex_t mutex_manager;
#define LOCK_MANAGER   pthread_mutex_lock(&mutex_manager);
#define UNLOCK_MANAGER pthread_mutex_unlock(&mutex_manager);
extern pthread_mutex_t mutex_uart;
#define LOCK_UART      pthread_mutex_lock(&mutex_uart);
#define UNLOCK_UART    pthread_mutex_unlock(&mutex_uart);
extern pthread_mutex_t mutex_print;
#define LOCK_PRINT     pthread_mutex_lock(&mutex_print);
#define UNLOCK_PRINT   pthread_mutex_unlock(&mutex_print);
#endif


#if defined _USRDLL
#define IS_ABORT(x)			if( GeneralAbort != 0 ) { x = TP_STATUS_ERROR; }
#else
#define IS_ABORT(x)
#endif

/* Status definition */
typedef enum
{
	STATE_IDLE,
	STATE_RUN,
	STATE_FAILED,
	STATE_ABORTED,
	STATE_SUCCESS
}Status_e;


typedef enum
{
	ITEM_INIT,
	ITEM_PORTCOM,
	ITEM_BINARY,
	ITEM_RESET_PIN,
	ITEM_PREAMBLE,
	ITEM_BIN_INFO,
	ITEM_FWUPDATECMD,
	ITEM_SYNC,
	ITEM_CHANGE_BAUD_RATE,
	ITEM_NVM,
	ITEM_FLASH_ERASE,
	ITEM_FLASH,
	ITEM_DUMP,
	ITEM_SET,
	ITEM_CRC,
	ITEM_OPERATION,
	Status_item_e_SIZE
}Status_item_e;

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

typedef tUInt           tp_status_t;        /* Return type for APIs */

typedef tUInt           tp_prgdev_id_t;     /* ID of the device used to program */

typedef tUInt           tp_prgdev_instance_t;/* device instance */

/* Programming device - UART specific */
typedef struct tp_prgdev_uart_config_s
{
	tPChar *              port;           /* name of the UART ports */
	tUInt                 nb_ports;       /* number of ports */
	tUInt                 def_br;         /* Standard baud rate of xloader/bootloader */
	tUInt                 prg_br;         /* Baud rate to be used during programming */
	tU8                   prg_br_index;   /* Index of baud rate (needed for old cuts) */
} tp_prgdev_uart_config_t;

/* Container for programming device */
typedef union tp_prgdev_params_s
{
	/* Here we could add I2C/SPI/... */
	tp_prgdev_uart_config_t   uart;
} tp_prgdev_params_t;

/* Programming device configuration */
typedef struct tp_prgdev_config_t
{
	tp_prgdev_id_t        dev;
	tp_prgdev_instance_t  inst;
	tp_prgdev_params_t    cfg;
} tp_prgdev_config_t;

/* General configuration */
typedef struct tp_general_config_s
{
	/* Command to be executed */
	trgt_command_t        cmd;

	/* Target core */
	trgt_core_t           target;

	/* I/O configuration */
	tBool                 quiet;
	tBool                 help;
	tBool                 script;
	tPChar                out_filename;
	tPChar                in_filename;
	tPChar			      PublicRSAKey;
	tPChar			      PrivateRSAKey;
	boolean               rts;
	boolean               dtr;
	tUInt                 reset_timing;

	/* Output stream configuration */
	tp_prgdev_config_t    prgdev_cfg;

	/* Input stream configuration */
	trgt_io_manager_t     io_mgr;
} tp_general_config_t;

/* Union of all xldr/fwupg targetted by this app */
typedef union tp_trgt_config_s
{
	fwupg_config_t        fwupg;
	xldr_teseo2_config_t  t2;
	xldr_teseo3_config_t  t3;
	xldr_teseo5_config_t  t5;
} tp_trgt_config_t;


/* Status */
typedef struct tp_progression_s
{
	// action is process on this topic
	tBool used;

	// action state on this topic, IDLE, RUNNING, PERFORMED, FAILED
	tUInt state;

	// percent of progression
	tUInt progression;

} tp_progression_t;

typedef struct tp_general_status_s
{
	tp_progression_t status[Status_item_e_SIZE];
} tp_general_status_t;

typedef struct tp_parameters_s
{
	const char option_letter;
	const tPChar Syntax;
	const tPChar Comment;
} tp_parameters_t;

/*****************************************************************************
   exported variables
*****************************************************************************/
extern tp_general_status_t tp_general_status;
extern boolean GeneralAbort;

/*****************************************************************************
   exported function prototypes
*****************************************************************************/

#endif /* TP_COMMON_H */
