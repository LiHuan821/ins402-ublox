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

#include "tp_common.h"
#include "tp_params.h"
#include "tp_tools.h"
#include "tp_print.h"
#include "tp_uart.h"

/*****************************************************************************
   external declarations
*****************************************************************************/

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define TP_PARSE_INFO(x)      TP_INFO(x)

/* Common command */
#define PARA_QUIET 'q'
#define PARA_QUIET_SYNTAX "QUIET_MODE;TRUE|FALSE;FALSE"
#define PARA_QUIET_COMMENT "enable quiet mode [pedsu]"

#define PARA_USAGE 'h'
#define PARA_USAGE_SYNTAX "HELP;-f t2|-f t3|-f t5;-f t3"
#define PARA_USAGE_COMMENT "print usage message, use with -t for device specific options"

#define PARA_INPUT_FILE 'i'
#define PARA_INPUT_FILE_SYNTAX "FIRMWARE;filename;c:\\bin\\teseo.bin"
#define PARA_INPUT_FILE_COMMENT "input binary file (mandatory for program/upgrade) [pud]"

#define PARA_OUTPUT_FILE 'o'
#define PARA_OUTPUT_FILE_SYNTAX "LOG;filename;c\\log\\programm_teseo_log.txt"
#define PARA_OUTPUT_FILE_COMMENT "output file name. It's the file where upgrade info is redirected [pedsu]"

#define PARA_TARGET 'f'
#define PARA_TARGET_SYNTAX "TARGET;-f t2|-f t3|-f t5;t3"
#define PARA_TARGET_COMMENT "target family (mandatory, t2 = Teseo2, t3 = Teseo3, t5 = Teseo5) [peds]"

#define PARA_COM 'c'
#define PARA_COM_SYNTAX "COM;COM name;COM1"
#define PARA_COM_COMMENT "serial COM port name (Default; COM1)[pedsu]"

#define PARA_BAUDRATE 'b'
#define PARA_BAUDRATE_SYNTAX "BAUDRATE;57600,115200,230400,460800,921600;115200"
#define PARA_BAUDRATE_COMMENT "serial COM port baud rate (Default; 115200) [pedsu]"

#define PARA_PUBLIC_RSA_FILE 'a'
#define PARA_PUBLIC_RSA_FILE_SYNTAX "RSA_PUBLIC;filename;c\\rsa\\public_rsa.ssh"
#define PARA_PUBLIC_RSA_FILE_COMMENT "Insert public RSA key in protocol for authentication [p]"

#define PARA_PRIVATE_RSA_FILE 'j'
#define PARA_PRIVATE_RSA_FILE_SYNTAX "RSA_PRIVATE;filename;c\\rsa\\private_rsa.ssh"
#define PARA_PRIVATE_RSA_FILE_COMMENT "Insert private RSA key in protocol for authentication [p]"

#define PARA_RESET_BOARD 'l'
#define PARA_RESET_BOARD_SYNTAX "RESET;RTS|DTR|RTSDTR;RTS=500"
#define PARA_RESET_BOARD_COMMENT "Execute a reset on GPIO RTS and/or DTR before operate in milliseconds [pedsu]"

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   global variable definitions  (scope: module-exported)
*****************************************************************************/
const tp_parameters_t tp_common_parameters[] =
{
	{ PARA_QUIET ,PARA_QUIET_SYNTAX, PARA_QUIET_COMMENT },
	{ PARA_USAGE ,PARA_USAGE_SYNTAX, PARA_USAGE_COMMENT },
	{ PARA_INPUT_FILE ,PARA_INPUT_FILE_SYNTAX, PARA_INPUT_FILE_COMMENT },
	{ PARA_OUTPUT_FILE , PARA_OUTPUT_FILE_SYNTAX ,PARA_OUTPUT_FILE_COMMENT },
	{ PARA_TARGET, PARA_TARGET_SYNTAX , PARA_TARGET_COMMENT },
	{ PARA_COM , PARA_COM_SYNTAX,PARA_COM_COMMENT },
	{ PARA_BAUDRATE,PARA_BAUDRATE_SYNTAX ,PARA_BAUDRATE_COMMENT },
	{ PARA_PUBLIC_RSA_FILE,PARA_PUBLIC_RSA_FILE_SYNTAX,PARA_PUBLIC_RSA_FILE_COMMENT },
	{ PARA_PRIVATE_RSA_FILE , PARA_PRIVATE_RSA_FILE_SYNTAX , PARA_PRIVATE_RSA_FILE_COMMENT },
	{ PARA_RESET_BOARD ,PARA_RESET_BOARD_SYNTAX,PARA_RESET_BOARD_COMMENT },
	{ PARA_UNKNOWN ,PARA_UNKNOWN_SYNTAX,PARA_UNKNOWN_COMMENT}

};

