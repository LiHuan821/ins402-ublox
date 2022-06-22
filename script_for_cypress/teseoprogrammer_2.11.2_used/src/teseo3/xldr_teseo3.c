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

#include "teseo3/xldr_teseo3.h"
#include "teseo3/xldr_teseo3_bootloader.h"


/*****************************************************************************
   external declarations
*****************************************************************************/

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define TESEO3_FLASHER_IDENTIFIER           0x215D1A40U
#define TESEO3_FLASHER_SYNC                 0x83984073U

#define TESEO3_OUTPUT_SRAM                  0U
#define TESEO3_OUTPUT_SQI                   1U
#define TESEO3_OUTPUT_NOR                   2U
#define TESEO3_OUTPUT_ITCM                  3U

#define TESEO3_DEVICE_NOT_FLASHED           0x0BU
#define TESEO3_DEVICE_FLASHED               0x0AU
#define TESEO3_DEVICE_DUMPED                0x0BU
#define TESEO3_DEVICE_READY                 0xC7U
#define TESEO3_DEVICE_START_COMMUNICATION   0xA3U
#define TESEO3_FLASHER_READY                0x4AU
#define TESEO3_CHANGE_BAUD_RATE             0x71U
#define TESEO3_BAUD_RATE_CHANGED            0x38U
#define TESEO3_HOST_READY                   0x5AU
#define TESEO3_HOST_NOT_READY               0x5BU
#define TESEO3_DEVICE_ACK                   0xCCU
#define TESEO3_DEVICE_NACK                  0xE6U

#define TESEO3_DEBUG_DUMP                   0x0U
#define TESEO3_DEBUG_SET                    0x0U

#define TESEO3_TP_DEF_ENTRY_POINT           0x00000000U
#define TESEO3_TP_DEF_DEST_ADDR             0x00000000U
#define TESEO3_TP_ID_MSP                    0xBCD501F4U
#define TESEO3_TP_ID_LSP                    0x83984073U
#define TESEO3_TP_ID_SIZE                   0x8U

#define TESEO3_WAIT_READ_DRIVER  			100
#define TESEO3_WAIT_SHORT_ACK			    1000
#define TESEO3_WAIT_ACK						3500
#define TESEO3_WAIT_ERASING					60000
#define TESEO3_WAIT_CRC                     25000
#define TESEO3_WAIT_DUMP_CHUNK              3500

#define TESEO3_FLASHER_CHUNKSIZE            (5*1024)
#define UART_MAX_REC_BUFFER					        32

#ifdef _STREAM_DUMP
#define DRIVER_STREAM(x)        TP_STREAM(x)
#else
#define DRIVER_STREAM(x)        
#endif
#define DRIVER_STANDARD(x)   TP_PRINT(x)     
#define DRIVER_ERROR(x)      TP_ERROR(x)  
#define DRIVER_DEBUG(x)      TP_INFO(x)

#define XLDR_WRITE(b,s)     xldr_teseo3_write(xldr_teseo3_manager,(tPChar)(b),s) 
#define XLDR_READ(b,s,t)    xldr_teseo3_read(xldr_teseo3_manager,(tPChar)(b),s,t)

#define XLDR_GETTIME()      xldr_teseo3_manager->io_mgr->gettime()

#define CITATS 

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

CITATS tUInt xldr_teseo3_read(xldr_teseo3_manager_t * xldr_teseo3_manager,tPChar buf, int size, int timeout)
{
	tUInt nb = xldr_teseo3_manager->io_mgr->dev_read_cb((tPChar)(buf), size, timeout, xldr_teseo3_manager->instance);

	DRIVER_STREAM((TP_ENDL "< [%d] ", nb));

	for (tUInt i = 0; i < nb; i++)
	{
		DRIVER_STREAM((" %02x", *(((tPChar)(buf)+i)) & 0xFF));
	}

	return nb;
}


CITATS void xldr_teseo3_write(xldr_teseo3_manager_t * xldr_teseo3_manager, tPChar buf, int size)
{
	DRIVER_STREAM((TP_ENDL "> [%d] ", size));

	for (int idx = 0; idx < size; idx++)
		DRIVER_STREAM(("%02x ", *((tPChar)(buf)+idx) & 0xFF));
	
	xldr_teseo3_manager->io_mgr->dev_write_cb((tPChar)(buf), size,xldr_teseo3_manager->instance);

	DRIVER_STREAM(("" TP_ENDL));
}

