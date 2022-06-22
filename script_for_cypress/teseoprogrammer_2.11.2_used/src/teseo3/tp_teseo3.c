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
#include "tp_teseo3.h"
#include "tp_uart.h"
#include "tp_tools.h"
#include "tp_print.h"
#include "teseo3/xldr_teseo3.h"

/*****************************************************************************
external declarations
*****************************************************************************/

/*****************************************************************************
defines and macros (scope: module-local)
*****************************************************************************/

#define MANAGER_STANDARD(x)   TP_PRINT(x)     
#define MANAGER_ERROR(x)      TP_ERROR(x)  
#define MANAGER_DEBUG(x)      TP_INFO(x)   

#define PARA_VARIANT 'v'
#define PARA_VARIANT_SYNTAX "VARIANT;STA8090XX;STA8090FG"
#define PARA_VARIANT_COMMENT "t3 variant (ex. STA8090EX, STA8090EXG, ... Default; STA8090FG) [pe]"

#define PARA_MEMORY 'm'
#define PARA_MEMORY_SYNTAX "MEMORY;SQI|NOR|RAM;SQI"
#define PARA_TARGET_COMMENT "target device memory. Available values are RAM, NOR and SQI (Default; SQI) [pe]"

#define PARA_DESTINATION_ADDRESS 'd'
#define PARA_DESTINATION_ADDRESS_SYNTAX "DESTINATION;Hexadecimal address;0x10000000[,0x100000]"
#define PARA_DESTINATION_ADDRESS_COMMENT "absolute destination address in Hex format associated to optionnal size to erase (Default; 0x10000000) [pe]"

#define PARA_ERASE_NVM 'e'
#define PARA_ERASE_NVM_SYNTAX "ERASE_NVM;TRUE|FALSE;FALSE"
#define PARA_ERASE_NVM_COMMENT "true if application must erase NVM flash memory (Default; false) [pe]"

#define PARA_NVM_OFFSET 'n'
#define PARA_NVM_OFFSET_SYNTAX "OFFSET_NVM;Hexadecimal address;0x100000"
#define PARA_NVM_OFFSET_COMMENT "NVM offset address (Default; 0x100000) [pe]"

#define PARA_NVM_SIZE 'z'
#define PARA_NVM_SIZE_SYNTAX "SIZE_NVM;decimal format;1024"
#define PARA_NVM_SIZE_COMMENT "NVM flash memory size expressed in kilobytes. Do not use if unsure (Default; 1024) [pe]"

#define PARA_PROGRAM_ONLY 'p'
#define PARA_PROGRAM_ONLY_SYNTAX "PROGRAM_ONLY;TRUE|FALSE;FALSE"
#define PARA_PROGRAM_ONLY_COMMENT "true if application must program flash code area without erasing (Default; false) [p]"

#define PARA_RESTART 'r'
#define PARA_RESTART_SYNTAX "RESTART;TRUE|FALSE;FALSE"
#define PARA_RESTART_COMMENT "true to enable Restart Mode (Default; false) [p]"

#define PARA_DUMP 'y'
#define PARA_DUMP_SYNTAX "DUMP_START;Hexadecimal address;0x100000"
#define PARA_DUMP_COMMENT "Dump/Set absolute destination address in Hex format [ds]"

#define PARA_DUMP_SIZE 'x'
#define PARA_DUMP_SIZE_SYNTAX "DUMP_SIZE;dump size;1024"
#define PARA_DUMP_SIZE_COMMENT "Dump memory size expressed in kilobytes (Default; 1024)[d]"

#define PARA_DUMP_PATTERN 'w'
#define PARA_DUMP_PATTERN_SYNTAX "DUMP_PATTTERN;hexadecimal value;0xAABBCCDD"
#define PARA_DUMP_PATTERN_COMMENT "Set 32 bits pattern in Hex format [s]"

#define PARA_SUB_SECTOR 'u'
#define PARA_SUB_SECTOR_SYNTAX "SUB_SECTOR;TRUE|FALSE;FALSE"
#define PARA_SUB_SECTOR_COMMENT "true if flash erase sector is 4KB instead of 64 KB (Default; false) [pe]"

