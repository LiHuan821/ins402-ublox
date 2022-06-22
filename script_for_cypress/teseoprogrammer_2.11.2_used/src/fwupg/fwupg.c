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
#include "tp_report.h"

#include "crc32.h"

#include "fwupg/fwupg.h"

/*****************************************************************************
   external declarations
*****************************************************************************/

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define FWUPG_IDENTIFIER          0xBCD501F4U
#define FWUPG_SYNC                0x83984073U

#define FWUPG_START_COMM          0xA3U
#define FWUPG_FLASHER_READY       0x4AU
#define FWUPG_DEVICE_ACK          0xCCU

#define FWUPG_FLASHER_CHUNKSIZE   (16*1024)

#define DRIVER_STANDARD(x)   TP_PRINT(x)     
#define DRIVER_ERROR(x)      TP_ERROR(x)  
#define DRIVER_DEBUG(x)      TP_INFO(x)

#define FWUPG_WRITE(b,s)      fwupg_manager->io_mgr->dev_write_cb((tPChar)(b),s,fwupg_manager->instance)
#define FWUPG_READ(b,s,t)     fwupg_manager->io_mgr->dev_read_cb((tPChar)(b),s,t,fwupg_manager->instance)
#define FWUPG_GETTIME()       fwupg_manager->io_mgr->gettime()

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   global variable definitions  (scope: module-exported)
*****************************************************************************/

/*****************************************************************************
   global variable definitions (scope: module-local)
*****************************************************************************/

static const tChar fwupg_nmeacmd[] = "$PSTMFWUPGRADE\r\n";
static const tChar fwupg_nmeacmdok[] = "$PSTMFWUPGRADEOK";
static const tChar fwupg_nmeacmdko[] = "$PSTMFWUPGRADEERROR";
static const tChar fwupg_nmeaswver[] = "$PSTMGETSWVER\r\n";
static const tChar fwupg_nmeaswverans[] = "$PSTMVER";

/*****************************************************************************
   function prototypes (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   function implementations (scope: module-local)
*****************************************************************************/

static trgt_status_t fwupg_waitack(fwupg_manager_t * fwupg_manager, tUInt timeout)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tU8 data_in;
	tUInt read_chars = 0;
	tUInt LoopAbort = timeout / 100;

	while (LoopAbort > 0 && error == TRGT_STATUS_OK)
	{
		read_chars = FWUPG_READ(&data_in, sizeof(data_in), 100);

		if (0 != read_chars)
		{
			break;
		}
		
		LoopAbort--;

		IS_ABORT(error);
	}

	if (0 == read_chars)
	{
		error = TRGT_STATUS_ERROR;
	}
	else if (FWUPG_DEVICE_ACK != data_in)
	{
		error = TRGT_STATUS_ERROR;
	}

	return error;
}

static tSize fwupg_readline(fwupg_manager_t * fwupg_manager, tPChar out_buf, const tSize max_chars, const tUInt timeout)
{
	tUInt read_chars = 0;
	tChar in_char = '\0';
	tUInt in_len = 0;
	tBool endofline = FALSE;

	while (FALSE == endofline)
	{
		in_len = FWUPG_READ(&in_char, 1, timeout);

		if (0 == in_len)
		{
			/* Timeout occurred */
			out_buf[read_chars] = '\0';
			endofline = TRUE;
		}
		else if (('\r' == in_char) || ('\n' == in_char))
		{
			/* New line received */
			if (0 == read_chars)
			{
				/* Beginning of string, discard */
			}
			else
			{
				out_buf[read_chars] = '\0';
				endofline = TRUE;
			}
		}
		else if (read_chars < max_chars)
		{
			out_buf[read_chars++] = in_char;
		}
		else
		{
			out_buf[read_chars - 1] = '\0';
			endofline = TRUE;
		}
	}

	return read_chars;
}

/*****************************************************************************
   function implementations (scope: module-exported)
*****************************************************************************/

trgt_status_t fwupg_init(fwupg_instance_t  inst, fwupg_config_t *mgr, trgt_io_manager_t *io_mgr,fwupg_manager_t ** pfwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	fwupg_manager_t * fwupg_manager = _clibs_calloc(1, sizeof(fwupg_manager_t));
	*pfwupg_manager = fwupg_manager;

	fwupg_manager->recovery = mgr->recovery;

	fwupg_manager->io_mgr = io_mgr;
	fwupg_manager->instance = inst;

	fwupg_manager->io_mgr = io_mgr;

	/* Fill binimg info */
	fwupg_manager->image_info.dump = FALSE;
	fwupg_manager->image_info.baudRate = mgr->br_fwupg_idx;

	fwupg_manager->image_info.firmwareSize = io_mgr->src_size;
	fwupg_manager->image_info.firmwareCRC = io_mgr->src_crc32;

	fwupg_manager->image_info.programOnly = mgr->program_only;
	fwupg_manager->image_info.eraseNVM = mgr->nvm_erase;
	fwupg_manager->image_info.nvmAddressOffset = mgr->nvm_offset;
	fwupg_manager->image_info.nvmSize = mgr->nvm_size;

	DRIVER_DEBUG(("baudrate %d" TP_ENDL, fwupg_manager->image_info.baudRate));
	DRIVER_DEBUG(("firmwareSize 0x%x" TP_ENDL, fwupg_manager->image_info.firmwareSize));
	DRIVER_DEBUG(("firmwareCRC 0x%x" TP_ENDL, fwupg_manager->image_info.firmwareCRC));
	DRIVER_DEBUG(("programOnly %d" TP_ENDL, fwupg_manager->image_info.programOnly));
	DRIVER_DEBUG(("eraseNVMy %d" TP_ENDL, fwupg_manager->image_info.eraseNVM));
	DRIVER_DEBUG(("nvm offset %x" TP_ENDL, fwupg_manager->image_info.nvmAddressOffset));
	DRIVER_DEBUG(("nvm size %d" TP_ENDL, fwupg_manager->image_info.nvmSize));

	return error;
}