CITATS trgt_status_t xldr_teseo3_waitack(xldr_teseo3_manager_t * xldr_teseo3_manager, tUInt timeout)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tU8 data_in;
	tUInt read_chars = 0;
	tUInt LoopAbort = timeout / TESEO3_WAIT_READ_DRIVER;

	while (LoopAbort > 0 && error == TRGT_STATUS_OK)
	{
		read_chars = XLDR_READ(&data_in, sizeof(data_in), TESEO3_WAIT_READ_DRIVER);

		if (0 != read_chars)
		{
			break;
		}

		IS_ABORT(error);

		LoopAbort--;
	}

	if (0 == read_chars)
	{
		error = TRGT_STATUS_ERROR;
	}
	else if (TESEO3_DEVICE_ACK != data_in)
	{
		error = TRGT_STATUS_ERROR;
	}

	return error;
}

/*****************************************************************************
   function implementations (scope: module-exported)
*****************************************************************************/

trgt_status_t xldr_teseo3_init(trgt_command_t cmd, xldr_teseo3_inst inst, xldr_teseo3_config_t *trgt_cfg, trgt_io_manager_t *io_mgr, xldr_teseo3_manager_t ** pxldr)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt crc32;

	xldr_teseo3_manager_t * xldr_teseo3_manager = _clibs_calloc(1, sizeof(xldr_teseo3_manager_t));
	*pxldr = xldr_teseo3_manager;

	xldr_teseo3_manager->io_mgr = io_mgr;
	xldr_teseo3_manager->instance = inst;

	/* Fill binimg info */
	xldr_teseo3_manager->binimg_info.fileSize = xldr_teseo3_manager->io_mgr->src_size;
	xldr_teseo3_manager->binimg_info.crc32 = xldr_teseo3_manager->io_mgr->src_crc32;

	switch (trgt_cfg->mem)
	{
	case TRGT_MEM_SQI:
		xldr_teseo3_manager->binimg_info.bootMode = TESEO3_OUTPUT_SQI;
		break;

	case TRGT_MEM_NOR:
		xldr_teseo3_manager->binimg_info.bootMode = TESEO3_OUTPUT_NOR;
		break;

	case TRGT_MEM_RAM:
		xldr_teseo3_manager->binimg_info.bootMode = TESEO3_OUTPUT_SRAM;
		break;

	default:
		break;
	}

	xldr_teseo3_manager->binimg_info.destinationAddress = trgt_cfg->destaddr;
	xldr_teseo3_manager->binimg_info.entryPoint = trgt_cfg->entry;
	xldr_teseo3_manager->binimg_info.eraseNVM = trgt_cfg->nvm_erase;
	xldr_teseo3_manager->binimg_info.programOnly = trgt_cfg->program_only;
	xldr_teseo3_manager->binimg_info.subSector = trgt_cfg->subsector;
	DRIVER_DEBUG((" --> subSector = %x " TP_ENDL, xldr_teseo3_manager->binimg_info.subSector));
	xldr_teseo3_manager->binimg_info.sta8090fg = (TRGT_VARIANT_STA8090FG == trgt_cfg->variant) ? TRUE : FALSE;
	DRIVER_DEBUG((" --> sta8090fg = %x " TP_ENDL, xldr_teseo3_manager->binimg_info.sta8090fg));

	if (trgt_cfg->desterasesize != 0)
	{
		// tSize RoundSize = xldr_teseo3_manager->io_mgr->src_size & ~0xFFFF;
		xldr_teseo3_manager->binimg_info.nvmEraseSize = trgt_cfg->desterasesize - xldr_teseo3_manager->io_mgr->src_size;
		xldr_teseo3_manager->binimg_info.nvmOffset = xldr_teseo3_manager->io_mgr->src_size;
		xldr_teseo3_manager->binimg_info.eraseNVM = TRUE;
		trgt_cfg->nvm_erase = TRUE;
		DRIVER_DEBUG((" --> Extra erase after code area from 0x%x size %d bytes " TP_ENDL, xldr_teseo3_manager->binimg_info.nvmOffset, xldr_teseo3_manager->binimg_info.nvmEraseSize));
	}
	else
	{
		xldr_teseo3_manager->binimg_info.nvmEraseSize = trgt_cfg->nvm_size;
		xldr_teseo3_manager->binimg_info.nvmOffset = trgt_cfg->nvm_offset; 
	}

	/* No switch on command line yet */
	xldr_teseo3_manager->binimg_info.eraseOnly = (TRGT_CMD_ERASE == cmd) ? TRUE : FALSE;

	xldr_teseo3_manager->binimg_info.debug = trgt_cfg->debug;
	xldr_teseo3_manager->binimg_info.debugAction = trgt_cfg->debugAction;
	xldr_teseo3_manager->binimg_info.debugAddress = trgt_cfg->debugAddress;
	xldr_teseo3_manager->binimg_info.debugSize = trgt_cfg->debugSize;
	xldr_teseo3_manager->binimg_info.debugData = trgt_cfg->debugData;

	/* Prepare preamble */
	xldr_teseo3_manager->preamble.destinationAddress = TESEO3_TP_DEF_DEST_ADDR;
	xldr_teseo3_manager->preamble.identifier_msp = TESEO3_TP_ID_MSP;
	xldr_teseo3_manager->preamble.identifier_lsp = TESEO3_TP_ID_LSP;
	xldr_teseo3_manager->preamble.options = (0xFFU << 16) | (1U << 8) | (2U << 1);

	/* Prepare bootloader */
	xldr_teseo3_manager->bootloader.buf_ptr = (tPChar)&xldr_teseo3_bootloader;
	xldr_teseo3_manager->bootloader.size = xldr_teseo3_bootloader_len;
	xldr_teseo3_manager->bootloader.entry = 0;

	crc32 = 0;

	/* Calculate CRC32 on bootloader size */
	CRC32_EVAL_INT(crc32, crc32, xldr_teseo3_manager->bootloader.size);
	/* Calculate CRC32 using the *new* "entry point" */
	CRC32_EVAL_INT(crc32, crc32, xldr_teseo3_manager->bootloader.entry);
	/* Calculate CRC32 on bootloader code */
	CRC32_EVAL_BUF(crc32, crc32, xldr_teseo3_manager->bootloader.buf_ptr, xldr_teseo3_manager->bootloader.size);

	xldr_teseo3_manager->bootloader.crc32 = crc32;

	DRIVER_DEBUG(("Binary Image Info" TP_ENDL));
	DRIVER_DEBUG(("size                : %d" TP_ENDL, xldr_teseo3_manager->binimg_info.fileSize));
	DRIVER_DEBUG(("crc32               : 0x%x" TP_ENDL, xldr_teseo3_manager->binimg_info.crc32));
	DRIVER_DEBUG(("memory              : %d" TP_ENDL, xldr_teseo3_manager->binimg_info.bootMode));
	DRIVER_DEBUG(("destination address : 0x%X" TP_ENDL, xldr_teseo3_manager->binimg_info.destinationAddress));
	DRIVER_DEBUG(("entry point         : 0x%x" TP_ENDL, xldr_teseo3_manager->binimg_info.entryPoint));
	DRIVER_DEBUG(("erase NVM           : %s" TP_ENDL, xldr_teseo3_manager->binimg_info.eraseNVM ? "true" : "false"));
	DRIVER_DEBUG(("program only        : %s" TP_ENDL, xldr_teseo3_manager->binimg_info.programOnly ? "true" : "false"));
	DRIVER_DEBUG(("sub sector          : %s" TP_ENDL, xldr_teseo3_manager->binimg_info.subSector ? "true" : "false"));
	DRIVER_DEBUG(("variant             : %d" TP_ENDL, xldr_teseo3_manager->binimg_info.sta8090fg));
	DRIVER_DEBUG(("NVM size            : 0x%x" TP_ENDL, xldr_teseo3_manager->binimg_info.nvmEraseSize));
	DRIVER_DEBUG(("erase only          : %s" TP_ENDL, xldr_teseo3_manager->binimg_info.eraseOnly ? "true" : "false"));
	DRIVER_DEBUG(("nvm offset          : 0x%x" TP_ENDL, xldr_teseo3_manager->binimg_info.nvmOffset));
	DRIVER_DEBUG(("debug enable        : %s" TP_ENDL, xldr_teseo3_manager->binimg_info.debug ? "true" : "false"));
	DRIVER_DEBUG(("debug action        : %s" TP_ENDL, xldr_teseo3_manager->binimg_info.debugAction ? "dump" : "set"));
	DRIVER_DEBUG(("debug address       : 0x%x" TP_ENDL, xldr_teseo3_manager->binimg_info.debugAddress));
	DRIVER_DEBUG(("debug data          : 0x%x" TP_ENDL, xldr_teseo3_manager->binimg_info.debugData));
	DRIVER_DEBUG(("Bootloader" TP_ENDL));
	DRIVER_DEBUG(("Size                : %d" TP_ENDL, xldr_teseo3_manager->bootloader.size = xldr_teseo3_bootloader_len));
	DRIVER_DEBUG(("Entry               : %x" TP_ENDL, xldr_teseo3_manager->bootloader.entry));

	return error;
}