#define PARA_SOFT_PC '+'
#define PARA_SOFT_PC_SYNTAX "PROGRAM START ADDRESS;hexadecimal value;0x1080000"
#define PARA_SOFT_PC_COMMENT "Set program start address after loading (Default; 0) [p]"

#define CITATS 

#define SYNC_TRIAL_MAX 100

/*****************************************************************************
typedefs and structures (scope: module-local)
*****************************************************************************/

typedef tPChar      tp_teseo3_pn_name_t;

typedef struct tp_teseo3_manager_s
{
	trgt_command_t            cmd;
	tp_prgdev_instance_t      inst;
	xldr_teseo3_config_t *    trgt_cfg;
	xldr_teseo3_manager_t *   xldr_mgr;
	trgt_io_manager_t *       io_mgr;
	tp_prgdev_config_t *      prgdev_cfg;
} tp_teseo3_manager_t;

/*****************************************************************************
global variable definitions  (scope: module-exported)
*****************************************************************************/
const tp_parameters_t tp_t3_parameters[] =
{
	{ PARA_MEMORY , PARA_MEMORY_SYNTAX , PARA_TARGET_COMMENT },
	{ PARA_DESTINATION_ADDRESS, PARA_DESTINATION_ADDRESS_SYNTAX,PARA_DESTINATION_ADDRESS_COMMENT },
	{ PARA_ERASE_NVM ,PARA_ERASE_NVM_SYNTAX, PARA_ERASE_NVM_COMMENT },
	{ PARA_NVM_OFFSET, PARA_NVM_OFFSET_SYNTAX ,PARA_NVM_OFFSET_COMMENT },
	{ PARA_NVM_SIZE, PARA_NVM_SIZE_SYNTAX,PARA_NVM_SIZE_COMMENT },
	{ PARA_PROGRAM_ONLY ,PARA_PROGRAM_ONLY_SYNTAX ,PARA_PROGRAM_ONLY_COMMENT },
	{ PARA_RESTART , PARA_RESTART_SYNTAX ,PARA_RESTART_COMMENT },
	{ PARA_DUMP , PARA_DUMP_SYNTAX , PARA_DUMP_COMMENT },
	{ PARA_DUMP_SIZE, PARA_DUMP_SIZE_SYNTAX, PARA_DUMP_SIZE_COMMENT },
	{ PARA_DUMP_PATTERN, PARA_DUMP_PATTERN_SYNTAX,PARA_DUMP_PATTERN_COMMENT },
	{ PARA_VARIANT ,PARA_VARIANT_SYNTAX,PARA_VARIANT_COMMENT},
	{ PARA_UNKNOWN ,PARA_UNKNOWN_SYNTAX,PARA_UNKNOWN_COMMENT },
	{ PARA_SOFT_PC,PARA_SOFT_PC_SYNTAX,PARA_SOFT_PC_COMMENT}
};

/*****************************************************************************
global variable definitions (scope: module-local)
*****************************************************************************/

CITATS tp_teseo3_pn_name_t tp_teseo3_pn_table[TRGT_VARIANT_STA8090_NUMBER + 1] =
{
	"",                   /* This is the unknown variant */
	"sta8089fg",
	"sta8089fgb",
	"sta8089fgbd",
	"sta8089g",
	"sta8089gr",
	"sta8089ga",
	"sta8089gay",
	"sta8090exg",
	"sta8090exga",
	"sta8090f4g",
	"sta8090f4gb",
	"sta8090f4gy",
	"sta8090fg",
	"sta8090fgb",
	"sta8090fgbd",
	"sta8090fgy",
	"sta8090ga",
	"sta8090gat",
	"sta8090wg",
	"sta8090wgr",
	"sta8089fga"
};


CITATS tp_teseo3_manager_t * tp_teseo3_manager[TP_COM_INSTANCE_MAX];

/*****************************************************************************
function prototypes (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function implementations (scope: module-local)
*****************************************************************************/

