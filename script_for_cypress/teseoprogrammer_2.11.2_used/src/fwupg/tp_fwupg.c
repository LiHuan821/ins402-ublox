/*******************************************************************************
*                            (C) 2009 STMicroelectronics
*    Reproduction and Communication of this document is strictly prohibited
*      unless specifically authorized in writing by STMicroelectronics.
*-----------------------------------------------------------------------------
*                                  APG / CRM / SA&PD
*                   Software Development Group - SW platform & HW Specific
******************************************************************************/

/*****************************************************************************
includes
*****************************************************************************/

#include <stdio.h>

#include "tp_common.h"
#include "tp_report.h"
#include "tp_uart.h"
#include "tp_tools.h"
#include "tp_print.h"
#include "fwupg/tp_fwupg.h"

/*****************************************************************************
external declarations
*****************************************************************************/

/*****************************************************************************
defines and macros (scope: module-local)
*****************************************************************************/

#define MANAGER_STANDARD(x)   TP_PRINT(x)     
#define MANAGER_ERROR(x)      TP_ERROR(x)  
#define MANAGER_DEBUG(x)      TP_INFO(x)     

#define PARA_NMEA_DEVICE_SPEED 'k'
#define PARA_NMEA_DEVICE_SPEED_SYNTAX "SPEED_LOAD;19200 to 921600;115200"
#define PARA_NMEA_DEVICE_SPEED_COMMENT "baudrate to send FW config command on NMEA port (Default; 115200) [u]"

#define PARA_ERASE_NVM 'e'
#define PARA_ERASE_NVM_SYNTAX "ERASE_NVM;TRUE|FALSE;FALSE"
#define PARA_ERASE_NVM_COMMENT "true if application must erase NVM flash memory (Default; false) [u]"

#define PARA_NVM_OFFSET 'n'
#define PARA_NVM_OFFSET_SYNTAX "OFFSET_NVM;Hexadecimal address;0x100000"
#define PARA_NVM_OFFSET_COMMENT "NVM offset address (Default 0x100000) [u]"

#define PARA_NVM_SIZE 'z'
#define PARA_NVM_SIZE_SYNTAX "SIZE_NVM;decimal format;1024"
#define PARA_NVM_SIZE_COMMENT "NVM flash memory size expressed in kilobytes. Do not use if unsure (Default; 1024) [u]"

#define PARA_PROGRAM_ONLY 'p'
#define PARA_PROGRAM_ONLY_SYNTAX "PROGRAM_ONLY;TRUE|FALSE;FALSE"
#define PARA_PROGRAM_ONLY_COMMENT "true if application must program flash code area without erasing (Default; false) [u]"

#define PARA_RECOVERY 'r'
#define PARA_RECOVERY_SYNTAX "RECOVERY;TRUE|FALSE;FALSE"
#define PARA_RECOVERY_COMMENT "true to enable Recovery Mode (Default; false) [u]"


/*****************************************************************************
typedefs and structures (scope: module-local)
*****************************************************************************/

typedef tPChar      tp_fwupg_pn_name_t;

typedef struct tp_fwupg_manager_s
{
	tp_prgdev_instance_t      inst;
	fwupg_config_t *          trgt_cfg;
	trgt_io_manager_t *       io_mgr;
	tp_prgdev_config_t *      prgdev_cfg;
	fwupg_manager_t *         fwupg_manager;
} tp_fwupg_manager_t;

/*****************************************************************************
global variable definitions  (scope: module-exported)
*****************************************************************************/
const tp_parameters_t tp_upgrade_parameters[] =
{
	{PARA_NMEA_DEVICE_SPEED ,PARA_NMEA_DEVICE_SPEED_SYNTAX , PARA_NMEA_DEVICE_SPEED_COMMENT },
	{PARA_ERASE_NVM , PARA_ERASE_NVM_SYNTAX , PARA_ERASE_NVM_COMMENT },
	{PARA_NVM_OFFSET, PARA_NVM_OFFSET_SYNTAX,PARA_NVM_OFFSET_COMMENT },
	{PARA_NVM_SIZE , PARA_NVM_SIZE_SYNTAX, PARA_NVM_SIZE_COMMENT },
	{PARA_PROGRAM_ONLY, PARA_PROGRAM_ONLY_SYNTAX,PARA_PROGRAM_ONLY_COMMENT },
	{PARA_RECOVERY, PARA_RECOVERY_SYNTAX,PARA_RECOVERY_COMMENT },
	{PARA_UNKNOWN,PARA_UNKNOWN_SYNTAX,PARA_UNKNOWN_COMMENT }
};

