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
#include "tp_uart.h"
#include "tp_tools.h"
#include "tp_print.h"

#include <windows.h>
#include <time.h>

/*****************************************************************************
external declarations
*****************************************************************************/

/*****************************************************************************
defines and macros (scope: module-local)
*****************************************************************************/

#define UART_STANDARD(x)   TP_PRINT(x)     
#define UART_ERROR(x)      TP_ERROR(x)  
#define UART_DEBUG(x)      TP_INFO(x) 
#define UART_STREAM(x)     TP_STREAM(x)

#define TP_UART_FILENAME_PREFIX     "\\\\.\\"

#define TP_UART_BAUDRATE_TABLESIZE  5U

#define TP_UART_READBUFFERSIZE      32U

/*****************************************************************************
typedefs and structures (scope: module-local)
*****************************************************************************/

typedef struct tp_uart_manager_s
{
	tPChar    comname;
	HANDLE *  handle;
	tU32      buf_pos;
	tPChar    readbuffer;
} tp_uart_manager_t;

/*****************************************************************************
global variable definitions  (scope: module-exported)
*****************************************************************************/

/*****************************************************************************
global variable definitions (scope: module-local)
*****************************************************************************/

#ifdef MULTI_COMS
pthread_mutex_t mutex_uart = PTHREAD_MUTEX_INITIALIZER;
#endif

static const tUInt tp_uart_baudrate_table[TP_UART_BAUDRATE_TABLESIZE] =
{
	57600,
	115200,
	230400,
	460800,
	921600,
};

static tp_uart_manager_t * tp_uart_manager[TP_COM_INSTANCE_MAX];

/*****************************************************************************
function prototypes (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function implementations (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function implementations (scope: module-exported)
*****************************************************************************/

tp_status_t tp_uart_init(tp_prgdev_uart_config_t *uart_cfg, tp_prgdev_instance_t inst)
{
	tp_status_t error = TP_STATUS_OK;

	LOCK_UART

		if (inst >= TP_COM_INSTANCE_MAX)
		{
			UART_ERROR(("Too many port com" TP_ENDL));
			error = TP_STATUS_ERROR;
		}

	tp_uart_manager[inst] = (tp_uart_manager_t*)_clibs_calloc(TP_COM_INSTANCE_MAX, sizeof(tp_uart_manager_t));

	if (tp_uart_manager == NULL || tp_uart_manager[inst] == NULL)
	{
		UART_ERROR(("Not enough memory!" TP_ENDL));
		error = TP_STATUS_ERROR;
	}

	/* Create UART filename */
	if (TP_STATUS_OK == error)
	{
		tUInt filename_len = 1 + _clibs_strlen(TP_UART_FILENAME_PREFIX) + _clibs_strlen(uart_cfg->port[inst]);

		tp_uart_manager[inst]->comname = _clibs_malloc(filename_len);
		tp_uart_manager[inst]->readbuffer = _clibs_malloc(TP_UART_READBUFFERSIZE);

		if (NULL == tp_uart_manager[inst]->comname || tp_uart_manager[inst]->readbuffer == NULL)
		{
			UART_ERROR(("Not enough memory!" TP_ENDL));
			error = TP_STATUS_ERROR;
		}
		else
		{
			_clibs_strcpy(tp_uart_manager[inst]->comname, TP_UART_FILENAME_PREFIX);
			_clibs_strcpy(&tp_uart_manager[inst]->comname[_clibs_strlen(TP_UART_FILENAME_PREFIX)], uart_cfg->port[inst]);
			tp_uart_manager[inst]->comname[filename_len - 1] = '\0';
		}
	}

	/* Try opening UART COM to report any error */
	if (TP_STATUS_OK == error)
	{
		tp_uart_manager[inst]->handle = CreateFileA(
			tp_uart_manager[inst]->comname,
			GENERIC_READ | GENERIC_WRITE,
			0,                          /* no share  */
			NULL,                       /* no security */
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,      /* no threads */
			NULL                        /* no templates */
		);

		if (INVALID_HANDLE_VALUE == tp_uart_manager[inst]->handle)
		{
			UART_ERROR(("Uart %s not found, instance = %d" TP_ENDL, uart_cfg->port[inst], inst));
			error = TP_STATUS_ERROR;
		}
		else
		{
			UART_DEBUG(("%s opened, instance = %d" TP_ENDL, uart_cfg->port[inst], inst));
		}
	}

	/* Configure UART COM */
	if (TP_STATUS_OK == error)
	{
		tChar         mode_str[128];
		tUInt         index = 0;
		DCB           port_settings;
		COMMTIMEOUTS  port_timeouts;

		_clibs_memset(&port_settings, 0, sizeof(port_settings));  /* clear the new struct  */
		port_settings.DCBlength = sizeof(port_settings);

		index = _clibs_sprintf(mode_str, "baud=%d", uart_cfg->def_br);
		index += _clibs_sprintf(&mode_str[index], " data=8 parity=n stop=1 dtr=off rts=off");

		if (!BuildCommDCBA(mode_str, &port_settings))
		{
			UART_ERROR(("Cannot build Uart config!" TP_ENDL));
			error = TP_STATUS_ERROR;
		}
		else
		{
			UART_DEBUG(("Uart config built, instance = %d" TP_ENDL, inst));

			if (!SetCommState(tp_uart_manager[inst]->handle, &port_settings))
			{
				UART_ERROR(("Cannot set Uart config, error %d!" TP_ENDL, GetLastError()));
				error = TP_STATUS_ERROR;
			}
			else
			{

				UART_DEBUG(("Uart config set, instance = %d" TP_ENDL, inst));

				port_timeouts.ReadIntervalTimeout = MAXDWORD;
				port_timeouts.ReadTotalTimeoutMultiplier = 0;
				port_timeouts.ReadTotalTimeoutConstant = 1000;
				port_timeouts.WriteTotalTimeoutMultiplier = 0;
				port_timeouts.WriteTotalTimeoutConstant = 0;

				if (!SetCommTimeouts(tp_uart_manager[inst]->handle, &port_timeouts))
				{
					UART_ERROR(("Cannot set Uart timouts config, instance = %d" TP_ENDL, inst));
					error = TP_STATUS_ERROR;
				}
				else
				{
					UART_DEBUG(("Uart timeouts set, instance = %d" TP_ENDL, inst));
				}
			}
		}
	}

	tp_uart_close(inst);

	UNLOCK_UART

		return error;
}