CITATS tp_status_t tp_teseo3_sync_with_device(tp_teseo3_manager_t * tp_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tInt start;
	tUInt time_out = SYNC_TRIAL_MAX;

	MANAGER_STANDARD(("Syncing with device, instance %d" TP_ENDL, tp_teseo3_manager->inst));
	tp_uart_flush(tp_teseo3_manager->inst);
	do
	{
		start = tp_tools_gettime();

		/* Sync with device */
		error = xldr_teseo3_sync(tp_teseo3_manager->xldr_mgr);

		time_out--;

		/* Wait up to 1s */
		while (10 > (tp_tools_gettime() - start));

		IS_ABORT(error);

	} while ((time_out > 0) && (TRGT_STATUS_ERROR == error));

	if (TRGT_STATUS_ERROR == error)
	{
		TP_PRINT(("Sync failed!" TP_ENDL));
	}

	return TP_TRGT2TP_ERROR(error);
}

CITATS tp_status_t tp_teseo3_wait_host_ready(tp_teseo3_manager_t * tp_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tInt start;
	tUInt trials = 0;

	/* Wait host to be ready */
	trials = 30;

	MANAGER_STANDARD(("Waiting host to be ready, instance %d" TP_ENDL, tp_teseo3_manager->inst));

	do
	{
		start = tp_tools_gettime();

		/* Close, open and flush uart */
		tp_uart_close(tp_teseo3_manager->inst);

		tp_tools_delay(500);

		tp_uart_open(tp_teseo3_manager->inst);
		tp_uart_flush(tp_teseo3_manager->inst);

		/* Sync with device */
		error = xldr_teseo3_check_baudrate(tp_teseo3_manager->xldr_mgr);

		trials--;

		/* Wait up to 1s */
		while (1000 > (tp_tools_gettime() - start));

		IS_ABORT(error);

	} while ((trials > 0) && (TRGT_STATUS_ERROR == error));

	if (TRGT_STATUS_ERROR == error)
	{
		TP_PRINT(("Host not ready!" TP_ENDL));
	}

	return TP_TRGT2TP_ERROR(error);
}

CITATS tp_status_t tp_teseo3_change_baudrate(tp_teseo3_manager_t * tp_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	if (tp_teseo3_manager->prgdev_cfg->cfg.uart.prg_br != tp_teseo3_manager->prgdev_cfg->cfg.uart.def_br)
	{
		/* New ROMs accept baud rate value, while old ones accepts its index*/
		/* Try sending baudrate */
		error = xldr_teseo3_send_baudrate(tp_teseo3_manager->xldr_mgr,tp_teseo3_manager->prgdev_cfg->cfg.uart.prg_br);

		if (TRGT_STATUS_ERROR == error)
		{
			tp_tools_delay(250);

			/* Flush uart */
			tp_uart_flush(tp_teseo3_manager->inst);

			tp_tools_delay(250);

			/* Sync with device */
			error = xldr_teseo3_sync(tp_teseo3_manager->xldr_mgr);

			/* Try sending idx */
			error = xldr_teseo3_send_baudrate(tp_teseo3_manager->xldr_mgr,tp_teseo3_manager->prgdev_cfg->cfg.uart.prg_br_index);
		}

		if (TRGT_STATUS_OK == error)
		{
			/* Set new baud rate */
			tp_uart_setbaudrate(tp_teseo3_manager->prgdev_cfg->cfg.uart.prg_br, tp_teseo3_manager->inst);
			MANAGER_STANDARD(("Set baud rate to %d" TP_ENDL, tp_teseo3_manager->prgdev_cfg->cfg.uart.prg_br));

			error = TP_TP2TRGT_ERROR(tp_teseo3_wait_host_ready(tp_teseo3_manager));
		}
	}

	return TP_TRGT2TP_ERROR(error);
}

