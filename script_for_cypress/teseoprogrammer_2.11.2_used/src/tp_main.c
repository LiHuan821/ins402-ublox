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
#include "tp_uart.h"
#include "tp_print.h"
#include "tp_tools.h"
#include "tp_help.h"
#include "fwupg/tp_fwupg.h"
#include "teseo2/tp_teseo2.h"
#include "teseo3/tp_teseo3.h"
#include "teseo5/tp_teseo5.h"

#if  _MSC_VER | _WIN32 | _WIN64
#include <direct.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "crc32.h"
#ifdef MULTI_COMS
#include "pthread.h"
#endif

/*****************************************************************************
   external declarations
*****************************************************************************/

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define TP_MAIN_INFO(x)         TP_INFO(x)
#define TP_MAIN_ERROR(x)        TP_ERROR(x)

#define TP_ARGV_MAX 50
#define TP_ARGV_LINE_MAX 255

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

typedef struct tp_manager_s
{
	/* Argument parse status */
	tPBool                  arg_parsed_table;

	/* Log configuration */
	tp_general_config_t     general_cfg;

	/* Target core configuration */
	tp_trgt_config_t        trgt_cfg;
} tp_manager_t;

/*****************************************************************************
   global variable definitions  (scope: module-exported)
*****************************************************************************/

/*****************************************************************************
   global variable definitions (scope: module-local)
*****************************************************************************/
#ifdef MULTI_COMS
pthread_mutex_t mutex_manager = PTHREAD_MUTEX_INITIALIZER;
#endif

static tp_manager_t   tp_manager = { 0 };
const tPChar TAG_BEGIN_PrivateRSAKey = { "-----BEGIN PRIVATE KEY-----" };
const tPChar TAG_END_PrivateRSAKey = { "-----END PRIVATE KEY-----" };
const tPChar TAG_BEGIN_PublicRSAKey = { "-----BEGIN PUBLIC KEY-----" };
const tPChar TAG_END_PublicRSAKey = { "-----END PUBLIC KEY-----" };

/*****************************************************************************
   function prototypes (scope: module-local)
*****************************************************************************/

static tp_status_t    tp_init(tUInt num_of_args);
static tVoid          tp_shutdown(tVoid);
static tp_status_t    tp_parse(tUInt argc, tPChar *argv);
static tp_status_t    tp_prgdev_open(tp_prgdev_config_t *prgdev_cfg, trgt_io_manager_t *io_mgr, tp_prgdev_instance_t inst);
static tVoid          tp_prgdev_close(tp_prgdev_config_t *prgdev_cfg, tp_prgdev_instance_t inst);
static tp_status_t    tp_src_read(tp_general_config_t * gen_cfg);
static tp_status_t    tp_RSA_read(tPChar RSA_File, const tPChar header, const tPChar footer, tPChar * key, tSize * key_size);
static tUInt          tp_params_checkunknown(tUInt num_of_args);
static tVoid          tp_script(tBool HTML);
static tVoid          tp_script_check_command_compliant(tChar tag, const tp_parameters_t * tp_parameters, FILE * out_stream, tBool HTML);
static tp_status_t    tp_script_extract(tUInt * argc, tPChar ** argv);
static tChar          tp_script_look_for_command(tPChar Tag);
static tVoid *        tp_fn_THREAD(void * p_data);

/*****************************************************************************
   function implementations (scope: module-local)
*****************************************************************************/

tp_status_t tp_init(tUInt num_of_args)
{
	tp_status_t error = TP_STATUS_OK;

	/* Initialize main manager */
	tp_manager.general_cfg.quiet = FALSE;
	tp_manager.general_cfg.help = FALSE;

	tp_manager.arg_parsed_table = (tPBool)_clibs_calloc(num_of_args, sizeof(tBool));

	if (NULL == tp_manager.arg_parsed_table)
	{
		TP_FATAL_ERROR(("Not enough memory" TP_ENDL));
		error = TP_STATUS_ERROR;
	}

	return error;
}

tVoid tp_shutdown(tVoid)
{
	for (tUInt instid = 0; instid < tp_manager.general_cfg.prgdev_cfg.cfg.uart.nb_ports; instid++)
		tp_prgdev_close(&tp_manager.general_cfg.prgdev_cfg, instid);

	tp_print_shutdown();

	if (NULL != tp_manager.general_cfg.out_filename)
	{
		_clibs_free(tp_manager.general_cfg.out_filename);
	}

	if (TP_PRGDEVID_UART == tp_manager.general_cfg.prgdev_cfg.dev)
	{
		if (NULL != tp_manager.general_cfg.prgdev_cfg.cfg.uart.port)
		{
			_clibs_free(tp_manager.general_cfg.prgdev_cfg.cfg.uart.port);
		}
	}

	if (NULL != tp_manager.general_cfg.in_filename)
	{
		_clibs_free(tp_manager.general_cfg.in_filename);
	}
}

