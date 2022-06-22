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
#include "tp_tools.h"
#include "tp_print.h"

#include <time.h>

/*****************************************************************************
external declarations
*****************************************************************************/

/*****************************************************************************
defines and macros (scope: module-local)
*****************************************************************************/

#define TP_TOOLS_INFO(x)      //TP_FATAL_ERROR(x)

/*****************************************************************************
typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
global variable definitions  (scope: module-exported)
*****************************************************************************/

/*****************************************************************************
global variable definitions (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function prototypes (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function implementations (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function implementations (scope: module-exported)
*****************************************************************************/

tVoid tp_tools_tolower(tPChar dest_str, const tPChar src_str)
{
	tUInt pos = 0;
	tUInt len = _clibs_strlen(src_str);

	while (pos < len)
	{
		dest_str[pos] = _clibs_tolower(src_str[pos]);
		pos++;
	}
	dest_str[pos] = '\0';
}

tVoid tp_tools_toupper(tPChar dest_str, const tPChar src_str)
{
	tUInt pos = 0;
	tUInt len = _clibs_strlen(src_str);

	while (pos < len)
	{
		dest_str[pos] = _clibs_toupper(src_str[pos]);
		pos++;
	}
	dest_str[pos] = '\0';
}

tVoid tp_tools_remove_endline(tPChar * line)
{
	tPChar pos;

	if ((pos = strchr(*line, '\n')) != NULL)
		*pos = '\0';
}

tPChar tp_tools_lcstr_new(const tPChar src_str)
{
	tPChar dest_str = NULL;
	tUInt pos = 0;
	tUInt len = _clibs_strlen(src_str);

	dest_str = (tPChar)_clibs_malloc(len + 1);
	if (NULL != dest_str)
	{
		tp_tools_tolower(dest_str, src_str);
	}

	TP_TOOLS_INFO(("arg lowercase string: %s" TP_ENDL, dest_str));

	return dest_str;
}

tVoid tp_tools_lcstr_delete(tPChar memptr)
{
	if (NULL != memptr)
	{
		_clibs_free(memptr);
	}
}

tp_status_t tp_tools_parse_valhex_valhex(const tPChar src_str, tUInt *value_ptr1, tUInt *value_ptr2)
{
	tp_status_t error = TP_STATUS_OK;

	tPChar test_str;
	tPChar src_str_second;

	/* look for second entry */
	src_str_second = _clibs_strchr(src_str, ',');

	if (src_str_second != NULL)
	{
		*src_str_second = 0;
	}
	else
	{
		*value_ptr2 = 0;
	}

	test_str = (tPChar)_clibs_calloc(_clibs_strlen(src_str) + 1, sizeof(tChar));

	if (NULL != test_str)
	{
		/* Extract first parameter */
		_clibs_sscanf(src_str, "0x%x", value_ptr1);
		_clibs_snprintf(test_str, strlen(src_str) + 1, "0x%x", *value_ptr1);

		/* Check if value is correct */
		if (TP_STATUS_OK == error)
		{
			if (0 != _clibs_strcmp(src_str, test_str))
			{
				error = TP_STATUS_ERROR;
			}
		}

		if (TP_STATUS_OK == error && src_str_second != NULL)
		{

			/* move on first caracter */
			src_str_second++;

			/* Extract second parameter */
			_clibs_sscanf(src_str_second, "0x%x", value_ptr2);
			_clibs_snprintf(test_str, strlen(src_str_second) + 1, "0x%x", *value_ptr2);

			/* Check if value is correct */
			if (TP_STATUS_OK == error)
			{
				if (0 != _clibs_strcmp(src_str_second, test_str))
				{
					error = TP_STATUS_ERROR;
				}
			}
		}
	}
	else
	{
		TP_FATAL_ERROR(("Not enough memory!" TP_ENDL));
		error = TP_STATUS_ERROR;
		return error;
	}

	_clibs_free(test_str);

	return error;
}

