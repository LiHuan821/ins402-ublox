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
#include "tp_print.h"

/*****************************************************************************
   external declarations
*****************************************************************************/

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define TP_PRINT_STRING_SIZE          512
#define TP_PRINT_STRCAT(i, s)         (i) += snprintf( &tp_print_manager.string[i], TP_PRINT_STRING_SIZE - (i), "%s", s)
#define TP_PRINT_VSTRCAT(i, f, v)     (i) += vsnprintf( &tp_print_manager.string[i], TP_PRINT_STRING_SIZE - (i), f, v)

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

typedef struct tp_print_manager_s
{
	tPChar      string;
	tUInt       type;
	FILE *      output_handle;
} tp_print_manager_t;

/*****************************************************************************
   global variable definitions  (scope: module-exported)
*****************************************************************************/

/*****************************************************************************
   global variable definitions (scope: module-local)
*****************************************************************************/

static tp_print_manager_t tp_print_manager = { 0 };

#ifdef MULTI_COMS
pthread_mutex_t mutex_print = PTHREAD_MUTEX_INITIALIZER;
#endif

/*****************************************************************************
   function prototypes (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   function implementations (scope: module-local)
*****************************************************************************/

#if defined(_MSC_VER )
/*
  snprintf is defined from VS 2015 upwards
*/
#if (_MSC_VER <= 1800)
__inline int snprintf(char *outBuf, size_t size, const char *format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = vsnprintf(outBuf, size, format, ap);
	va_end(ap);

	return count;
}
#endif
#endif

/*****************************************************************************
   function implementations (scope: module-exported)
*****************************************************************************/

tp_status_t tp_print_init(tp_general_config_t *gen_cfg)
{
	tp_status_t error = TP_STATUS_OK;

	/* Disable stdout buffering */
	setvbuf(stdout, NULL, _IONBF, 0);

	if (FALSE == gen_cfg->quiet)
	{
		if (NULL != gen_cfg->out_filename)
		{
			/* Log must go to a file */
			tp_print_manager.output_handle = fopen(gen_cfg->out_filename, "wt");

			if (NULL == tp_print_manager.output_handle)
			{
				TP_FATAL_ERROR(("[ERROR] Cannot open file %s for writing!" TP_ENDL, gen_cfg->out_filename));
				error = TP_STATUS_ERROR;
			}
		}
		else
		{
			tp_print_manager.output_handle = NULL;
		}

		if (TP_STATUS_OK == error)
		{
			/* Get memory for print string buffer */
			tp_print_manager.string = (tPChar)_clibs_malloc(TP_PRINT_STRING_SIZE);

			if (NULL == tp_print_manager.string)
			{
				TP_FATAL_ERROR(("[ERROR] Not enough memory!" TP_ENDL));
				error = TP_STATUS_ERROR;
			}
		}
	}
	else
	{
		tp_print_manager.type = TP_PRINT_QUIET;
	}

	if (TP_STATUS_ERROR == error)
	{
		tp_print_shutdown();
	}

	return error;
}

tVoid tp_print_shutdown(tVoid)
{
	if (NULL != tp_print_manager.output_handle)
	{
		fclose(tp_print_manager.output_handle);
	}
}

tVoid tp_print_settype(const tU8 type)
{
	if (TP_PRINT_QUIET != tp_print_manager.type)
	{
		tp_print_manager.type = type;
	}
}

tVoid tp_print(const tPChar msg, ...)
{
	tp_status_t error = TP_STATUS_OK;

	LOCK_PRINT

		// Prevent display before initializing
		if (tp_print_manager.string == NULL)
		{
			/* Get memory for print string buffer */
			tp_print_manager.string = (tPChar)_clibs_malloc(TP_PRINT_STRING_SIZE);

			if (tp_print_manager.string == NULL)
				return;
		}

	if (TP_PRINT_QUIET != tp_print_manager.type)
	{
		tUInt index = 0;

		switch (tp_print_manager.type)
		{
		case TP_PRINT_STDMSG:
			break;

		case TP_PRINT_DEBUG:
			TP_PRINT_STRCAT(index, "[INFO] ");
			break;

		case TP_PRINT_ERROR:
			TP_PRINT_STRCAT(index, "[ERROR] ");
			break;

		case TP_PRINT_STREAM:
			if (NULL != tp_print_manager.output_handle)
				TP_PRINT_STRCAT(index, "[STREAM] ");
			else
				return;
			break;

		default:
			break;
		}

		if (TP_STATUS_OK == error)
		{
			va_list ap;

			va_start(ap, msg);

			TP_PRINT_VSTRCAT(index, msg, ap);

			if (NULL != tp_print_manager.output_handle)
			{
				fprintf(tp_print_manager.output_handle, "%s", tp_print_manager.string);
			}
			else
			{
				if (tp_print_manager.type != TP_PRINT_DEBUG && tp_print_manager.type != TP_PRINT_STREAM)
					_clibs_printf("%s", tp_print_manager.string);

				fflush(stdout);
			}

			tp_print_manager.string[sizeof(tp_print_manager.string) - 1] = '\0';

			va_end(ap);
		}

		tp_print_manager.type = TP_PRINT_INVALID;
	}

	UNLOCK_PRINT
}

tVoid tp_print_version(tVoid)
{
	_clibs_printf(TP_ENDL "Aceinna STA9100 upgrade tool v%s" TP_ENDL, TP_VERSION_STRING);
	_clibs_printf("Flasher revisions:" TP_ENDL);
	_clibs_printf("- %s" TP_ENDL, &xldr_teseo3_bootloader[XLDR_TESEO3_FLASHERVER_OFFSET]);
	_clibs_printf("- %s" TP_ENDL, &xldr_teseo5_bootloader_cut2[XLDR_TESEO5_FLASHERVER_OFFSET]);

#if defined(_MSC_VER)
	_clibs_printf("Built with VS %d, compiler version %d (%s %s CET)" TP_ENDL TP_ENDL, _MSC_VER == 1800 ? 2013 : (_MSC_VER) == 1900 ? 2015 : 0, _MSC_VER, __DATE__, __TIME__);
#elif defined( __GNUC__)
#if defined( __MINGW32__ )
	_clibs_printf("MinGW C/C++ Compiler version: %d.%d.%d" TP_ENDL TP_ENDL, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
	_clibs_printf("GCC C/C++ Compiler version: %d.%d.%d" TP_ENDL TP_ENDL, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
#endif
}

tVoid tp_print_buffer(tPChar topic, tPChar buffer, tU16 size)
{
	_clibs_printf(TP_ENDL "%s", topic);

	if (size > 0)
	{
		for (int i = 0; i < size; i++)
		{
			_clibs_printf("%02x", (*(buffer + i) & 0xFF));
		}
	}

	_clibs_printf(TP_ENDL);
}