tp_status_t tp_parse(tUInt argc, tPChar *argv)
{
	tp_status_t error = TP_STATUS_OK;

	// Check script analysis
	if (argc == 2 && argv[1][0] != '-' && _clibs_strcmp(argv[1], "script") != 0)
	{
		error = tp_script_extract(&argc, &argv);

		tp_manager.arg_parsed_table = (tPBool)_clibs_calloc(argc, sizeof(tBool));
	}

	error = tp_params_parse(argc, argv, &tp_manager.general_cfg, tp_manager.arg_parsed_table);

	if ((TP_STATUS_OK == error) && (FALSE == tp_manager.general_cfg.help))
	{
		error = tp_params_check(&tp_manager.general_cfg);

		if (TP_STATUS_OK == error)
		{
			if (TRGT_CMD_FWUPG == tp_manager.general_cfg.cmd)
			{
				/* Params are referring to FW upgrade protocol */
				tp_fwupg_params_init(&tp_manager.trgt_cfg.fwupg);

				error = tp_fwupg_params_parse(argc, argv, &tp_manager.trgt_cfg.fwupg, tp_manager.arg_parsed_table);

				if (TP_STATUS_OK == error)
				{
					tp_uart_getbaudrateidx(tp_manager.general_cfg.prgdev_cfg.cfg.uart.prg_br, &tp_manager.trgt_cfg.fwupg.br_fwupg_idx);
					error = tp_fwupg_params_check(&tp_manager.trgt_cfg.fwupg);
				}
			}
			else
			{
				/* Params are referring to XLoader protocol.
				   Parameters are specific for target */
				switch (tp_manager.general_cfg.target)
				{
				case TRGT_FAMILY_TESEO2:
					tp_teseo2_params_init(&tp_manager.trgt_cfg.t2);

					error = tp_teseo2_params_parse(argc, argv, &tp_manager.trgt_cfg.t2, tp_manager.arg_parsed_table);

					if (TP_STATUS_OK == error)
					{
						/* As baudrate change is handled by bootloader in T2, add it to configuration */
						if (TP_PRGDEVID_UART == tp_manager.general_cfg.prgdev_cfg.dev)
						{
							tp_manager.trgt_cfg.t2.baudrate = tp_manager.general_cfg.prgdev_cfg.cfg.uart.prg_br;
						}
						error = tp_teseo2_params_check(tp_manager.general_cfg.cmd, &tp_manager.trgt_cfg.t2);
					}
					break;

				case TRGT_FAMILY_TESEO3:

					tp_teseo3_params_init(&tp_manager.trgt_cfg.t3);

					error = tp_teseo3_params_parse(argc, argv, &tp_manager.trgt_cfg.t3, tp_manager.arg_parsed_table);

					if (TP_STATUS_OK == error)
					{
						error = tp_teseo3_params_check(tp_manager.general_cfg.cmd, &tp_manager.trgt_cfg.t3);
					}
					break;

				case TRGT_FAMILY_TESEO5:

					tp_teseo5_params_init(&tp_manager.trgt_cfg.t5);

					error = tp_teseo5_params_parse(argc, argv, &tp_manager.trgt_cfg.t5, tp_manager.arg_parsed_table);

					if (TP_STATUS_OK == error)
					{
						error = tp_teseo5_params_check(tp_manager.general_cfg.cmd, &tp_manager.trgt_cfg.t5);
					}
					break;

				default:
					break;
				}
			}
		}
	}

	return error;
}

tp_status_t tp_prgdev_open(tp_prgdev_config_t *prgdev_cfg, trgt_io_manager_t *io_mgr, tp_prgdev_instance_t inst)
{
	tp_status_t error = TP_STATUS_OK;

	switch (prgdev_cfg->dev)
	{
	case TP_PRGDEVID_UART:
		/* Open UART with default settings */
		if (TP_STATUS_OK == tp_uart_init(&prgdev_cfg->cfg.uart, prgdev_cfg->inst))
		{
			/* Open uart */
			tp_uart_open(inst);

			/* Flush RX/TX chars */
			tp_uart_flush(inst);

			/* Set I/O manager for programmer */
			io_mgr->dev_read_cb = tp_uart_read;
			io_mgr->dev_write_cb = tp_uart_write;
		}
		else
		{
			error = TP_STATUS_ERROR;
		}
		break;

	default:
		TP_FATAL_ERROR(("Communication channel unknown: %d", prgdev_cfg->dev));
		error = TP_STATUS_ERROR;
		break;
	}

	return error;
}

tVoid tp_prgdev_close(tp_prgdev_config_t *prgdev_cfg, tp_prgdev_instance_t inst)
{
	switch (prgdev_cfg->dev)
	{
	case TP_PRGDEVID_UART:
		tp_uart_close(inst);
		break;

	default:
		break;
	}
}