trgt_status_t xldr_teseo3_sync(xldr_teseo3_manager_t * xldr_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt t3id;
	tUInt t3sync;
	tUInt read_chars = 0;

	t3id = TESEO3_FLASHER_IDENTIFIER;

	/* Send data */
	XLDR_WRITE(&t3id, sizeof(t3id));

	/* Receive data */
	read_chars = XLDR_READ(&t3sync, sizeof(t3sync), TESEO3_WAIT_READ_DRIVER); /* @@@ (replaced 1000 by 100 ms) */

	if (TESEO3_FLASHER_SYNC != t3sync)
	{
		error = TRGT_STATUS_ERROR;
	}

	return error;
}

trgt_status_t xldr_teseo3_send_baudrate(xldr_teseo3_manager_t * xldr_teseo3_manager, tUInt baud_rate)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tU8 data_out;

	DRIVER_STANDARD(("Sending baud rate change command" TP_ENDL));

	/* Send baud rate command, baud rate and changed byte */
	data_out = TESEO3_CHANGE_BAUD_RATE;
	XLDR_WRITE(&data_out, sizeof(data_out));

	if (TRGT_STATUS_OK == error)
	{
		DRIVER_STANDARD(("Sending baud rate %d" TP_ENDL, baud_rate));

		XLDR_WRITE(&baud_rate, sizeof(baud_rate));

		error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_ACK);
	}

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Baud rate sending failed!" TP_ENDL));
	}

	return error;
}