CITATS tp_status_t tp_teseo3_preamble(tp_teseo3_manager_t * tp_teseo3_manager)
{
	tp_status_t error = TP_STATUS_OK;

	/* Using UART, preamble must be sent at 115200 */
	if (TP_PRGDEVID_UART == tp_teseo3_manager->prgdev_cfg->dev)
	{
		tp_uart_setbaudrate(tp_teseo3_manager->prgdev_cfg->cfg.uart.def_br, tp_teseo3_manager->inst);
		MANAGER_STANDARD(("Set baud rate to %d" TP_ENDL, tp_teseo3_manager->prgdev_cfg->cfg.uart.def_br));
	}

	/* Try to sync with device */
	error = tp_teseo3_sync_with_device(tp_teseo3_manager);

	IS_ABORT(error);

	if (TP_STATUS_OK == error)
	{
		if (TP_PRGDEVID_UART == tp_teseo3_manager->prgdev_cfg->dev)
		{
			error = tp_teseo3_change_baudrate(tp_teseo3_manager);
		}
	}

	IS_ABORT(error);

	if (TRGT_STATUS_OK == error)
	{
		/* Check for host to be ready */
		error = TP_TRGT2TP_ERROR(xldr_teseo3_check_hostready(tp_teseo3_manager->xldr_mgr));
	}

	IS_ABORT(error);

	if (TP_STATUS_OK == error)
	{
		/* Send preamble */
		error = TP_TRGT2TP_ERROR(xldr_teseo3_send_preamble(tp_teseo3_manager->xldr_mgr));
	}

	IS_ABORT(error);

	if (TP_STATUS_OK == error)
	{
		/* Send bootloader */
		error = TP_TRGT2TP_ERROR(xldr_teseo3_send_bootloader(tp_teseo3_manager->xldr_mgr));
	}

	IS_ABORT(error);

	return error;
}

CITATS tp_status_t tp_teseo3_erase_flash(tp_teseo3_manager_t * tp_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	TP_REPORT(tp_teseo3_manager->inst,ITEM_FLASH_ERASE, STATE_RUN, 0);

	/* Wait for program flash erasing */
	error = xldr_teseo3_wait_eraseprog(tp_teseo3_manager->xldr_mgr);

	IS_ABORT(error);

	if (TRGT_STATUS_OK == error)
	{
		TP_REPORT_OK(tp_teseo3_manager->inst, ITEM_FLASH_ERASE);

		/* Wait for NVM flash erasing */
		if (TRUE == tp_teseo3_manager->trgt_cfg->nvm_erase)
		{
			error = xldr_teseo3_wait_erasenvm(tp_teseo3_manager->xldr_mgr);

			IS_ABORT(error);
		}
	}
	else
	{
		TP_REPORT_KO(tp_teseo3_manager->inst, ITEM_FLASH_ERASE);
	}

	return TP_TRGT2TP_ERROR(error);
}

CITATS tp_status_t tp_teseo3_send_buffer(tp_teseo3_manager_t * tp_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	/* Send buffer to flash */
	error = xldr_teseo3_send_buffer(tp_teseo3_manager->xldr_mgr);

	IS_ABORT(error);

	if (TRGT_STATUS_OK == error)
	{
		/* Check CRC */
		xldr_teseo3_check_crc(tp_teseo3_manager->xldr_mgr);
	}

	return TP_TRGT2TP_ERROR(error);
}

CITATS tp_status_t tp_teseo3_dump(tp_teseo3_manager_t * tp_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	/* Send buffer to flash */
	error = xldr_teseo3_dump(tp_teseo3_manager->xldr_mgr);

	IS_ABORT(error);

	return TP_TRGT2TP_ERROR(error);
}

CITATS tp_status_t tp_teseo3_set(tp_teseo3_manager_t * tp_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	/* Send buffer to flash */
	error = xldr_teseo3_set(tp_teseo3_manager->xldr_mgr);

	IS_ABORT(error);

	return TP_TRGT2TP_ERROR(error);
}

/*****************************************************************************
function implementations (scope: module-exported)
*****************************************************************************/

tVoid tp_teseo3_params_init(xldr_teseo3_config_t *cfg)
{
}