tp_status_t tp_RSA_read(tPChar RSA_File, const tPChar header, const tPChar footer, tPChar * key, tSize * key_size)
{
	tUInt error = TP_STATUS_OK;

	/* Open input file */
	FILE * rsa_handle = fopen(RSA_File, "rb");

	if (NULL == rsa_handle)
	{
		TP_MAIN_ERROR(("RSA public key %s access denied !" TP_ENDL, RSA_File));
		error = TP_STATUS_ERROR;
	}
	else
	{
		/* Get public key */
		/* look for TAG BEGIN  */
		int position_rsa_begin = 0;
		int position_rsa_end = 0;
		tPChar buf_research = (tPChar)_clibs_malloc(strlen(header));
		boolean TAG_found = false;

		while (TAG_found == false && TP_STATUS_OK == error)
		{
			if (fseek(rsa_handle, position_rsa_begin, SEEK_SET) != 0)
				error = TP_STATUS_ERROR;

			if (fread(buf_research, 1, strlen(header), rsa_handle) != strlen(header))
				error = TP_STATUS_ERROR;

			if (strncmp(buf_research, header, strlen(header)) == 0)
			{
				TAG_found = true;

				position_rsa_begin += strlen(header);
				_clibs_free(buf_research);
			}
			else
				position_rsa_begin++;
		}

		/* look for TAG END  */
		TAG_found = false;
		position_rsa_end = position_rsa_begin;
		buf_research = (tPChar)_clibs_malloc(strlen(footer));

		while (TAG_found == false && TP_STATUS_OK == error)
		{
			if (fseek(rsa_handle, position_rsa_end, SEEK_SET) != 0)
				error = TP_STATUS_ERROR;

			if (fread(buf_research, 1, strlen(footer), rsa_handle) != strlen(footer))
				error = TP_STATUS_ERROR;

			if (strncmp(buf_research, footer, strlen(footer)) == 0)
			{
				TAG_found = true;

				_clibs_free(buf_research);
			}
			else
				position_rsa_end++;
		}

		if (TP_STATUS_OK == error && position_rsa_begin < position_rsa_end)
		{
			// Load rsa key
			if (fseek(rsa_handle, position_rsa_begin, SEEK_SET) != 0)
				error = TP_STATUS_ERROR;

			*key = (tPChar)_clibs_malloc(position_rsa_end - position_rsa_begin);

			if (*key == NULL)
				error = TP_STATUS_ERROR;
			else
			{
				if (fread(*key, 1, position_rsa_end - position_rsa_begin, rsa_handle) != position_rsa_end - position_rsa_begin)
					error = TP_STATUS_ERROR;
			}

			*key_size = position_rsa_end - position_rsa_begin;
		}
	}

	return error;

}


tp_status_t tp_src_read(tp_general_config_t * gen_cfg)
{
	tp_status_t error = TP_STATUS_OK;
	FILE *src_handle;

	trgt_io_manager_t  * io_mgr = &gen_cfg->io_mgr;

	/* RSA public & private key */
	if (gen_cfg->PublicRSAKey != NULL && gen_cfg->PrivateRSAKey != NULL)
	{
		error = tp_RSA_read(gen_cfg->PublicRSAKey, TAG_BEGIN_PublicRSAKey, TAG_END_PublicRSAKey, &io_mgr->public_rsa_key, &io_mgr->public_rsa_size);

		if (TP_STATUS_OK == error)
		{
			error = tp_RSA_read(gen_cfg->PrivateRSAKey, TAG_BEGIN_PrivateRSAKey, TAG_END_PrivateRSAKey, &io_mgr->private_rsa_key, &io_mgr->private_rsa_size);
		}
	}
	else
	{
		/* No RSA data to append */
		io_mgr->public_rsa_size = 0;
		io_mgr->private_rsa_size = 0;
	}

	if (TP_STATUS_OK != error)
		return error;

	/* Open input file */
	src_handle = fopen(gen_cfg->in_filename, "rb");

	if (NULL == src_handle)
	{
		TP_ERROR(("File %s does not exist!" TP_ENDL, gen_cfg->in_filename));
		//TP_TOOLS_DLL_ERROR(("File %s does not exist!" TP_ENDL, gen_cfg.in_filename));
		error = TP_STATUS_ERROR;
	}

	if (TP_STATUS_OK == error)
	{
		/* Get file info */
		fseek(src_handle, 0L, SEEK_END);
		io_mgr->src_size = ftell(src_handle);
		rewind(src_handle);
		//TP_MAIN_INFO(("File %s, size %d" TP_ENDL, gen_cfg->in_filename, io_mgr->src_size));

		/* Create a buffer and read it */
		int size = io_mgr->src_size + io_mgr->public_rsa_size + io_mgr->private_rsa_size;

		/* Patch 64 bits platform to align with 64 bit word*/
		if (sizeof(short int) == 4 && ((io_mgr->src_size + io_mgr->public_rsa_size + io_mgr->private_rsa_size) % 8) > 0)
		{
			size += 8 - ((io_mgr->src_size + io_mgr->public_rsa_size + io_mgr->private_rsa_size) % 8);
		}

		/* Allocate buffer for firmware */
		io_mgr->src_buf = (tPChar)_clibs_malloc(io_mgr->src_size + io_mgr->public_rsa_size + io_mgr->private_rsa_size);

		if (NULL == io_mgr->src_buf)
		{
			TP_MAIN_ERROR(("Not enough memory!" TP_ENDL));
			error = TP_STATUS_ERROR;
		}
	}

	if (TP_STATUS_OK == error)
	{
		tUInt read_bytes;

		/* Copy file to buffer */
		read_bytes = fread(io_mgr->src_buf, 1, io_mgr->src_size, src_handle);

		if (read_bytes != io_mgr->src_size)
		{
			TP_MAIN_ERROR(("Read failed! Size %d, read %d." TP_ENDL, io_mgr->src_size, read_bytes));
			error = TP_STATUS_ERROR;
		}
		else
		{
			/* Append RSA key at end of buffer PRIVATE & PUBLIC */
			if (io_mgr->public_rsa_key != 0 && io_mgr->private_rsa_size != 0)
			{
				_clibs_memcpy(io_mgr->src_buf + io_mgr->src_size, io_mgr->private_rsa_key, io_mgr->private_rsa_size);
				_clibs_memcpy(io_mgr->src_buf + io_mgr->src_size + io_mgr->private_rsa_size, io_mgr->public_rsa_key, io_mgr->public_rsa_size);
				io_mgr->src_size += (io_mgr->public_rsa_size + io_mgr->private_rsa_size);
			}

			/* Evaluate CRC32 */
			tUInt file_crc32 = 0;
			CRC32_EVAL_INT(file_crc32, file_crc32, io_mgr->src_size);
			CRC32_EVAL_BUF(file_crc32, file_crc32, io_mgr->src_buf, io_mgr->src_size);

			io_mgr->src_crc32 = file_crc32;

			//TP_PRINT(("File %s read. Size %d, crc32 0x%x" TP_ENDL, gen_cfg.in_filename, io_mgr->src_size, file_crc32));
		}
	}

	if (NULL != src_handle)
	{
		fclose(src_handle);
	}

	return error;
}