/*****************************************************************************
global variable definitions (scope: module-local)
*****************************************************************************/

tp_fwupg_manager_t * tp_fwupg_manager[TP_COM_INSTANCE_MAX];


/*****************************************************************************
function prototypes (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function implementations (scope: module-local)
*****************************************************************************/

static tp_status_t tp_fwupg_sync(tp_fwupg_manager_t * tp_fwupg_manager)
{
	tp_status_t error = TP_STATUS_OK;
	tInt start;
	tInt rest;
	tBool synced = FALSE;

	MANAGER_STANDARD(("Syncing with device." TP_ENDL));

	start = tp_tools_gettime();
	rest = 0;

	TP_REPORT_START(tp_fwupg_manager->inst, ITEM_SYNC);

	while (((5000 > rest) || (TRUE == tp_fwupg_manager->trgt_cfg->recovery)) && (FALSE == synced) && (error == TP_STATUS_OK))
	{
		if (TRGT_STATUS_OK == fwupg_sync(tp_fwupg_manager->fwupg_manager))
		{
			synced = TRUE;
		}
		else
		{
			if (tp_fwupg_manager->trgt_cfg->recovery == FALSE)
				TP_REPORT_PROGRESS(tp_fwupg_manager->inst, ITEM_SYNC, (int)((((float)100) / (float)5000) * ((float)(rest % 5000))));
		}

		IS_ABORT(error);

		rest = tp_tools_gettime() - start;
	}

	if (FALSE == synced)
	{
		MANAGER_ERROR(("Syncing failed!" TP_ENDL));
		TP_REPORT_KO(tp_fwupg_manager->inst, ITEM_SYNC);
		error = TP_STATUS_ERROR;
	}
	else
	{
		MANAGER_STANDARD(("Start communication." TP_ENDL));
		TP_REPORT_OK(tp_fwupg_manager->inst, ITEM_SYNC);
		error = TP_TRGT2TP_ERROR(fwupg_start_comm(tp_fwupg_manager->fwupg_manager));
	}

	IS_ABORT(error);

	return error;
}

static tp_status_t tp_fwupg_erase_flash(tp_fwupg_manager_t * tp_fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	TP_REPORT(tp_fwupg_manager->inst, ITEM_FLASH_ERASE, STATE_RUN, 0);

	/* Wait for program flash erasing */
	error = fwupg_wait_eraseprog(tp_fwupg_manager->fwupg_manager);

	if (TRGT_STATUS_OK == error)
	{
		TP_REPORT_OK(tp_fwupg_manager->inst, ITEM_FLASH_ERASE);

		/* Wait for NVM flash erasing */
		if (TRUE == tp_fwupg_manager->trgt_cfg->nvm_erase)
		{
			error = fwupg_wait_erasenvm(tp_fwupg_manager->fwupg_manager);
		}
	}
	else
	{
		TP_REPORT_KO(tp_fwupg_manager->inst, ITEM_FLASH_ERASE);
	}

	return TP_TRGT2TP_ERROR(error);
}

static tp_status_t tp_fwupg_send_buffer(tp_fwupg_manager_t * tp_fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	/* Send buffer to flash */
	error = fwupg_send_buffer(tp_fwupg_manager->fwupg_manager);

	IS_ABORT(error);

	if (TRGT_STATUS_OK == error)
	{
		/* Check CRC */
		fwupg_check_crc(tp_fwupg_manager->fwupg_manager);
	}

	return TP_TRGT2TP_ERROR(error);
}

/*****************************************************************************
function implementations (scope: module-exported)
*****************************************************************************/

tVoid tp_fwupg_params_init(fwupg_config_t *cfg)
{
}