tp_status_t tp_tools_parse_val(const tPChar src_str, const tp_tools_parse_fmt_t src_format, tVoid *value_ptr)
{
	tp_status_t error = TP_STATUS_OK;

	if (TP_TOOLS_PARSE_FMT_BOOL == src_format)
	{
		if (0 == _clibs_strcmp(src_str, "true"))
		{
			*((tBool *)value_ptr) = TRUE;
		}
		else if (0 == _clibs_strcmp(src_str, "false"))
		{
			*((tBool *)value_ptr) = FALSE;
		}
		else if (0 == _clibs_strcmp(src_str, "3"))
		{
			// *((tU8 *)value_ptr) == 0;
			*((tU8 *)value_ptr) = 3;
		}
		else
		{
			error = TP_STATUS_ERROR;
		}
	}
	else
	{
		tUInt value_int;
		tPChar test_str;

		test_str = (tPChar)_clibs_calloc(_clibs_strlen(src_str) + 1, sizeof(tChar));

		if (NULL != test_str)
		{
			/* Select format string and value pointer */
			switch (src_format)
			{
			case TP_TOOLS_PARSE_FMT_UINT:
				_clibs_sscanf(src_str, "%d", &value_int);
				_clibs_snprintf(test_str, strlen(src_str) + 1, "%d", value_int);
				break;

			case TP_TOOLS_PARSE_FMT_HEX:
				_clibs_sscanf(src_str, "0x%x", &value_int);
				_clibs_snprintf(test_str, strlen(src_str) + 1, "0x%x", value_int);
				break;

			default:
				TP_FATAL_ERROR(("Parse format %d invalid" TP_ENDL, src_format));
				error = TP_STATUS_ERROR;
				break;
			}

			/* Check if value is correct */
			if (TP_STATUS_OK == error)
			{
				if (0 != _clibs_strcmp(src_str, test_str))
				{
					error = TP_STATUS_ERROR;
				}
			}
		}
		else
		{
			TP_FATAL_ERROR(("Not enough memory!" TP_ENDL));
			error = TP_STATUS_ERROR;
		}

		_clibs_free(test_str);

		/* Set the return value (if no error) */
		if (TP_STATUS_OK == error)
		{
			switch (src_format)
			{
			case TP_TOOLS_PARSE_FMT_UINT:
			case TP_TOOLS_PARSE_FMT_HEX:
				*((tPUInt)value_ptr) = value_int;
				break;

			default:
				break;
			}
		}
	}

	return error;
}

tp_tools_time_t tp_tools_gettime(tVoid)
{
	clock_t timenow = clock();

	return (tp_tools_time_t)(((double)timenow) / (((double)CLOCKS_PER_SEC) / 1000.0));
}

tVoid tp_tools_delay(const tInt ms)
{
	tp_tools_time_t start;

	start = tp_tools_gettime();

	while ((tp_tools_gettime() - start) < ms);
}

tUInt tp_tools_split(tPChar str, tPChar ** SplitStrings, tChar Separator)
{
	tUInt SplitCounter = 1;
	tPChar ResearchPointer = str;

	while (*ResearchPointer != '\0')
	{
		if (*ResearchPointer == Separator && (*(ResearchPointer + 1) != '\0'))
			SplitCounter++;

		ResearchPointer++;
	}

	tPChar dummystring = _clibs_calloc(strlen(str) + 1, sizeof(tChar));
	_clibs_strcpy(dummystring, str);
	*SplitStrings = _clibs_calloc(SplitCounter, sizeof(tPChar));

	SplitCounter = 0;
	tPChar SplitString = strtok(dummystring, ",");
	while (SplitString != NULL)
	{
		(*SplitStrings)[SplitCounter] = SplitString;
		SplitCounter++;
		SplitString = strtok(NULL, ",");
	}

	return SplitCounter;

}