tp_status_t tp_src_write(trgt_io_manager_t *io_mgr)
{
	tp_status_t error = TP_STATUS_OK;

	/* Open input file */
	io_mgr->out_file = fopen(tp_manager.general_cfg.in_filename, "wb");

	if (NULL == io_mgr->out_file)
	{
		TP_ERROR(("File access is denied %s!" TP_ENDL, tp_manager.general_cfg.in_filename));
		error = TP_STATUS_ERROR;
	}

	return error;
}

tUInt tp_params_checkunknown(tUInt num_of_args)
{
	tp_status_t error = TP_STATUS_OK;
	tUInt arg_curr = 1;

	while ((arg_curr < num_of_args) && (TP_STATUS_OK == error))
	{
		if (tp_manager.arg_parsed_table[arg_curr] == FALSE)
		{
			error = TP_STATUS_ERROR;
		}
		else
		{
			arg_curr++;
		}
	}

	return (TP_STATUS_ERROR == error) ? arg_curr : 0;
}

tVoid tp_usage(tPChar exename)
{
	tp_print_version();

	tp_params_usage(exename);

	if (TRGT_CMD_FWUPG == tp_manager.general_cfg.cmd)
	{
		tp_fwupg_params_usage();
	}
	else
	{
		/* Print params specific for target */
		switch (tp_manager.general_cfg.target)
		{
		case TRGT_FAMILY_TESEO2:
			tp_teseo2_params_usage();
			break;

		case TRGT_FAMILY_TESEO3:
			tp_teseo3_params_usage();
			break;

		case TRGT_FAMILY_TESEO5:
			tp_teseo5_params_usage();
			break;

		default:
			_clibs_printf("Use -h with target family for specific help : -h -f [t2,t3 or t5]" TP_ENDL);
			break;
		}
	}
}