trgt_status_t xldr_teseo3_check_baudrate(xldr_teseo3_manager_t * xldr_teseo3_manager )
{
	trgt_status_t error = TRGT_STATUS_ERROR;
	tU8 data_in, data_out;
	tUInt read_chars = 0;

	data_out = TESEO3_BAUD_RATE_CHANGED;
	XLDR_WRITE(&data_out, sizeof(data_out));
	read_chars = XLDR_READ(&data_in, sizeof(data_in), TESEO3_WAIT_READ_DRIVER);

	if (0 < read_chars)
	{
		if (TESEO3_DEVICE_ACK == data_in)
		{
			error = TRGT_STATUS_OK;
		}
	}

	return error;
}

trgt_status_t xldr_teseo3_check_hostready(xldr_teseo3_manager_t * xldr_teseo3_manager )
{
	trgt_status_t error = TRGT_STATUS_OK;
	tU8 data_out;

	data_out = TESEO3_HOST_READY;
	XLDR_WRITE(&data_out, sizeof(data_out));

	error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_SHORT_ACK);

	return error;
}

trgt_status_t xldr_teseo3_send_preamble(xldr_teseo3_manager_t * xldr_teseo3_manager )
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Sending preamble." TP_ENDL));
	XLDR_WRITE(&xldr_teseo3_manager->preamble, sizeof(xldr_teseo3_preamble_t));

	return error;
}

trgt_status_t xldr_teseo3_send_bootloader(xldr_teseo3_manager_t * xldr_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tU8 data_out;
	tUInt chunk_size = 0;
	tPU8  src_ptr = (tPU8)xldr_teseo3_manager->bootloader.buf_ptr;
	tUInt remaining_bytes = xldr_teseo3_manager->bootloader.size;

	DRIVER_STANDARD(("Sending bootloader." TP_ENDL));

	/* Write the bootstrap CRC32 */
	XLDR_WRITE(&xldr_teseo3_manager->bootloader.crc32, sizeof(xldr_teseo3_manager->bootloader.crc32));
	/* Write the bootstrap Size */
	XLDR_WRITE(&xldr_teseo3_manager->bootloader.size, sizeof(xldr_teseo3_manager->bootloader.size));
	/* Write the bootstrap Entry Point */
	XLDR_WRITE(&xldr_teseo3_manager->bootloader.entry, sizeof(xldr_teseo3_manager->bootloader.entry));

	/* Write bootloader binary image */
	while ((TRGT_STATUS_OK == error) && (0 < remaining_bytes))
	{
		/* Check bytes to sent */
		if (TESEO3_FLASHER_CHUNKSIZE < remaining_bytes)
		{
			chunk_size = TESEO3_FLASHER_CHUNKSIZE;
			remaining_bytes -= TESEO3_FLASHER_CHUNKSIZE;
		}
		else
		{
			chunk_size = remaining_bytes;
			remaining_bytes = 0;
		}

		/* Send buffer */
		XLDR_WRITE(src_ptr, chunk_size);

		src_ptr += chunk_size;

		IS_ABORT(error);
	}

	error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_ACK);

	IS_ABORT(error);

	if (TRGT_STATUS_OK == error)
	{
		DRIVER_STANDARD(("Waiting flasher." TP_ENDL));

		data_out = TESEO3_FLASHER_READY;
		XLDR_WRITE(&data_out, sizeof(data_out));

		error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_ACK);
	}

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Flasher not ready!" TP_ENDL));
	}

	return error;
}