tp_status_t tp_fwupg_params_parse(int argc, char* argv[], fwupg_config_t *cfg, tPBool arg_parsed)
{
	tp_status_t error = TP_STATUS_OK;
	tUInt   arg_num, arg_opt, arg_param;
	tPChar  arg_tempstr = NULL;
	tUInt   prgdev_speed = 0;
	tBool   found;

	arg_opt = 1;
	arg_num = (tUInt)argc;

	while ((arg_opt < arg_num) && (TP_STATUS_OK == error))
	{
		arg_param = 0;
		found = TRUE;

		if (FALSE == arg_parsed[arg_opt])
		{
			if ('-' == argv[arg_opt][0])
			{
				tChar option = _clibs_tolower(argv[arg_opt][1]);

				/* check switches with params */
				arg_param = arg_opt + 1;

				if (arg_param < arg_num)
				{
					arg_tempstr = tp_tools_lcstr_new(argv[arg_param]);

					switch (option)
					{
					case PARA_NMEA_DEVICE_SPEED:

						/* NMEA device speed */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_UINT, &cfg->br_nmea))
						{
							MANAGER_DEBUG(("NMEA device speed: %d" TP_ENDL, cfg->br_nmea));
						}
						else
						{
							MANAGER_ERROR(("NMEA device speed %s is invalid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;

					case PARA_ERASE_NVM:
						/* Erase NVM area */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_BOOL, &cfg->nvm_erase))
						{
							MANAGER_DEBUG(("Erase NVM memory: %s." TP_ENDL, cfg->nvm_erase == TRUE ? "TRUE" : "FALSE"));
						}
						else
						{
							MANAGER_ERROR(("Erase NVM memory option %s not valid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;

					case PARA_NVM_SIZE:
						/* NVM size */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_UINT, &cfg->nvm_size))
						{
							MANAGER_DEBUG(("NVM size: %dkb" TP_ENDL, cfg->nvm_size));
						}
						else
						{
							MANAGER_ERROR(("NVM size %s is invalid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;

					case PARA_NVM_OFFSET:
						/* NVM offset address */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_HEX, &cfg->nvm_offset))
						{
							MANAGER_DEBUG(("NVM offset addr: 0x%x" TP_ENDL, cfg->nvm_offset));
						}
						else
						{
							MANAGER_ERROR(("NVM offset %s invalid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;

					case PARA_PROGRAM_ONLY:
						/* Program only */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_BOOL, &cfg->program_only))
						{
							MANAGER_DEBUG(("Program only: %s." TP_ENDL, cfg->program_only == TRUE ? "TRUE" : "FALSE"));
						}
						else
						{
							MANAGER_ERROR(("Program only option %s not valid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;

					case PARA_RECOVERY:
						/* Recovery mode */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_BOOL, &cfg->recovery))
						{
							MANAGER_DEBUG(("Recovery mode: %s." TP_ENDL, cfg->recovery == TRUE ? "TRUE" : "FALSE"));
						}
						else
						{
							MANAGER_ERROR(("Recovery mode option %s not valid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;

					default:
						found = FALSE;
						break;
					}

					tp_tools_lcstr_delete(arg_tempstr);
				}
				else
				{
					MANAGER_ERROR(("Missing parameter for switch -%c." TP_ENDL, option));
					error = TP_STATUS_ERROR;
				}
			}

			/* If error occurred, options is not found */
			if (TP_STATUS_ERROR == error)
			{
				found = FALSE;
			}

			/* Set argument parsed status */
			arg_parsed[arg_opt++] = found;

			if (0 != arg_param)
			{
				/* parameter found for switch, set also it as parsed */
				arg_parsed[arg_opt++] = found;
			}

		}
		else
		{
			arg_opt++;
		}
	}

	return error;
}

tp_status_t tp_fwupg_params_check(fwupg_config_t *cfg)
{
	tp_status_t error = TP_STATUS_OK;

	if (0 == cfg->nvm_offset)
	{
		cfg->nvm_offset = 0x100000U;
	}

	if (0 == cfg->nvm_size)
	{
		cfg->nvm_size = 0x100000U;
	}

	return error;
}

tVoid tp_fwupg_params_usage(tVoid)
{
	_clibs_printf("Teseo firmware upgrade options:" TP_ENDL);

	int usage_idx = 0;
	while (tp_upgrade_parameters[usage_idx].option_letter != PARA_UNKNOWN)
	{
		_clibs_printf("-%c: %s" TP_ENDL, tp_upgrade_parameters[usage_idx].option_letter, tp_upgrade_parameters[usage_idx].Comment);
		usage_idx++;
	}

}

tp_status_t tp_fwupg_exec(fwupg_config_t *trgt_cfg, tp_prgdev_instance_t inst, trgt_io_manager_t *io_mgr, tp_prgdev_config_t *prg_cfg)
{
	tp_status_t error = TP_STATUS_OK;

	LOCK_MANAGER

		tp_fwupg_manager[inst] = _clibs_calloc(1, sizeof(tp_fwupg_manager_t));
	tp_fwupg_manager[inst]->inst = inst;
	tp_fwupg_manager[inst]->trgt_cfg = trgt_cfg;
	tp_fwupg_manager[inst]->io_mgr = io_mgr;
	tp_fwupg_manager[inst]->prgdev_cfg = prg_cfg;
	tp_fwupg_manager[inst]->fwupg_manager = NULL;

	fwupg_init(inst, trgt_cfg, io_mgr, &tp_fwupg_manager[inst]->fwupg_manager);

	UNLOCK_MANAGER

		/* If NMEA baudrate is not specified, use default 115200 value */
		if (0U == trgt_cfg->br_nmea)
		{
			trgt_cfg->br_nmea = 115200U;
		}

	/* Set */

	//TP_PRINT(("end init baud nmea %d " TP_ENDL, trgt_cfg->br_nmea ));
	//TP_PRINT(("end init baud def %d " TP_ENDL, tp_fwupg_manager.prgdev_cfg->cfg.uart.def_br));
	//TP_PRINT(("end init baud prg %d " TP_ENDL, tp_fwupg_manager.prgdev_cfg->cfg.uart.prg_br));
	//TP_PRINT(("end init baud index %d " TP_ENDL, trgt_cfg->br_fwupg_idx));

	if (FALSE == trgt_cfg->recovery)
	{
		/* Set baudrate to send NMEA FW upgrade command */
		tp_uart_setbaudrate(trgt_cfg->br_nmea, tp_fwupg_manager[inst]->io_mgr->instance_device);
		tp_uart_flush(tp_fwupg_manager[inst]->io_mgr->instance_device);
		//printf("test1\r\n");
		/* Send NMEA command*/
		error = fwupg_send_fwupgcmd(tp_fwupg_manager[inst]->fwupg_manager);
	}

	if (TP_STATUS_OK == error)
	{
		if (TP_PRGDEVID_UART == tp_fwupg_manager[inst]->prgdev_cfg->dev)
		{
			/* Set sync baudrate and flush uart */
			tp_uart_setbaudrate(tp_fwupg_manager[inst]->prgdev_cfg->cfg.uart.def_br, tp_fwupg_manager[inst]->io_mgr->instance_device);
			tp_uart_flush(tp_fwupg_manager[inst]->io_mgr->instance_device);
			//printf("test2\r\n");
		}

		/* Sync with device */
		error = tp_fwupg_sync(tp_fwupg_manager[inst]);
	}

	if (TP_STATUS_OK == error)
	{
		tUInt trials = 50;

		/* Send image info */
		fwupg_send_imageinfo(tp_fwupg_manager[inst]->fwupg_manager);

		if (TP_PRGDEVID_UART == tp_fwupg_manager[inst]->prgdev_cfg->dev)
		{
			if (tp_fwupg_manager[inst]->prgdev_cfg->cfg.uart.prg_br != tp_fwupg_manager[inst]->prgdev_cfg->cfg.uart.def_br)
			{
				MANAGER_STANDARD(("Changing baud rate from %d to  %d" TP_ENDL, tp_fwupg_manager[inst]->prgdev_cfg->cfg.uart.def_br, tp_fwupg_manager[inst]->prgdev_cfg->cfg.uart.prg_br));

				tp_tools_delay(100);

				MANAGER_DEBUG(("Delay 100 ms " TP_ENDL));

				/* Set prog baudrate and flush uart */
				//printf("test3\r\n");
				tp_uart_setbaudrate(tp_fwupg_manager[inst]->prgdev_cfg->cfg.uart.prg_br, tp_fwupg_manager[inst]->io_mgr->instance_device);
				tp_uart_flush(tp_fwupg_manager[inst]->io_mgr->instance_device);
				tp_tools_delay(100);
			}

			error = TP_TRGT2TP_ERROR(fwupg_wait_changebaudrate(tp_fwupg_manager[inst]->fwupg_manager));
		}
	}

	IS_ABORT(error);

	if (TP_STATUS_OK == error)
	{
		error = TP_TRGT2TP_ERROR(fwupg_wait_devinit(tp_fwupg_manager[inst]->fwupg_manager));
	}

	IS_ABORT(error);

	if (TP_STATUS_OK == error)
	{
		if (FALSE == tp_fwupg_manager[inst]->trgt_cfg->program_only)
		{
			error = tp_fwupg_erase_flash(tp_fwupg_manager[inst]);
		}
	}
	else
	{
		MANAGER_ERROR(("Initialization failed!" TP_ENDL));
	}

	IS_ABORT(error);

	if (TP_STATUS_OK == error)
	{
		error = tp_fwupg_send_buffer(tp_fwupg_manager[inst]);
	}

	return error;
}