tVoid tp_script(tBool HTML)
{
	/* For each command generate a template */
	tUInt ModeName = 0;
	FILE * handle_html = NULL;
	const tPChar Modescript[] = { "program.tp","upgrade.tp","dump.tp","set.tp","erase.tp",NULL };
	const tPChar Modehtml[] = { "doc_program.html","doc_upgrade.html","doc_dump.html","doc_set.html","doc_erase.html",NULL };
	const tPChar ModeCommand[] = { "PROGRAM","UPGRADE","DUMP","SET","ERASE",NULL };
	const tPChar ModeDescription[] = { "Load TESEO firmware","Upgrade TESEO firmware","Dump Teseo memory","Set a 32 bit value in Teseo memory","Erase Teseo flash memory",NULL };
	const char ModeTag[] = { 'p','u','d','s','e' };

	tUInt TargetName = 0;
	const tp_parameters_t * Targets[] = { tp_t2_parameters,tp_t3_parameters,tp_t5_parameters,NULL };
	const tPChar TargetsNames[] = { "t2_","t3_","t5_", NULL };
	const tPChar TargetsFullNames[] = { "Teseo II [STA8088]","Teseo III [STA8089/90]","Teseo V [STA8100]", NULL };

	tUInt cmdidx = 0;
	tPChar script_name = _clibs_malloc(100);

	if (script_name == NULL)
	{
		TP_MAIN_ERROR(("Not enough memory" TP_ENDL));
		return;
	}

	// Select right output
	const tPChar * Mode = (HTML) ? Modehtml : Modescript;

	// Open doc.html global
	if (HTML)
	{
		handle_html = fopen("index.html", "wt");

		if (handle_html == NULL)
		{
			TP_MAIN_ERROR(("Access to file denied" TP_ENDL));
			return;
		}

		fprintf(handle_html, html_header);
		fprintf(handle_html, html_logo);
		fprintf(handle_html, "\n<h1>Teseo Programmer user manual</h1>");
		fprintf(handle_html, "\n<h2>Teseo Programmer with script (easy mode)</h2>");
		fprintf(handle_html, "\n<p>This script (*.tp) replace the set of command required by teseoprogrammer.exe</br>");

		fprintf(handle_html, "\n<li>First generate all script patterns by invoking :</li></br>");
		fprintf(handle_html, "\n\n<p style = \"color:rgb(255,0,0);\"><b>TeseoProgrammer.exe script</b></p>");

		fprintf(handle_html, "\n<p>It's possible to remove some section in script to custom it for your target</br>");
		fprintf(handle_html, "\nAfter changes, please rename your '.tp' script to avoid overwriting of script command!</p></br>");

		fprintf(handle_html, "\n<li>Call your script :</li></br>");
		fprintf(handle_html, "\n\n<p style = \"color:rgb(255,0,0);\"><b>TeseoProgrammer.exe my_script_name.tp</b></p>");

		fprintf(handle_html, "\n<h2>Teseo Programmer in line</h2>");
		fprintf(handle_html, "\n<p>Invoke command :</p></br>");
		fprintf(handle_html, "\n<li>TeseoProgammer.exe [command] options </li></br>");
		fprintf(handle_html, "\n<p>Available commands:</p></br>");
		fprintf(handle_html, "\n  <li><b>program</b> : program using XLoader protocol</li></br>");
		fprintf(handle_html, "\n  <li><b>erase</b>   : erase flash using XLoader protocol</li></br>");
		fprintf(handle_html, "\n  <li><b>set</b>     : set a region using XLoader protocol</li></br>");
		fprintf(handle_html, "\n  <li><b>dump</b>    : dump a region using XLoader protocol</li></br>");
		fprintf(handle_html, "\n  <li><b>upgrade</b> : flash using FWupgrade protocol</li></br>");
		fprintf(handle_html, "\n  <li><b>script</b>  : generate templates of script for each commands to ease usage</li></br></p>");
		fprintf(handle_html, "\n<p>Available options:</p></br>");

	}

	// Verify folder exist
	if (HTML)
	{
		const char folderHelp[] = { "help" };

		struct stat sb;

		if (stat(folderHelp, &sb) == -1 || !(sb.st_mode | S_IFDIR))
		{
#if  _MSC_VER | _WIN32 | _WIN64
			if (_mkdir(folderHelp) != 0)
#else
			if (mkdir(folderHelp, S_IRUSR | S_IWUSR | S_IXUSR) != 0)
#endif

			{
				TP_ERROR(("Cannot create folder %s" TP_ENDL, folderHelp));
				return;
			}
		}
	}
	else {
		const char folderHelp[] = { "script" };

		struct stat sb;

		if (stat(folderHelp, &sb) == -1 || !(sb.st_mode | S_IFDIR))
		{
#if  _MSC_VER | _WIN32 | _WIN64
			if (_mkdir(folderHelp) != 0)
#else
			if (mkdir(folderHelp, S_IRUSR | S_IWUSR | S_IXUSR) != 0)
#endif

			{
				TP_ERROR(("Cannot create folder %s" TP_ENDL, folderHelp));
				return;
			}
		}
	}

	while (TargetsNames[TargetName] != NULL)
	{
		ModeName = 0;

		if (HTML)
		{
			fprintf(handle_html, "\n<h3>%s</h3>", TargetsFullNames[TargetName]);
		}

		while (Mode[ModeName] != NULL)
		{
			if (HTML)
				_clibs_strcpy(script_name, "help/");
			else
				_clibs_strcpy(script_name, "script/");

			_clibs_strcpy(script_name + strlen(script_name), TargetsNames[TargetName]);
			_clibs_strcpy(script_name + strlen(script_name), Mode[ModeName]);

			FILE * template = fopen(script_name, "wt");

			if (template == NULL)
			{
				TP_MAIN_ERROR(("Access to file denied" TP_ENDL));
				return;
			}

			if (HTML == false)
			{
				fprintf(template, "\nST-Microelectronics, Teseo programmer script\n\n");
				fprintf(template, "\n This file replace the set of command required by teseoprogrammer.exe");
				fprintf(template, "\n Invoke teseoprogrammer.exe following by your script path : ");
				fprintf(template, "\n  -> teseoprogrammer.exe upgrade_my_product.tp");
				fprintf(template, "\n\n\n It's possible to remove some section in script to custom it for your target");
				fprintf(template, "\n After changes, please rename your '.tp' script to avoid overwriting of script command!");
				fprintf(template, "\n\n\n The script command must always start with this tag :");
				fprintf(template, "\nSTART:%s", ModeCommand[ModeName]);

				// Common parameters
				tp_script_check_command_compliant(ModeTag[ModeName], tp_common_parameters, template, false);
				tp_script_check_command_compliant(ModeTag[ModeName], tp_upgrade_parameters, template, false);
				tp_script_check_command_compliant(ModeTag[ModeName], Targets[TargetName], template, false);
			}
			else
			{
				fprintf(template, html_header);
				fprintf(handle_html, "\n<li><a href=\"./%s\"style=\"color:#39A9DB\">%s</a></h1></li>", script_name, ModeCommand[ModeName]);
				fprintf(template, "\n<h1>Syntax of command : %s</h1>", ModeCommand[ModeName]);
				fprintf(template, "\n<h2>%s</h2>", ModeDescription[ModeName]);
				fprintf(template, html_table_begin);

				// Common parameters
				tp_script_check_command_compliant(ModeTag[ModeName], tp_common_parameters, template, true);
				tp_script_check_command_compliant(ModeTag[ModeName], tp_upgrade_parameters, template, true);
				tp_script_check_command_compliant(ModeTag[ModeName], Targets[TargetName], template, true);

				fprintf(template, html_table_end);
				fprintf(template, "\n</br></br><a href=\"./../index.html\"style=\"color:#39A9DB\">Back to index</a></h1>");
				fprintf(template, html_footer);
			}


			fclose(template);

			ModeName++;
		}

		TargetName++;
	}

	if (HTML)
	{
		fprintf(handle_html, "\n</html>");
		fclose(handle_html);
		printf("\nA documentation has been generated.Please, open index.html to access it.\n");
	}
}