tp_status_t tp_uart_open(tp_prgdev_instance_t inst)
{
	tp_status_t error = TP_STATUS_OK;

	LOCK_UART

		if (NULL == tp_uart_manager[inst]->handle)
		{
			tp_uart_manager[inst]->handle = CreateFileA(
				tp_uart_manager[inst]->comname,
				GENERIC_READ | GENERIC_WRITE,
				0,                          /* no share  */
				NULL,                       /* no security */
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,      /* no threads */
				NULL                        /* no templates */
			);

			if (INVALID_HANDLE_VALUE == tp_uart_manager[inst]->handle)
			{
				UART_ERROR(("Uart %s not found!" TP_ENDL, tp_uart_manager[inst]->comname));
				error = TP_STATUS_ERROR;
			}
			else
			{
				UART_DEBUG(("%s opened, instance = %d" TP_ENDL, tp_uart_manager[inst]->comname, inst));
			}
		}
		else
		{
			UART_ERROR(("Uart instance %d not found!" TP_ENDL, inst));
		}

	UNLOCK_UART

		return error;
}

tp_status_t tp_uart_close(tp_prgdev_instance_t inst)
{
	tp_status_t error = TP_STATUS_OK;

	if (tp_uart_manager[inst] == NULL)
		return error;

	if (INVALID_HANDLE_VALUE != tp_uart_manager[inst]->handle)
	{
		CloseHandle(tp_uart_manager[inst]->handle);
		UART_DEBUG(("Close uart, instance = %d" TP_ENDL, inst));
	}
	else
	{
		UART_ERROR(("Close uart failure, instance = %d" TP_ENDL, inst));
	}

	tp_uart_manager[inst]->handle = NULL;

	return error;
}

tp_status_t tp_uart_flowcontrol(boolean rts, boolean dtr, tUInt timing, tp_prgdev_instance_t inst)
{
	tp_status_t error = TP_STATUS_OK;

	LOCK_UART
		DCB port_settings = { 0 };

	UART_DEBUG(("tp_uart_flowcontrol rts %d dtr %d duration %d,instance =  %d" TP_ENDL, rts, dtr, timing, inst));

	port_settings.DCBlength = sizeof(port_settings);
	if (!GetCommState(tp_uart_manager[inst]->handle, &port_settings))
	{
		UART_ERROR(("Cannot get Uart config, instance = %d" TP_ENDL, inst));
		error = TP_STATUS_ERROR;
	}

	UART_DEBUG(("TOGGLE pins" TP_ENDL));

	if (rts)
	{
		port_settings.fRtsControl = RTS_CONTROL_ENABLE;
	}
	if (dtr)
	{
		port_settings.fDtrControl = DTR_CONTROL_ENABLE;
	}


	if (!SetCommState(tp_uart_manager[inst]->handle, &port_settings))
	{
		UART_ERROR(("Cannot set Uart config!" TP_ENDL));
		error = TP_STATUS_ERROR;
		UNLOCK_UART
			return error;
	}

	UART_DEBUG(("delay %d" TP_ENDL, timing));

	if ((rts || dtr) && timing > 0)
		tp_tools_delay(timing);

	UART_DEBUG(("TOGGLE pins" TP_ENDL));

	if (rts)
	{
		port_settings.fRtsControl = RTS_CONTROL_DISABLE;
	}
	if (dtr)
	{
		port_settings.fDtrControl = DTR_CONTROL_DISABLE;
	}

	if (!SetCommState(tp_uart_manager[inst]->handle, &port_settings))
	{
		UART_ERROR(("Cannot set Uart config, instance %d!" TP_ENDL, inst));
		error = TP_STATUS_ERROR;
	}

	UNLOCK_UART

		return error;
}