trgt_status_t fwupg_send_fwupgcmd(fwupg_manager_t * fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tChar nmea_ans_buf[256];
	tUInt nmea_ans_len = 0;
	tBool nmea_ans_found = FALSE;
	tUInt trials = 5;

	// Low speed cannot detect answer, detect activity than silence
	tUInt LowSpeedDetection = 0;

	TP_REPORT(fwupg_manager->instance,ITEM_FWUPDATECMD, STATE_RUN, 0);

	DRIVER_STANDARD(("Send FW Upgrade command" TP_ENDL));

	FWUPG_WRITE(fwupg_nmeacmd, sizeof(fwupg_nmeacmd));

	while ((FALSE == nmea_ans_found) && (0 < trials) && (error == TRGT_STATUS_OK))
	{
		nmea_ans_len = fwupg_readline(fwupg_manager,nmea_ans_buf, sizeof(nmea_ans_buf), 3000);

		if (nmea_ans_len > 0)
		{
			if (0 == _clibs_strncmp(fwupg_nmeacmdok, nmea_ans_buf, strlen(fwupg_nmeacmdok)))
			{
				/* Command answer is ok */
				DRIVER_DEBUG(("Command acknowledge" TP_ENDL));
				nmea_ans_found = TRUE;
			}
			else if (0 == _clibs_strncmp(fwupg_nmeacmdko, nmea_ans_buf, strlen(fwupg_nmeacmdko)))
			{
				/* Command answer is ko */
				DRIVER_ERROR(("Command not acknowledge!" TP_ENDL));
				nmea_ans_found = TRUE;
				error = TRGT_STATUS_ERROR;
			}
			else
			{
				// Low speed detection, NMEA and binary
				if ((nmea_ans_buf[0] == '$') || (nmea_ans_buf[0] == 0xAA))
				{
					LowSpeedDetection++;
					DRIVER_DEBUG(("Activity detection" TP_ENDL));
				}
			}
		}
		else
		{
			trials--;

			// Force next step for low baud rate
			if (LowSpeedDetection > 5 && trials == 0)
			{
				nmea_ans_found = TRUE;
			}

			TP_REPORT_PROGRESS(fwupg_manager->instance, ITEM_FWUPDATECMD, 20 * (5 - trials));
		}

		IS_ABORT(error);
	}

	if (FALSE == nmea_ans_found )
	{
		TP_REPORT_KO(fwupg_manager->instance, ITEM_FWUPDATECMD);
		error = TRGT_STATUS_ERROR;
	}
	else
	{
		TP_REPORT_OK(fwupg_manager->instance, ITEM_FWUPDATECMD);
	}

	return error;
}

trgt_status_t fwupg_sync(fwupg_manager_t * fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt fwupg_id;
	tUInt fwupg_sync;
	tUInt read_chars = 0;

	fwupg_id = FWUPG_IDENTIFIER;

	FWUPG_WRITE(&fwupg_id, sizeof(fwupg_id));

	read_chars = FWUPG_READ(&fwupg_sync, sizeof(fwupg_id), (fwupg_manager->recovery)? 1000:10);

	if (FWUPG_SYNC != fwupg_sync)
	{
		error = TRGT_STATUS_ERROR;
	}

	return error;
}

trgt_status_t fwupg_start_comm(fwupg_manager_t * fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tU8 data_out;

	data_out = FWUPG_START_COMM;

	FWUPG_WRITE(&data_out, sizeof(data_out));

	error = fwupg_waitack(fwupg_manager, 1000);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Communication not started!" TP_ENDL));
	}

	return error;
}