tVoid tp_script_check_command_compliant(tChar cmdtag, const tp_parameters_t * tp_parameters, FILE * out_stream, tBool HTML)
{

	tPChar tag;
	tPChar choice;
	tPChar defaultchoice;

	tUInt cmdidx = 0;

	while (tp_parameters[cmdidx].option_letter != PARA_UNKNOWN)
	{
		tUInt levelresearch = 0;
		tPChar comment = tp_parameters[cmdidx].Comment;

		// look for [xxxtagxxx]
		while (*comment != 0)
		{
			if (levelresearch == 0 && * comment == '[')
				levelresearch = 1;

			if (levelresearch == 1 && * comment == cmdtag)
				levelresearch = 2;

			if (levelresearch == 2 && * comment == ']')
				levelresearch = 3;

			comment++;
		}

		if (levelresearch == 3)
		{
			tPChar desc = _clibs_malloc(strlen(tp_parameters[cmdidx].Syntax) + 1);
			_clibs_strcpy(desc, tp_parameters[cmdidx].Syntax);

			/* get items */
			tag = strtok(desc, ";");
			choice = strtok(NULL, ";");
			defaultchoice = strtok(NULL, ";");

			if (false == HTML)
			{
				fprintf(out_stream, "\n\n# usage         : -%c", tp_parameters[cmdidx].option_letter);
				fprintf(out_stream, "\n# description : %s", tp_parameters[cmdidx].Comment);
				fprintf(out_stream, "\n# type/choice : %s", choice);
				fprintf(out_stream, "\n# sample      : %s", defaultchoice);
				fprintf(out_stream, "\n%s:%s", tag, defaultchoice);
			}
			else
			{
				fprintf(out_stream, "\n<tr><td>-%c</td><td>%s</td><td>%s</td><td>%s</td></tr>", tp_parameters[cmdidx].option_letter, tp_parameters[cmdidx].Comment, choice, defaultchoice);
			}
			_clibs_free(desc);
		}

		cmdidx++;
	}

}