tSize tp_uart_read(tPChar out_buf, const tSize chars, const tUInt timeout, tp_prgdev_instance_t inst)
{
	tUInt read_chars = 0U;
	tUInt chars_to_read = 0U;

	COMMTIMEOUTS uart_timeouts = { 0 };

	/* Get chars from buffer */
	if (tp_uart_manager[inst]->buf_pos >= chars)
	{
		/* The read buffer already contains enough chars */
		memcpy(out_buf, tp_uart_manager[inst]->readbuffer, chars);
		tp_uart_manager[inst]->buf_pos -= chars;
		return chars;
	}
	else
	{
		/* Copy only available chars */
		chars_to_read = chars - tp_uart_manager[inst]->buf_pos;

		/* Prepare UART */
		uart_timeouts.ReadTotalTimeoutConstant = timeout;
		SetCommTimeouts(tp_uart_manager[inst]->handle, &uart_timeouts);

		ReadFile(tp_uart_manager[inst]->handle, &tp_uart_manager[inst]->readbuffer[tp_uart_manager[inst]->buf_pos], chars_to_read, (LPDWORD)(&read_chars), NULL);

		if (read_chars != 0)
			UART_STREAM(("Read %d chars from %s instance = %d" TP_ENDL, read_chars, tp_uart_manager[inst]->comname, inst));

		if (0U < read_chars)
		{
			if (read_chars == chars_to_read)
			{
				/* All wanted chars has been read. Reset read buffer */
				memcpy(out_buf, tp_uart_manager[inst]->readbuffer, chars);
				tp_uart_manager[inst]->buf_pos = 0U;
			}
			else
			{
				/* Add chars to read buffer for next read */
				tp_uart_manager[inst]->buf_pos += read_chars;
			}
		}
		else
		{
			/* No more chars read, keep read buffer */
		}

		return read_chars;
	}

}

tSize tp_uart_write(const tPChar in_buf, const tSize chars, tp_prgdev_instance_t inst)
{
	tInt written_chars = 0;
	DWORD last_error;
	BOOL write_status;


	if (tp_uart_manager[inst]->handle == NULL)
	{
		UART_ERROR(("Handle is not valid, instance = %d" TP_ENDL, inst));
		return 0;
	}

	write_status = WriteFile(tp_uart_manager[inst]->handle, in_buf, chars, (LPDWORD)(&written_chars), NULL);

	do
	{
		last_error = GetLastError();
	} while (ERROR_IO_PENDING == last_error);

	if (!write_status)
	{
		written_chars = 0;
	}

	UART_STREAM(("Written %d chars to %s, instance = %d" TP_ENDL, written_chars, tp_uart_manager[inst]->comname, inst));

	return written_chars;
}

tBool tp_uart_getbaudrateidx(const tUInt baudrate, tPU8 idx_ptr)
{
	tBool valid = FALSE;
	tUInt cnt = 0;

	while ((TP_UART_BAUDRATE_TABLESIZE > cnt) && (FALSE == valid))
	{
		if (baudrate == tp_uart_baudrate_table[cnt])
		{
			if (NULL != idx_ptr)
			{
				*idx_ptr = cnt;
			}
			valid = TRUE;
		}
		else
		{
			cnt++;
		}
	}

	UART_DEBUG(("Baud rate index translation %d -> %d" TP_ENDL, baudrate, *idx_ptr));

	return valid;
}

tp_status_t tp_uart_setbaudrate(const tUInt baudrate, tp_prgdev_instance_t inst)
{
	tp_status_t error = TP_STATUS_OK;
	DCB port_settings = { 0 };

	UART_DEBUG(("tp_uart_setbaudrate %d, instance = %d" TP_ENDL, baudrate, inst));

	port_settings.DCBlength = sizeof(port_settings);
	if (!GetCommState(tp_uart_manager[inst]->handle, &port_settings))
	{
		UART_ERROR(("Cannot get Uart config, instance = %d" TP_ENDL, inst));
		error = TP_STATUS_ERROR;
	}

	port_settings.BaudRate = baudrate;

	if (!SetCommState(tp_uart_manager[inst]->handle, &port_settings))
	{
		UART_ERROR(("Cannot set Uart config, instance = %d" TP_ENDL, inst));
		error = TP_STATUS_ERROR;
	}

	return error;
}

tp_status_t tp_uart_flush(tp_prgdev_instance_t inst)
{
	tp_status_t error = TP_STATUS_OK;

	UART_DEBUG(("tp_uart_flush, instance = %d" TP_ENDL, inst));

	PurgeComm(tp_uart_manager[inst]->handle, PURGE_RXCLEAR | PURGE_RXABORT);
	PurgeComm(tp_uart_manager[inst]->handle, PURGE_TXCLEAR | PURGE_TXABORT);

	return error;
}