trgt_status_t fwupg_send_imageinfo(fwupg_manager_t * fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Sending image info." TP_ENDL));

	DRIVER_DEBUG(("eraseNVM %x" TP_ENDL, fwupg_manager->image_info.eraseNVM));
	DRIVER_DEBUG(("programOnly %x" TP_ENDL, fwupg_manager->image_info.programOnly));
	DRIVER_DEBUG(("dump %x" TP_ENDL, fwupg_manager->image_info.dump));
	DRIVER_DEBUG(("baudRate %x" TP_ENDL, fwupg_manager->image_info.baudRate));
	DRIVER_DEBUG(("firmwareSize %x" TP_ENDL, fwupg_manager->image_info.firmwareSize));
	DRIVER_DEBUG(("firmwareCRC %x" TP_ENDL, fwupg_manager->image_info.firmwareCRC));
	DRIVER_DEBUG(("nvmAddressOffset %x" TP_ENDL, fwupg_manager->image_info.nvmAddressOffset));
	DRIVER_DEBUG(("nvmSize %x" TP_ENDL, fwupg_manager->image_info.nvmSize));

	FWUPG_WRITE(&fwupg_manager->image_info, sizeof(fwupg_manager->image_info));

	return error;
}

trgt_status_t fwupg_wait_changebaudrate(fwupg_manager_t * fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tU8 data_out;
  tUInt trials = 50;

	data_out = FWUPG_FLASHER_READY;

	while (0 < trials && error == TRGT_STATUS_OK)
	{
		FWUPG_WRITE(&data_out, sizeof(data_out));

        error = fwupg_waitack(fwupg_manager, 500);
		if (TRGT_STATUS_OK == error)
		{
			break;
		}
		trials--;
		error = TRGT_STATUS_OK;
		IS_ABORT(error);
	}

	return error;
}

trgt_status_t fwupg_wait_devinit(fwupg_manager_t * fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	error = fwupg_waitack(fwupg_manager,1000);

	return error;
}

trgt_status_t fwupg_wait_eraseprog(fwupg_manager_t * fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Erasing program flash." TP_ENDL));

	TP_REPORT_START(fwupg_manager->instance, ITEM_FLASH_ERASE);

	error = fwupg_waitack(fwupg_manager, 60000);

	if (TRGT_STATUS_ERROR == error)
	{
		TP_REPORT_KO(fwupg_manager->instance, ITEM_FLASH_ERASE);
		DRIVER_ERROR(("Program flash erase failed!" TP_ENDL));
	}
	else
	{
		TP_REPORT_OK(fwupg_manager->instance, ITEM_FLASH_ERASE);
	}

	return error;
}

trgt_status_t fwupg_wait_erasenvm(fwupg_manager_t * fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Erasing NVM flash." TP_ENDL));

	TP_REPORT_START(fwupg_manager->instance, ITEM_NVM);

	error = fwupg_waitack(fwupg_manager,60000);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("NVM flash erase failed!" TP_ENDL));
		TP_REPORT_KO(fwupg_manager->instance, ITEM_NVM);
	}
	else
  {
		TP_REPORT_OK(fwupg_manager->instance, ITEM_NVM);
  }
	return error;
}

trgt_status_t fwupg_send_buffer(fwupg_manager_t * fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt chunk_size = 0;
	tPU8  src_ptr = (tPU8)fwupg_manager->io_mgr->src_buf;
	tUInt remaining_bytes = fwupg_manager->io_mgr->src_size;

	DRIVER_STANDARD(("Sending buffer." TP_ENDL));

	TP_REPORT_START(fwupg_manager->instance, ITEM_FLASH);

	while ((TRGT_STATUS_OK == error) && (0 < remaining_bytes) && (error == TRGT_STATUS_OK))
	{
		/* Check bytes to sent */
		if (FWUPG_FLASHER_CHUNKSIZE < remaining_bytes)
		{
			chunk_size = FWUPG_FLASHER_CHUNKSIZE;
			remaining_bytes -= FWUPG_FLASHER_CHUNKSIZE;
		}
		else
		{
			chunk_size = remaining_bytes;
			remaining_bytes = 0;
		}

		/* Send buffer */
		FWUPG_WRITE(src_ptr, chunk_size);

		TP_REPORT_PROGRESS(fwupg_manager->instance, ITEM_FLASH, (int)((float)(((float)100 / (float)fwupg_manager->io_mgr->src_size)) * (float)(fwupg_manager->io_mgr->src_size - remaining_bytes)));

		error = fwupg_waitack(fwupg_manager, 3500);

		src_ptr += chunk_size;

		IS_ABORT(error);
	}

	if (TRGT_STATUS_ERROR == error)
	{
		TP_REPORT_KO(fwupg_manager->instance, ITEM_FLASH);
		DRIVER_ERROR(("Buffer sending failed!" TP_ENDL));
	}
	else
	{
		TP_REPORT_OK(fwupg_manager->instance, ITEM_FLASH);
	}

	return error;
}

trgt_status_t fwupg_check_crc(fwupg_manager_t * fwupg_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Checking CRC." TP_ENDL));

	TP_REPORT_START(fwupg_manager->instance, ITEM_CRC);

	error = fwupg_waitack(fwupg_manager,25000);

	if (TRGT_STATUS_ERROR == error)
	{
		TP_REPORT_KO(fwupg_manager->instance, ITEM_CRC);
		DRIVER_ERROR(("CRC check failed!" TP_ENDL));
	}
	else
  {
		TP_REPORT_OK(fwupg_manager->instance, ITEM_CRC);
  }

	return error;
}

tVoid fwupg_shutdown(fwupg_manager_t * fwupg_manager)
{
}