trgt_status_t xldr_teseo3_send_bininfo(xldr_teseo3_manager_t * xldr_teseo3_manager,const tBool waitForAck)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt read_chars = 0;

	DRIVER_STANDARD(("Sending binary info." TP_ENDL));

	XLDR_WRITE(&xldr_teseo3_manager->binimg_info, sizeof(xldr_teseo3_manager->binimg_info));

	IS_ABORT(error);

	if (TRUE == waitForAck)
	{
		error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_SHORT_ACK);
	}

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Binary info sending failed!" TP_ENDL));
	}

	return error;
}

trgt_status_t xldr_teseo3_wait_devinit(xldr_teseo3_manager_t * xldr_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Initializing bootloader." TP_ENDL));

	error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_SHORT_ACK);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Bootloader initialization failed!" TP_ENDL));
	}

	return error;
}

trgt_status_t xldr_teseo3_wait_eraseprog(xldr_teseo3_manager_t * xldr_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Erasing program flash." TP_ENDL));

	error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_ERASING);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Program flash erase failed!" TP_ENDL));
	}

	return error;
}

trgt_status_t xldr_teseo3_wait_erasenvm(xldr_teseo3_manager_t * xldr_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Erasing NVM flash." TP_ENDL));

	TP_REPORT_START(xldr_teseo3_manager->instance,ITEM_NVM);

	error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_ERASING);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("NVM flash erase failed!" TP_ENDL));
		TP_REPORT_KO(xldr_teseo3_manager->instance, ITEM_NVM);
	}
	else
	{
		TP_REPORT_OK(xldr_teseo3_manager->instance, ITEM_NVM);
	}

	return error;
}

trgt_status_t xldr_teseo3_dump(xldr_teseo3_manager_t * xldr_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt chunk_size = 0;
	tU8  data_out = TESEO3_DEVICE_ACK;
	tU8  host_ready = TESEO3_FLASHER_READY;
	tUInt remaining_bytes = xldr_teseo3_manager->binimg_info.debugSize * 4;

	/// ADD ON
	XLDR_WRITE(&host_ready, 1);

	/* Create a buffer to read memory */
	xldr_teseo3_manager->io_mgr->out_buf = (tPChar)_clibs_malloc(UART_MAX_REC_BUFFER);

	if (NULL == xldr_teseo3_manager->io_mgr->out_buf)
	{
		TP_ERROR(("Not enough memory!" TP_ENDL));
		error = TP_STATUS_ERROR;
	}

	if (NULL == xldr_teseo3_manager->io_mgr->out_file)
	{
		TP_ERROR(("File handle NULL" TP_ENDL));
		error = TP_STATUS_ERROR;
	}
	
	while ((TRGT_STATUS_OK == error) && (0 < remaining_bytes))
	{
		/* Check bytes to sent */
		if ((UART_MAX_REC_BUFFER ) < remaining_bytes)
		{
			chunk_size = UART_MAX_REC_BUFFER;
			remaining_bytes -= UART_MAX_REC_BUFFER;
		}
		else
		{
			chunk_size = remaining_bytes;
			remaining_bytes = 0;
		}


		/* read buffer */
		tUInt ReceivedBytes = XLDR_READ(xldr_teseo3_manager->io_mgr->out_buf, chunk_size, TESEO3_WAIT_DUMP_CHUNK);

		if ( ReceivedBytes != chunk_size)
		{
			error = TRGT_STATUS_ERROR;

			DRIVER_ERROR(("UART reception time out [%d] " TP_ENDL, ReceivedBytes));
		}
		else
		{
			/* Copy buffer to file */
			if (fwrite((const void *)xldr_teseo3_manager->io_mgr->out_buf, 1, chunk_size, xldr_teseo3_manager->io_mgr->out_file) != chunk_size)
			{
				error = TRGT_STATUS_ERROR;

				DRIVER_ERROR(("Writing error in dump file : " TP_ENDL));
			}

		}

		TP_REPORT_PROGRESS(xldr_teseo3_manager->instance, ITEM_DUMP, (int)((float)(((float)100 / (float)xldr_teseo3_manager->binimg_info.debugSize)) * (float)(xldr_teseo3_manager->binimg_info.debugSize - remaining_bytes)));

		XLDR_WRITE(&data_out,1);

		IS_ABORT(error);
	}

	// Free ressources
	if (xldr_teseo3_manager->io_mgr->out_buf != NULL)
		_clibs_free(xldr_teseo3_manager->io_mgr->out_buf);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("memory dump failed!" TP_ENDL));
		TP_REPORT_KO(xldr_teseo3_manager->instance, ITEM_DUMP);
	}
	else
	{
		TP_REPORT_OK(xldr_teseo3_manager->instance, ITEM_DUMP);
	}

	// Close file
	fclose(xldr_teseo3_manager->io_mgr->out_file);

	DRIVER_STANDARD(("Leave dump & close file" TP_ENDL));

	return error;
}