/*****************************************************************************
   global variable definitions (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   function prototypes (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   function implementations (scope: module-local)
*****************************************************************************/

tp_status_t tp_parse_get_bool(const tPChar src_str, tPBool value_ptr)
{
	tp_status_t error = TP_STATUS_OK;
	tBool value = FALSE;

	if ((0 == _clibs_strcmp(src_str, "true")) && (4 == _clibs_strlen(src_str)))
	{
		value = TRUE;
	}
	else if ((0 == _clibs_strcmp(src_str, "false")) && (5 == _clibs_strlen(src_str)))
	{
		value = FALSE;
	}
	else
	{
		error = TP_STATUS_ERROR;
	}

	if (TP_STATUS_ERROR == error)
	{
		*value_ptr = FALSE;
	}

	return error;
}

/*****************************************************************************
function implementations (scope: module-exported)
*****************************************************************************/

/*
  Generic command line parser. This will parse only generic switches and
  options.
*/
tp_status_t tp_params_parse(int argc, char* argv[], tp_general_config_t *gen_mgr, tPBool arg_parsed)
{
	tp_status_t error = TP_STATUS_OK;
	tUInt   prgdev_speed = 0;
	tUInt   arg_num, arg_opt, arg_param;
	tPChar  arg_tempstr = NULL;
	tBool   found;

	arg_opt = 1;
	arg_num = (tUInt)argc;

	while ((arg_opt < arg_num))
	{
		arg_param = 0;
		found = TRUE;

		if (FALSE == arg_parsed[arg_opt])
		{
			if ('-' == argv[arg_opt][0])
			{
				if (0 == argv[arg_opt][1])
				{
					/* - specified without */
					TP_FATAL_ERROR(("- used without switch." TP_ENDL));
					error = TP_STATUS_ERROR;
				}
				else
				{
					/* argument is a switch */
					tChar option = _clibs_tolower(argv[arg_opt][1]);

					/* check switches with optional params */
					if (PARA_QUIET == option)
					{
						gen_mgr->quiet = TRUE;

						/* remove TRUE FALSE parameter if present*/
						if (arg_opt + 1 < (tUInt)argc)
						{
							arg_tempstr = tp_tools_lcstr_new(argv[arg_opt + 1]);
							if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_BOOL, &gen_mgr->quiet))
							{
								arg_param = arg_opt + 1;
							}
							printf("arg_tempstr = %s,gen_mgr->quiet = %d\r\n",arg_tempstr,gen_mgr->quiet);
						}
					}
					else if (PARA_USAGE == option)
					{
						gen_mgr->help = TRUE;

						/* remove TRUE FALSE parameter if present*/
						if (arg_opt + 1 < (tUInt)argc)
						{
							arg_tempstr = tp_tools_lcstr_new(argv[arg_opt + 1]);
							if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_BOOL, &gen_mgr->help))
							{
								arg_param = arg_opt + 1;
							}
						}
					}
					else
					{
						/* check switches with params */
						arg_param = arg_opt + 1;

						if (arg_param < arg_num)
						{
							arg_tempstr = tp_tools_lcstr_new(argv[arg_param]);

							switch (option)
							{
							case PARA_INPUT_FILE:
								/*
								Input file option
								*/
								if (NULL == gen_mgr->in_filename)
								{
									/*
				  If the length of the copy depends on source buffer, latest GCC releases
				  reports a warning because in theory destination buffer could be smaller
				  and overflow would occur. In this case buffer is allocated starting from
				  source buffer size, but the compiler cannot see it.
				  To avoid warning, bufsize is declared volatile. In this way the compiler
				  cannot optimize putting strlen in strncpy call, and the warning in not
				  triggered.
				  */
									tPChar filename = NULL;
									volatile tUInt bufsize = _clibs_strlen(argv[arg_param]) + 1;

									filename = (tPChar)_clibs_malloc(bufsize);
									if (NULL == filename)
									{
										TP_FATAL_ERROR(("Not enough memory!" TP_ENDL));
										error = TP_STATUS_ERROR;
									}
									else
									{
										_clibs_strncpy(filename, argv[arg_param], bufsize);
										TP_PARSE_INFO(("Input binary file: %s." TP_ENDL, filename));
										gen_mgr->in_filename = filename;
									}
								}
								else
								{
									error = TP_STATUS_ERROR;
									TP_FATAL_ERROR(("Binary file name already specified: %s!" TP_ENDL, gen_mgr->in_filename));
								}
								break;

							case PARA_OUTPUT_FILE:
								/*
								Output file option
								*/
								if (NULL == gen_mgr->out_filename)
								{
									/*
									If the length of the copy depends on source buffer, latest GCC releases
										reports a warning because in theory destination buffer could be smaller
										and overflow would occur.In this case buffer is allocated starting from
										source buffer size, but the compiler cannot see it.
										To avoid warning, bufsize is declared volatile.In this way the compiler
										cannot optimize putting strlen in strncpy call, and the warning in not
										triggered.
										*/
									tPChar filename = NULL;
									volatile tUInt bufsize = _clibs_strlen(argv[arg_param]) + 1;

									filename = (tPChar)_clibs_malloc(bufsize);
									if (NULL == filename)
									{
										TP_FATAL_ERROR(("Not enough memory!" TP_ENDL));
										error = TP_STATUS_ERROR;
									}
									else
									{
										_clibs_strncpy(filename, argv[arg_param], bufsize);
										TP_PARSE_INFO(("Output binary file: %s." TP_ENDL, filename));
										gen_mgr->out_filename = filename;
									}
								}
								else
								{
									error = TP_STATUS_ERROR;
									TP_FATAL_ERROR(("Binary file name already specified: %s!" TP_ENDL, gen_mgr->out_filename));
								}
								break;

							case PARA_TARGET:
								/*
								Option for target
								*/
								if (TRGT_CFG_UNKNOWN == gen_mgr->target)
								{
									if (0 == _clibs_strcmp("t2", arg_tempstr))
									{
										gen_mgr->target = TRGT_FAMILY_TESEO2;
									}
									else if (0 == _clibs_strcmp("t3", arg_tempstr))
									{
										gen_mgr->target = TRGT_FAMILY_TESEO3;
									}
									else if (0 == _clibs_strcmp("t5", arg_tempstr))
									{
										gen_mgr->target = TRGT_FAMILY_TESEO5;
									}
									else
									{
										TP_FATAL_ERROR(("Target %s not recognized." TP_ENDL, argv[arg_param]));
										error = TP_STATUS_ERROR;
									}
								}
								else
								{
									error = TP_STATUS_ERROR;
									TP_FATAL_ERROR(("Target specified more than once!" TP_ENDL));
								}
								break;

								/*
								Options for device
								*/
							case PARA_COM:
								/* Programming device port */
								if (TP_CFG_UNKNOWN == gen_mgr->prgdev_cfg.dev)
								{
									/* Split COM port list -c COM1,COM2,COMx,.... */
									tPChar * ListComs = NULL;
									/* do not change character case in case of UNIX path */
									tUInt COMs = tp_tools_split(argv[arg_param], &ListComs, ',');

									gen_mgr->prgdev_cfg.cfg.uart.port = _clibs_calloc(COMs, sizeof(tPChar));

									if (gen_mgr->prgdev_cfg.cfg.uart.port == NULL)
									{
										TP_FATAL_ERROR(("Not enough memory!" TP_ENDL));
										error = TP_STATUS_ERROR;
									}
									else
									{
										for (tUInt comid = 0; comid < COMs && error != TP_STATUS_ERROR; comid++)
										{

											if (0 == _clibs_strncmp(ListComs[comid], "com", 3) || 0 == _clibs_strncmp(ListComs[comid], "COM", 3) ||
											0 == _clibs_strncmp(ListComs[comid], "/dev/", strlen("/dev/")))
											{
												tPChar portname;

												portname = (tPChar)_clibs_malloc(_clibs_strlen(ListComs[comid]) + 1);
												if (NULL != portname)
												{
#ifdef MSVC
													tp_tools_toupper(portname, ListComs[comid]);
#else
													_clibs_strcpy(portname,ListComs[comid]);
#endif
													TP_PARSE_INFO(("Programming device port: %s." TP_ENDL, portname));

													gen_mgr->prgdev_cfg.dev = TP_PRGDEVID_UART;
													gen_mgr->prgdev_cfg.cfg.uart.port[comid] = portname;
													if (0 != prgdev_speed)
													{
														gen_mgr->prgdev_cfg.cfg.uart.prg_br = prgdev_speed;
													}
												}
												else
												{
													TP_FATAL_ERROR(("Not enough memory!" TP_ENDL));
													error = TP_STATUS_ERROR;
												}
											}
											else
											{
												TP_FATAL_ERROR(("Programming device port %s not supported!" TP_ENDL, arg_tempstr));
												error = TP_STATUS_ERROR;
											}
										}

										gen_mgr->prgdev_cfg.cfg.uart.nb_ports = COMs;

#ifndef MULTI_COMS
										if (gen_mgr->prgdev_cfg.cfg.uart.nb_ports > 1)
										{
											TP_FATAL_ERROR(("This programmer version does not support multi com ports" TP_ENDL));
											error = TP_STATUS_ERROR;
										}
#endif
									}
								}
								else
								{
									TP_FATAL_ERROR(("Programming device defined more than once!/r/n"));
									error = TP_STATUS_ERROR;
								}
								break;

							case PARA_BAUDRATE:
								/* Programming device speed */
								if (TP_STATUS_OK == tp_tools_parse_val(arg_tempstr, TP_TOOLS_PARSE_FMT_UINT, &prgdev_speed))
								{
									TP_PARSE_INFO(("Programming device speed: %d" TP_ENDL, prgdev_speed));

									if (TP_PRGDEVID_UART == gen_mgr->prgdev_cfg.dev)
									{
										gen_mgr->prgdev_cfg.cfg.uart.prg_br = prgdev_speed;
									}
								}
								else
								{
									TP_FATAL_ERROR(("Programming device speed %s is invalid!" TP_ENDL, argv[arg_param]));
									error = TP_STATUS_ERROR;
								}
								break;

								// private RSA key file
							case PARA_PRIVATE_RSA_FILE:
								if (NULL == gen_mgr->PublicRSAKey)
								{
									tPChar filename = NULL;
									volatile tUInt bufsize = _clibs_strlen(argv[arg_param]) + 1;

									filename = (tPChar)_clibs_malloc(bufsize);
									if (NULL == filename)
									{
										TP_FATAL_ERROR(("Not enough memory!" TP_ENDL));
										error = TP_STATUS_ERROR;
									}
									else
									{
										_clibs_strncpy(filename, argv[arg_param], bufsize);
										TP_PARSE_INFO(("Public RSA key file: %s." TP_ENDL, filename));
										gen_mgr->PublicRSAKey = filename;
									}
								}
								else
								{
									error = TP_STATUS_ERROR;
									TP_FATAL_ERROR(("Public RSA key file name already specified: %s!" TP_ENDL, gen_mgr->PublicRSAKey));
								}
								break;

								// private RSA key file
							case PARA_PUBLIC_RSA_FILE:
								if (NULL == gen_mgr->PrivateRSAKey)
								{
									tPChar filename = NULL;
									volatile tUInt bufsize = _clibs_strlen(argv[arg_param]) + 1;

									filename = (tPChar)_clibs_malloc(bufsize);
									if (NULL == filename)
									{
										TP_FATAL_ERROR(("Not enough memory!" TP_ENDL));
										error = TP_STATUS_ERROR;
									}
									else
									{
										_clibs_strncpy(filename, argv[arg_param], bufsize);
										TP_PARSE_INFO(("Private RSA key file: %s." TP_ENDL, filename));
										gen_mgr->PrivateRSAKey = filename;
									}
								}
								else
								{
									error = TP_STATUS_ERROR;
									TP_FATAL_ERROR(("Public RSA key file name already specified: %s!" TP_ENDL, gen_mgr->PrivateRSAKey));
								}
								break;

							case PARA_RESET_BOARD:

								if (gen_mgr->dtr == false && gen_mgr->rts == false)
								{
									tPChar strlower = _clibs_calloc(strlen(argv[arg_param]), sizeof(tChar));
									if (strlower == NULL)
									{
										TP_FATAL_ERROR(("Not enough memory" TP_ENDL));
										error = TP_STATUS_ERROR;
										break;
									}

									tp_tools_tolower(strlower, argv[arg_param]);

									if (_clibs_sscanf(strlower, "rts=%d", &gen_mgr->reset_timing) == 1)
										gen_mgr->rts = true;

									if (_clibs_sscanf(strlower, "rtsdtr=%d", &gen_mgr->reset_timing) == 1) {
										gen_mgr->rts = true;
										gen_mgr->dtr = true;
									}

									if (_clibs_sscanf(strlower, "dtrrts=%d", &gen_mgr->reset_timing) == 1) {
										gen_mgr->rts = true;
										gen_mgr->dtr = true;
									}

									if (_clibs_sscanf(strlower, "dtr=%d", &gen_mgr->reset_timing) == 1) {
										gen_mgr->dtr = true;
									}

									if ((gen_mgr->dtr == false && gen_mgr->rts == false) || gen_mgr->reset_timing == 0)
									{
										TP_FATAL_ERROR(("Bad parameter format for RTS & DTR controls" TP_ENDL));
										error = TP_STATUS_ERROR;
									}
									else
									{
										TP_PARSE_INFO(("RTS & DTR controls setup completed" TP_ENDL));
									}
								}
								else
								{
									TP_FATAL_ERROR(("RTS & DTR controls already specified: !" TP_ENDL));
									error = TP_STATUS_ERROR;
								}


								break;

							default:
								/* Option not recognized */
								found = FALSE;
								break;
							}

							tp_tools_lcstr_delete(arg_tempstr);
						}
						else
						{
							TP_FATAL_ERROR(("Missing parameter for switch -%c." TP_ENDL, option));
							error = TP_STATUS_ERROR;
						}
					}
				}
			}
			else
			{
				/* Not a switch, check for command */
				if (TRGT_CFG_UNKNOWN == gen_mgr->cmd)
				{
					arg_tempstr = tp_tools_lcstr_new(argv[arg_opt]);

					if (0 == _clibs_strcmp("program", arg_tempstr))
					{
						gen_mgr->cmd = TRGT_CMD_PROG;
					}
					else if (0 == _clibs_strcmp("erase", arg_tempstr))
					{
						gen_mgr->cmd = TRGT_CMD_ERASE;
					}
					else if (0 == _clibs_strcmp("set", arg_tempstr))
					{
						gen_mgr->cmd = TRGT_CMD_SET;
					}
					else if (0 == _clibs_strcmp("dump", arg_tempstr))
					{
						gen_mgr->cmd = TRGT_CMD_DUMP;
					}
					else if (0 == _clibs_strcmp("upgrade", arg_tempstr))
					{
						gen_mgr->cmd = TRGT_CMD_FWUPG;
					}
					else if (0 == _clibs_strcmp("script", arg_tempstr))
					{
						gen_mgr->script = true;
						return TP_STATUS_OK;
					}
					else
					{
						TP_FATAL_ERROR(("Command %s not recognized." TP_ENDL, argv[arg_opt]));
						error = TP_STATUS_ERROR;
					}

					tp_tools_lcstr_delete(arg_tempstr);
				}
				else
				{
					TP_FATAL_ERROR(("Command specified more than once!" TP_ENDL));
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

tp_status_t tp_params_check(tp_general_config_t *gen_cfg)
{
	tp_status_t error = TP_STATUS_OK;

	/* Check command param */
	if (TRGT_CFG_UNKNOWN == gen_cfg->cmd)
	{
		if (gen_cfg->script == true)
			return TP_STATUS_OK;
		else
		{
			TP_FATAL_ERROR(("No command specified!" TP_ENDL));
			error = TP_STATUS_ERROR;
		}
	}

	/* Check input filename */
	if ((TRGT_CMD_PROG == gen_cfg->cmd) || (TRGT_CMD_FWUPG == gen_cfg->cmd) || (TRGT_CMD_DUMP == gen_cfg->cmd))
	{
		if (NULL == gen_cfg->in_filename)
		{
			TP_FATAL_ERROR(("Input filename needed!" TP_ENDL));
			error = TP_STATUS_ERROR;
		}
	}
	else
	{
		if (NULL != gen_cfg->in_filename)
		{
			TP_FATAL_ERROR(("Input filename not needed!" TP_ENDL));
			error = TP_STATUS_ERROR;
		}
	}

	/*
	Check programming device params. Defaults:
	- default speed = depends on core
	- programming speed = default speed, check correct value
	*/

	switch (gen_cfg->prgdev_cfg.dev)
	{
	case TP_PRGDEVID_UART:
		if (TRGT_CMD_FWUPG == gen_cfg->cmd)
		{
			gen_cfg->prgdev_cfg.cfg.uart.def_br = TP_PRGDEV_UART_DEFBR_FWUPG;
		}
		else
		{
			switch (gen_cfg->target)
			{
			case TRGT_FAMILY_TESEO2:
				gen_cfg->prgdev_cfg.cfg.uart.def_br = TP_PRGDEV_UART_DEFBR_TESEO2;
				break;

			case TRGT_FAMILY_TESEO3:
				gen_cfg->prgdev_cfg.cfg.uart.def_br = TP_PRGDEV_UART_DEFBR_TESEO3;
				break;

			case TRGT_FAMILY_TESEO5:
				gen_cfg->prgdev_cfg.cfg.uart.def_br = TP_PRGDEV_UART_DEFBR_TESEO5;
				break;

			default:
				break;
			}
		}

		if (0 == gen_cfg->prgdev_cfg.cfg.uart.prg_br)
		{
			gen_cfg->prgdev_cfg.cfg.uart.prg_br = gen_cfg->prgdev_cfg.cfg.uart.def_br;
		}

		if (FALSE == tp_uart_getbaudrateidx(gen_cfg->prgdev_cfg.cfg.uart.prg_br, &gen_cfg->prgdev_cfg.cfg.uart.prg_br_index))
		{
			TP_FATAL_ERROR(("Baud rate %d not valid." TP_ENDL, gen_cfg->prgdev_cfg.cfg.uart.prg_br));
			error = TP_STATUS_ERROR;
		}
		break;

	default:
		break;
	}

	return error;
}

tVoid tp_params_usage(tPChar exename)
{
	_clibs_printf(TP_ENDL);
	_clibs_printf("Usage: %s command [options]" TP_ENDL, exename);
	_clibs_printf(TP_ENDL);
	_clibs_printf("Available commands:" TP_ENDL);
	_clibs_printf("  program : program using XLoader protocol" TP_ENDL);
	_clibs_printf("  erase   : erase flash using XLoader protocol" TP_ENDL);
	_clibs_printf("  set     : set a region using XLoader protocol" TP_ENDL);
	_clibs_printf("  dump    : dump a region using XLoader protocol" TP_ENDL);
	_clibs_printf("  upgrade : flash using FWupgrade protocol" TP_ENDL);
	_clibs_printf(TP_ENDL);
	_clibs_printf("  script  : generate templates of script for each commands to ease usage");
	_clibs_printf(TP_ENDL);
	_clibs_printf("General options (in parenthesis command affected, a for all):" TP_ENDL);

	int usage_idx = 0;
	while (tp_common_parameters[usage_idx].option_letter != PARA_UNKNOWN)
	{
		_clibs_printf("-%c: %s" TP_ENDL, tp_common_parameters[usage_idx].option_letter, tp_common_parameters[usage_idx].Comment);
		usage_idx++;
	}

	_clibs_printf(TP_ENDL);
	_clibs_printf("Examples:" TP_ENDL);
	_clibs_printf("  %s program -f t3 -c COM1 -m SQI -e true -r true image.bin" TP_ENDL, exename);
	_clibs_printf(TP_ENDL);
}