tp_status_t tp_teseo3_params_parse(int argc, char* argv[], xldr_teseo3_config_t *cfg, tPBool arg_parsed)
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
					case PARA_SOFT_PC:
						/* Destination address */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_HEX, &cfg->entry))
						{
							MANAGER_DEBUG(("software start addr: 0x%x" TP_ENDL, cfg->entry));
						}
						else
						{
							MANAGER_ERROR(("software start addr %s invalid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;
					case PARA_VARIANT:
						/* Variant of Teseo3 */
						if (TRGT_CFG_UNKNOWN == cfg->variant)
						{
							tUInt pn = 1;

							while (pn < TRGT_VARIANT_STA8090_NUMBER)
							{
								if (0 == _clibs_strcmp(arg_tempstr, tp_teseo3_pn_table[pn]))
								{
									cfg->variant = pn;
									MANAGER_DEBUG(("variant %s valid." TP_ENDL, arg_tempstr));
									break;
								}
								else
								{
									pn++;
								}
							}

							if (TRGT_CFG_UNKNOWN == cfg->variant)
							{
								MANAGER_ERROR(("Variant %s not valid." TP_ENDL, arg_tempstr));
								error = TP_STATUS_ERROR;
							}
						}
						else
						{
							MANAGER_ERROR(("Variant specified more than once!" TP_ENDL));
							error = TP_STATUS_ERROR;
						}
						break;

					case PARA_MEMORY:
						/* Memory type */
						if (TRGT_CFG_UNKNOWN == cfg->mem)
						{
							if (0 == _clibs_strcmp("ram", arg_tempstr))
							{
								cfg->mem = TRGT_MEM_RAM;
							}
							else if (0 == _clibs_strcmp("nor", arg_tempstr))
							{
								cfg->mem = TRGT_MEM_NOR;
							}
							else if (0 == _clibs_strcmp("sqi", arg_tempstr))
							{
								cfg->mem = TRGT_MEM_SQI;
							}
							else
							{
								MANAGER_ERROR(("Memory %s is not valid." TP_ENDL, argv[arg_param]));
								error = TP_STATUS_ERROR;
							}
						}
						else
						{
							MANAGER_ERROR(("Target specified more than once!" TP_ENDL));
						}
						break;

					case PARA_DESTINATION_ADDRESS:
						/* Destination address */
						if (TP_STATUS_OK == tp_tools_parse_valhex_valhex(arg_tempstr, &cfg->destaddr, &cfg->desterasesize))
						{
							if (cfg->desterasesize == 0)
							{
								MANAGER_DEBUG(("Target dest addr: 0x%x" TP_ENDL, cfg->destaddr));
							}
							else
							{
								MANAGER_DEBUG(("Target dest addr: 0x%x and force erase: 0x%x" TP_ENDL, cfg->destaddr, cfg->desterasesize));
							}
						}
						else
						{
							MANAGER_ERROR(("Destination address %s invalid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;

#if 0
					case 'n':
						/* Entry address */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_HEX, &cfg->entry))
						{
							MANAGER_DEBUG(("Entry address 0x%x" TP_ENDL, cfg->entry));
						}
						else
						{
							MANAGER_ERROR(("Destination address %s invalid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;
#endif

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

					case PARA_NVM_SIZE:
						/* NVM size */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_UINT, &cfg->nvm_size))
						{
							MANAGER_DEBUG(("NVM size: %dkb." TP_ENDL, cfg->nvm_size));
						}
						else
						{
							MANAGER_ERROR(("NVM size %s is invalid!" TP_ENDL, argv[arg_param]));
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

					case PARA_SUB_SECTOR:
						/* Subsector erasing */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_BOOL, &cfg->subsector))
						{
							MANAGER_DEBUG(("Subsector erasing: %s." TP_ENDL, cfg->subsector == TRUE ? "TRUE" : "FALSE"));
						}
						else
						{
							MANAGER_ERROR(("Subsector erasing option %s not valid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;

					case PARA_RESTART:
						/* Restart mode */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_BOOL, &cfg->restart))
						{
							MANAGER_DEBUG(("Restart mode: %s." TP_ENDL, cfg->restart == TRUE ? "TRUE" : "FALSE"));
						}
						else
						{
							MANAGER_ERROR(("Restart mode option %s not valid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;

					case PARA_DUMP:
						/* Dump absolute destination address in Hex format */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_HEX, &cfg->debugAddress))
						{
							MANAGER_DEBUG(("Dump addr: 0x%x" TP_ENDL, cfg->debugAddress));
						}
						else
						{
							MANAGER_ERROR(("Dump addr %s invalid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;
						

					case PARA_DUMP_SIZE:
						/* Dump memory size expressed in kilobytes (Default: 1024) */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_UINT, &cfg->debugSize))
						{
							MANAGER_DEBUG(("Dump size: %d" TP_ENDL, cfg->debugSize));
						}
						else
						{
							MANAGER_ERROR(("Dump size %s invalid!" TP_ENDL, argv[arg_param]));
							error = TP_STATUS_ERROR;
						}
						break;

					case PARA_DUMP_PATTERN:
						/* Set 32 bits pattern value in Hex format */
						if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_HEX, &cfg->debugData))
						{
							MANAGER_DEBUG(("Set 32 bits pattern : 0x%x" TP_ENDL, cfg->debugData));
						}
						else
						{
							MANAGER_ERROR(("Set 32 bits pattern %s invalid!" TP_ENDL, argv[arg_param]));
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

tp_status_t tp_teseo3_params_check(trgt_command_t cmd, xldr_teseo3_config_t *cfg)
{
	tp_status_t error = TP_STATUS_OK;

	switch (cmd)
	{
	case TRGT_CMD_PROG:
	case TRGT_CMD_ERASE:
	case TRGT_CMD_DUMP:
	case TRGT_CMD_SET:


		if (TP_CFG_UNKNOWN == cfg->variant)
		{
			cfg->variant = TRGT_VARIANT_STA8090EXG;
		}

		if (TP_CFG_UNKNOWN == cfg->mem)
		{
			cfg->mem = TRGT_MEM_SQI;
		}

		if (0 == cfg->destaddr)
		{
			switch (cfg->mem)
			{
			case TRGT_MEM_SQI:
				cfg->destaddr = 0x10000000U;
				break;

			case TRGT_MEM_NOR:
				cfg->destaddr = 0x20000000U;
				break;

			case TRGT_MEM_RAM:
				MANAGER_ERROR(("Destination address for RAM must be specified." TP_ENDL));
				error = TP_STATUS_ERROR;
				break;

			default:
				break;
			}
		}

		if (0 == cfg->nvm_offset)
		{
			cfg->nvm_offset = 0x100000U;
		}

		if (0 == cfg->nvm_size)
		{
			cfg->nvm_size = 0x100000U;
		}

		if (1 == cfg->nvm_erase && 0 != cfg->desterasesize)
		{
			MANAGER_ERROR(("NVM erasing cannot be use at same times than over binary erasing" TP_ENDL));
			error = TP_STATUS_ERROR;
		}
		break;

	default:
		break;
	}

	switch (cmd)
	{
	case TRGT_CMD_DUMP:
		cfg->debug = TRGT_DEBUG_ON;
		cfg->debugAction = TRGT_DEBUG_DUMP;
		break;

	case TRGT_CMD_SET:
		cfg->debug = TRGT_DEBUG_ON;
		cfg->debugAction = TRGT_DEBUG_SET;
		break;
	}

	return error;
}

tVoid tp_teseo3_params_usage(tVoid)
{
	_clibs_printf("Teseo3 options:" TP_ENDL);

	int usage_idx = 0;
	while (tp_t3_parameters[usage_idx].option_letter != PARA_UNKNOWN)
	{
		_clibs_printf("-%c: %s" TP_ENDL, tp_t3_parameters[usage_idx].option_letter, tp_t3_parameters[usage_idx].Comment);
		usage_idx++;
	}
}

tp_status_t tp_teseo3_exec(trgt_command_t cmd, tp_prgdev_instance_t inst, xldr_teseo3_config_t *trgt_cfg, trgt_io_manager_t *io_mgr, tp_prgdev_config_t *prg_cfg)
{
	tp_status_t error = TP_STATUS_OK;

	LOCK_MANAGER

	tp_teseo3_manager[inst] = _clibs_calloc(1, sizeof(tp_teseo3_manager_t));
	tp_teseo3_manager[inst]->inst = inst;
	tp_teseo3_manager[inst]->cmd = cmd;
	tp_teseo3_manager[inst]->trgt_cfg = trgt_cfg;
	tp_teseo3_manager[inst]->io_mgr = io_mgr;
	tp_teseo3_manager[inst]->prgdev_cfg = prg_cfg;
	tp_teseo3_manager[inst]->xldr_mgr = NULL;

	xldr_teseo3_init(cmd, inst, trgt_cfg, io_mgr, &tp_teseo3_manager[inst]->xldr_mgr);

	UNLOCK_MANAGER

	TP_REPORT(inst, ITEM_PREAMBLE, STATE_RUN, 0);

	error = tp_teseo3_preamble(tp_teseo3_manager[inst]);

	if (TP_STATUS_OK == error)
	{
		/* Send binary info */
		TP_REPORT_OK(inst, ITEM_PREAMBLE);
		TP_REPORT(inst, ITEM_BIN_INFO, STATE_RUN, 0);

		// Manage ACK later for SET command
		error = TP_TRGT2TP_ERROR(xldr_teseo3_send_bininfo(tp_teseo3_manager[inst]->xldr_mgr,cmd != TRGT_CMD_SET));

		if (TP_STATUS_OK == error)
		{
			TP_REPORT_OK(inst, ITEM_BIN_INFO);
		}
		else
		{
			TP_REPORT_KO(inst, ITEM_BIN_INFO);
		}
	}
	else
	{
		TP_REPORT_KO(inst, ITEM_PREAMBLE);
	}

	if (TP_STATUS_OK == error && cmd != TRGT_CMD_DUMP && cmd != TRGT_CMD_SET)
	{
		/* Wait for device initialization */
		error = TP_TRGT2TP_ERROR(xldr_teseo3_wait_devinit(tp_teseo3_manager[inst]->xldr_mgr));
	}

	IS_ABORT(error);

	if (TP_STATUS_OK == error)
	{
		switch (cmd)
		{
		case TRGT_CMD_PROG:
			switch (trgt_cfg->mem)
			{
			case TRGT_MEM_SQI:
			case TRGT_MEM_NOR:
				if (FALSE == tp_teseo3_manager[inst]->trgt_cfg->program_only)
				{
					error = tp_teseo3_erase_flash(tp_teseo3_manager[inst]);
				}

				if (TP_STATUS_OK == error)
				{
					error = tp_teseo3_send_buffer(tp_teseo3_manager[inst]);
				}
				break;

			case TRGT_MEM_RAM:
				error = tp_teseo3_send_buffer(tp_teseo3_manager[inst]);
				break;

			default:
				break;
			}
			break;

		case TRGT_CMD_ERASE:
			switch (trgt_cfg->mem)
			{
			case TRGT_MEM_SQI:
			case TRGT_MEM_NOR:
				error = tp_teseo3_erase_flash(tp_teseo3_manager[inst]);
				break;

			default:
				break;
			}
			break;

		case TRGT_CMD_DUMP:
			error = tp_teseo3_dump(tp_teseo3_manager[inst]);
			break;

		case TRGT_CMD_SET:
			error = tp_teseo3_set(tp_teseo3_manager[inst]);
			break;

		default:
			error = TP_STATUS_ERROR;
			break;
		}
	}

	if (error != TP_STATUS_ERROR)
	{
		MANAGER_STANDARD(("Program OK : instance %d" TP_ENDL, inst));
	}
	else
	{
		MANAGER_STANDARD(("Program KO : instance %d" TP_ENDL, inst));
	}

	return error;
}