trgt_status_t xldr_teseo3_set(xldr_teseo3_manager_t * xldr_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	
	error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_ACK);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Set memory failed!" TP_ENDL));
		TP_REPORT_KO(xldr_teseo3_manager->instance, ITEM_SET);
	}
	else
	{
		TP_REPORT_OK(xldr_teseo3_manager->instance, ITEM_SET);
	}

	return error;
}


trgt_status_t xldr_teseo3_send_buffer(xldr_teseo3_manager_t * xldr_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt chunk_size = 0;
	tPU8  src_ptr = (tPU8)xldr_teseo3_manager->io_mgr->src_buf;
	tUInt remaining_bytes = xldr_teseo3_manager->io_mgr->src_size;

	DRIVER_STANDARD(("Sending buffer." TP_ENDL));

	while ((TRGT_STATUS_OK == error) && (0 < remaining_bytes))
	{
		/* Check bytes to sent */
		if (TESEO3_FLASHER_CHUNKSIZE < remaining_bytes)
		{
			chunk_size = TESEO3_FLASHER_CHUNKSIZE;
			remaining_bytes -= TESEO3_FLASHER_CHUNKSIZE;
		}
		else
		{
			chunk_size = remaining_bytes;
			remaining_bytes = 0;
		}

		/* Send buffer */
		XLDR_WRITE(src_ptr, chunk_size);

		error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_ACK);

		TP_REPORT_PROGRESS(xldr_teseo3_manager->instance, ITEM_FLASH,  (int)((float)(((float)100 / (float)xldr_teseo3_manager->io_mgr->src_size)) * (float)(xldr_teseo3_manager->io_mgr->src_size - remaining_bytes)) );

		src_ptr += chunk_size;

		IS_ABORT(error);
	}

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Buffer sending failed!" TP_ENDL));
		TP_REPORT_KO(xldr_teseo3_manager->instance, ITEM_FLASH);
	}
	else
	{
		TP_REPORT_OK(xldr_teseo3_manager->instance, ITEM_FLASH);
	}

	return error;
}

trgt_status_t xldr_teseo3_check_crc(xldr_teseo3_manager_t * xldr_teseo3_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Checking CRC." TP_ENDL));

	TP_REPORT_START(xldr_teseo3_manager->instance, ITEM_CRC);

	error = xldr_teseo3_waitack(xldr_teseo3_manager,TESEO3_WAIT_CRC);

	if (TRGT_STATUS_ERROR == error)
	{
		TP_REPORT_KO(xldr_teseo3_manager->instance, ITEM_CRC);
		DRIVER_ERROR(("CRC check failed!" TP_ENDL));
	}
	else
	{
		TP_REPORT_OK(xldr_teseo3_manager->instance, ITEM_CRC);
	}

	return error;
}

tVoid xldr_teseo3_shutdown(xldr_teseo3_manager_t * xldr_teseo3_manager)
{
}