tp_status_t tp_script_extract(tUInt * argc, tPChar ** argv)
{
	tPChar * ARGV;
	tUInt ARGC = 0;
	tPChar line;

	FILE * script_handle = fopen((*argv)[1], "rt");

	if (script_handle == NULL)
	{
		TP_MAIN_ERROR(("Access denied" TP_ENDL));
		return TP_STATUS_ERROR;
	}

	ARGV = _clibs_malloc(sizeof(tPChar *) * TP_ARGV_MAX);
	line = _clibs_malloc(sizeof(tChar) * TP_ARGV_LINE_MAX);

	if (ARGV == NULL || line == NULL)
	{
		TP_MAIN_ERROR(("Not enough memory" TP_ENDL));
		fclose(script_handle);
		return TP_STATUS_ERROR;
	}

	ARGV[ARGC++] = *argv[0];

	/* Extract command */
	while (fgets(line, TP_ARGV_LINE_MAX, script_handle) == line)
	{
		if (strncmp(line, "START:", strlen("START:")) == 0)
		{
			tPChar cmd = _clibs_calloc(strlen(line + strlen("START:")), 1);
			_clibs_memcpy(cmd, line + strlen("START:"), strlen(line + strlen("START:")));
			tp_tools_remove_endline(&cmd);
			ARGV[ARGC++] = cmd;
			break;
		}
	}

	/* Extract parameters */
	while (fgets(line, TP_ARGV_LINE_MAX, script_handle) == line)
	{
		if (strpbrk(line, "#") != line && strpbrk(line, " ") != line && strpbrk(line, ":") > line)
		{
			tPChar tag = _clibs_calloc(strpbrk(line, ":") - line + 1, 1);
			if (tag == NULL)
			{
				TP_MAIN_ERROR(("Not enough memory" TP_ENDL));
				fclose(script_handle);
				return TP_STATUS_ERROR;
			}
			_clibs_memcpy(tag, line, strpbrk(line, ":") - line);

			// look for parameter into dictionnary
			char option_car = tp_script_look_for_command(tag);

			if (option_car == PARA_UNKNOWN)
			{
				TP_MAIN_ERROR(("Option %s is unknown in file %s" TP_ENDL, tag, (*argv)[1]));
				_clibs_free(tag);
				fclose(script_handle);
				return TP_STATUS_ERROR;
			}

			// copy option
			tPChar option_argv = _clibs_calloc(3, sizeof(char));
			if (option_argv == NULL)
			{
				TP_MAIN_ERROR(("Not enough memory" TP_ENDL));
				fclose(script_handle);
				return TP_STATUS_ERROR;
			}
			_clibs_sprintf(option_argv, "-%c", option_car);
			ARGV[ARGC++] = option_argv;

			tPChar value_argv = _clibs_calloc(strlen(strpbrk(line, ":")) + 1, sizeof(char));
			if (value_argv == NULL)
			{
				TP_MAIN_ERROR(("Not enough memory" TP_ENDL));
				fclose(script_handle);
				return TP_STATUS_ERROR;
			}
			_clibs_memcpy(value_argv, strpbrk(line, ":") + 1, strlen(strpbrk(line, ":")));
			tp_tools_remove_endline(&value_argv);
			ARGV[ARGC++] = value_argv;
			_clibs_free(tag);
		}
	}

	fclose(script_handle);

	*argc = ARGC;
	*argv = ARGV;

	return TP_STATUS_OK;
}

tChar tp_script_look_for_command(tPChar Tag)
{
	const tp_parameters_t * Targets[] = { tp_common_parameters, tp_upgrade_parameters, tp_t2_parameters,tp_t3_parameters,tp_t5_parameters,NULL };
	tUInt TargetsName = 0;

	while (Targets[TargetsName] != NULL)
	{
		tUInt OptionName = 0;

		while (Targets[TargetsName][OptionName].option_letter != PARA_UNKNOWN)
		{
			if (_clibs_strncmp(Tag, Targets[TargetsName][OptionName].Syntax, strlen(Tag)) == 0)
				return Targets[TargetsName][OptionName].option_letter;

			OptionName++;
		}

		TargetsName++;
	}

	return PARA_UNKNOWN;
}



/*****************************************************************************
   function implementations (scope: module-exported)
*****************************************************************************/

int main(int argc, char* argv[])
{
	tInt exit_code = 0;
#ifdef MULTI_COMS
	pthread_t * ThreadCollection;
#endif

	/* Initialize TeseoProgrammer */
	if (TP_STATUS_ERROR == tp_init(argc))
	{
		exit_code = 1;
	}

	/* Parse parameters */
	if (TP_STATUS_ERROR == tp_parse(argc, argv))
	{
		exit_code = 1;
	}

	/* check if script generation is required */
	if (tp_manager.general_cfg.script == true)
	{
		tp_script(false);

		TP_PRINT(("Please find templates into \"script\" folder" TP_ENDL));

		exit_code = 0;
		return exit_code;
	}

	/* Check if help is requested */
	if (TRUE == tp_manager.general_cfg.help)
	{
		tp_usage(argv[0]);

		tp_script(true);

		exit_code = 0;
	}
	else if (0 == exit_code)
	{
		/* Check if all params where parsed */
		tUInt unknown_param;

		unknown_param = tp_params_checkunknown(argc);

		if (0 != unknown_param)
		{
			TP_FATAL_ERROR(("Switch %s not valid!" TP_ENDL, argv[unknown_param]));
			exit_code = 1;
		}

		if (0 == exit_code)
		{
			/* Init print module */
			if (TP_STATUS_OK == tp_print_init(&tp_manager.general_cfg))
			{
				tp_print_version();
			}
		}

		/* Open input file if needed and read image */
		if (0 == exit_code)
		{
			if (NULL != tp_manager.general_cfg.in_filename)
			{
				switch (tp_manager.general_cfg.cmd)
				{
				case TRGT_CMD_DUMP:
					if (TP_STATUS_ERROR == tp_src_write(&tp_manager.general_cfg.io_mgr))
					{
						TP_ERROR(("Dump file access failed!" TP_ENDL));
						exit_code = 1;
					}

					break;

				default:
					if (TP_STATUS_ERROR == tp_src_read(&tp_manager.general_cfg))
					{
						TP_ERROR(("Binary read failed!" TP_ENDL));
						exit_code = 1;
					}
					break;
				}

			}
			else
			{
				/* Init already set to NULL and 0 all params of io_mgr */
			}
		}

#ifdef MULTI_COMS
		ThreadCollection = _clibs_calloc(sizeof(pthread_t *), tp_manager.general_cfg.prgdev_cfg.cfg.uart.nb_ports);

		for (tUInt comId = 0; comId < tp_manager.general_cfg.prgdev_cfg.cfg.uart.nb_ports; comId++)
		{
			tUInt * pcomdid = _clibs_malloc(sizeof(tUInt));
			*pcomdid = comId;

			int ret = pthread_create(
				&ThreadCollection[comId], NULL,
				tp_fn_THREAD, (void *)pcomdid
			);

			if (ret)
			{
				TP_ERROR(("Binary read failed!" TP_ENDL));
				exit_code = 1;
			}
		}

		/* Wait for threads. */
		for (tUInt threadid = 0; threadid < tp_manager.general_cfg.prgdev_cfg.cfg.uart.nb_ports && exit_code != 1; threadid++)
		{
			pthread_join(ThreadCollection[threadid], NULL);
		}
#else
		tUInt * pcomdid = _clibs_malloc(sizeof(tUInt));
		*pcomdid = 0;
		tp_fn_THREAD(pcomdid);
#endif

			}

	tp_shutdown();

	if (1 == exit_code)
	{
		TP_FATAL_ERROR(("Use -h to get help" TP_ENDL));
	}

	return exit_code;
		}

void * tp_fn_THREAD(void * p_data)
{
	tInt exit_code = 0;
	tUInt instance = (tUInt)(*(tUInt *)p_data);

	tp_manager.general_cfg.prgdev_cfg.inst = instance;

	/* Open communication channel */
	TP_MAIN_INFO(("Open com instance %d" TP_ENDL, instance));

	if (TP_STATUS_ERROR == tp_prgdev_open(&tp_manager.general_cfg.prgdev_cfg, &tp_manager.general_cfg.io_mgr, instance))
	{
		TP_ERROR(("Communication channel open failed!" TP_ENDL));
		exit_code = 1;
	}

	/* Set time API */
	tp_manager.general_cfg.io_mgr.gettime = tp_tools_gettime;

	/* Check if reset is required before operate       */
	/* RTS and/or DTR is toggle to execute this action */
	if (tp_manager.general_cfg.reset_timing != 0)
	{
		tp_uart_flowcontrol(tp_manager.general_cfg.rts, tp_manager.general_cfg.dtr, tp_manager.general_cfg.reset_timing, instance);
	}

	/* Run specific command */
	if (0 == exit_code)
	{
		if (TRGT_CMD_FWUPG == tp_manager.general_cfg.cmd)
		{
			/* Calls fwupg protocol */
			if (TP_STATUS_ERROR == tp_fwupg_exec(&tp_manager.trgt_cfg.fwupg, instance, &tp_manager.general_cfg.io_mgr, &tp_manager.general_cfg.prgdev_cfg))
			{
				exit_code = 1;
			}
		}
		else
		{
			switch (tp_manager.general_cfg.target)
			{
			case TRGT_FAMILY_TESEO2:
				if (TP_STATUS_ERROR == tp_teseo2_exec(tp_manager.general_cfg.cmd, instance, &tp_manager.trgt_cfg.t2, &tp_manager.general_cfg.io_mgr, &tp_manager.general_cfg.prgdev_cfg))
				{
					exit_code = 1;
				}
				break;

			case TRGT_FAMILY_TESEO3:
				if (TP_STATUS_ERROR == tp_teseo3_exec(tp_manager.general_cfg.cmd, instance, &tp_manager.trgt_cfg.t3, &tp_manager.general_cfg.io_mgr, &tp_manager.general_cfg.prgdev_cfg))
				{
					exit_code = 1;
				}
				break;

			case TRGT_FAMILY_TESEO5:
				if (TP_STATUS_ERROR == tp_teseo5_exec(tp_manager.general_cfg.cmd, instance, &tp_manager.trgt_cfg.t5, &tp_manager.general_cfg.io_mgr, &tp_manager.general_cfg.prgdev_cfg))
				{
					exit_code = 1;
				}
				break;

			default:
				exit_code = 1;
				break;
			}
		}
	}

	return (tVoid *)NULL;
}



