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

#include "teseo2/xldr_teseo2.h"
#include "teseo2/xldr_teseo2_bootloader.h"

/*****************************************************************************
   external declarations
*****************************************************************************/

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/

#define TESEO2_OUTPUT_SRAM                  0U
#define TESEO2_OUTPUT_SQI                   1U
#define TESEO2_OUTPUT_NOR                   2U
#define TESEO2_OUTPUT_ITCM                  3U

#define TESEO2_DEVICE_NOT_FLASHED           0x0BU
#define TESEO2_DEVICE_FLASHED               0x0AU
#define TESEO2_DEVICE_DUMPED                0x0BU
#define TESEO2_DEVICE_READY                 0xC7U
#define TESEO2_DEVICE_START_COMMUNICATION   0xA3U
#define TESEO2_FLASHER_READY                0x4AU
#define TESEO2_BAUD_RATE_CHANGED            0x38U
#define TESEO2_HOST_READY                   0x5AU
#define TESEO2_HOST_NOT_READY               0x5BU
#define TESEO2_DEVICE_ACK                   0xCCU
#define TESEO2_DEVICE_NACK                  0xE6U

#define TESEO2_DEBUG_DUMP                   0x0U
#define TESEO2_DEBUG_SET                    0x0U

#define TESEO2_TP_DEF_ENTRY_POINT           0x00000000U
#define TESEO2_TP_DEF_DEST_ADDR             0x00000000U
#define TESEO2_TP_ID_MSP                    0xBCD501F4U
#define TESEO2_TP_ID_LSP                    0x83984073U
#define TESEO2_TP_ID_SIZE                   0x8U

#define TESEO2_WAIT_READ_DRIVER  			100
#define TESEO2_WAIT_SHORT_ACK			    1000
#define TESEO2_WAIT_ACK						3500
#define TESEO2_WAIT_ERASING					60000
#define TESEO2_WAIT_CRC                     25000
#define TESEO2_WAIT_DUMP_CHUNK              3500

#define TESEO2_FLASHER_CHUNKSIZE            (5*1024)
#define UART_REC_MAX_BUFFER					        32

#ifdef _STREAM_DUMP
#define DRIVER_STREAM(x)        TP_STREAM(x)
#else
#define DRIVER_STREAM(x)        
#endif
#define DRIVER_STANDARD(x)   TP_PRINT(x)     
#define DRIVER_ERROR(x)      TP_ERROR(x)  
#define DRIVER_DEBUG(x)      TP_INFO(x)


#define XLDR_WRITE(b,s)     xldr_teseo2_write(xldr_teseo2_manager,(tPChar)(b),s);
#define XLDR_READ(b,s,t)    xldr_teseo2_read(xldr_teseo2_manager,(tPChar)(b),s,t)
#define XLDR_GETTIME()      xldr_teseo2_manager->io_mgr->gettime()

#define ICTATS

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

ICTATS tUInt xldr_teseo2_read(xldr_teseo2_manager_t * xldr_teseo2_manager, tPChar buf, int size, int timeout)
{
	tUInt nb = xldr_teseo2_manager->io_mgr->dev_read_cb((tPChar)(buf), size, timeout, xldr_teseo2_manager->instance);

	DRIVER_STREAM((TP_ENDL "< [%d] ", nb));

	for (tUInt i = 0; i < nb; i++)
	{
		DRIVER_STREAM((" %02x", *(((tPChar)(buf)+i)) & 0xFF));
	}

	return nb;
}


ICTATS void xldr_teseo2_write(xldr_teseo2_manager_t * xldr_teseo2_manager, tPChar buf, int size)
{
	DRIVER_STREAM((TP_ENDL "> [%d] ", size));

	for (int idx = 0; idx < size; idx++)
		DRIVER_STREAM(("%02x ", *((tPChar)(buf)+idx) & 0xFF));

	xldr_teseo2_manager->io_mgr->dev_write_cb((tPChar)(buf), size, xldr_teseo2_manager->instance );

	DRIVER_STREAM(("" TP_ENDL));
}

static trgt_status_t xldr_teseo2_waitack(xldr_teseo2_manager_t * xldr_teseo2_manager, tUInt timeout)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tU8 data_in;
	tUInt read_chars = 0;

	read_chars = XLDR_READ(&data_in, sizeof(data_in), timeout);

	if (0 == read_chars)
	{
		error = TRGT_STATUS_ERROR;
	}
	else if (TESEO2_DEVICE_ACK != data_in)
	{
		error = TRGT_STATUS_ERROR;
	}

	return error;
}

/*****************************************************************************
   function implementations (scope: module-exported)
*****************************************************************************/

trgt_status_t xldr_teseo2_init(trgt_command_t cmd, xldr_teseo2_inst inst, xldr_teseo2_config_t *trgt_cfg, trgt_io_manager_t *io_mgr, xldr_teseo2_manager_t ** pxldr)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt crc32;

	xldr_teseo2_manager_t * xldr_teseo2_manager = _clibs_calloc(1, sizeof(xldr_teseo2_manager_t));
	*pxldr = xldr_teseo2_manager;

	xldr_teseo2_manager->io_mgr = io_mgr;
	xldr_teseo2_manager->instance = inst;

	/* Fill binimg info */
	xldr_teseo2_manager->binimg_info.fileSize = xldr_teseo2_manager->io_mgr->src_size;
	xldr_teseo2_manager->binimg_info.crc32 = xldr_teseo2_manager->io_mgr->src_crc32;

	switch (trgt_cfg->mem)
	{
	case TRGT_MEM_SQI:
		xldr_teseo2_manager->binimg_info.bootMode = TESEO2_OUTPUT_SQI;
		break;

	case TRGT_MEM_NOR:
		xldr_teseo2_manager->binimg_info.bootMode = TESEO2_OUTPUT_NOR;
		break;

	case TRGT_MEM_RAM:
		xldr_teseo2_manager->binimg_info.bootMode = TESEO2_OUTPUT_SRAM;
		break;

	default:
		break;
	}

	xldr_teseo2_manager->binimg_info.destinationAddress = trgt_cfg->destaddr;
	xldr_teseo2_manager->binimg_info.entryPoint = trgt_cfg->entry;
	xldr_teseo2_manager->binimg_info.eraseNVM = trgt_cfg->nvm_erase;
	xldr_teseo2_manager->binimg_info.programOnly = trgt_cfg->program_only;
	xldr_teseo2_manager->binimg_info.nvmEraseSize = trgt_cfg->nvm_size;
	xldr_teseo2_manager->binimg_info.baudRate = trgt_cfg->baudrate;

	/* No switch on command line yet */
	xldr_teseo2_manager->binimg_info.eraseOnly = (TRGT_CMD_ERASE == cmd) ? TRUE : FALSE;
	xldr_teseo2_manager->binimg_info.nvmOffset = trgt_cfg->nvm_offset;
	xldr_teseo2_manager->binimg_info.debug = trgt_cfg->debug;
	xldr_teseo2_manager->binimg_info.debugAction = trgt_cfg->debugAction;
	xldr_teseo2_manager->binimg_info.debugAddress = trgt_cfg->debugAddress;
	xldr_teseo2_manager->binimg_info.debugData = trgt_cfg->debugData;

	/* Prepare preamble */
	xldr_teseo2_manager->preamble.destinationAddress = TESEO2_TP_DEF_DEST_ADDR;
	xldr_teseo2_manager->preamble.identifier_msp = TESEO2_TP_ID_MSP;
	xldr_teseo2_manager->preamble.identifier_lsp = TESEO2_TP_ID_LSP;
	xldr_teseo2_manager->preamble.options = (0xFFU << 16) | (1U << 8);

	/* Prepare bootloader */
	xldr_teseo2_manager->bootloader.buf_ptr = (tPChar)&xldr_teseo2_bootloader;
	xldr_teseo2_manager->bootloader.size = xldr_teseo2_bootloader_len;
	xldr_teseo2_manager->bootloader.entry = 0;

	crc32 = 0;

	// Calculate CRC32 on bootloader size
	CRC32_EVAL_INT(crc32, crc32, xldr_teseo2_manager->bootloader.size);
	// Calculate CRC32 using the *new* "entry point"
	CRC32_EVAL_INT(crc32, crc32, xldr_teseo2_manager->bootloader.entry);
	// Calculate CRC32 on bootloader code
	CRC32_EVAL_BUF(crc32, crc32, xldr_teseo2_manager->bootloader.buf_ptr, xldr_teseo2_manager->bootloader.size);

	xldr_teseo2_manager->bootloader.crc32 = crc32;

	DRIVER_DEBUG(("Binary Image Info" TP_ENDL ));
	DRIVER_DEBUG(("size                : %d" TP_ENDL, xldr_teseo2_manager->binimg_info.fileSize));
	DRIVER_DEBUG(("crc32               : 0x%x" TP_ENDL, xldr_teseo2_manager->binimg_info.crc32));
	DRIVER_DEBUG(("memory              : %d" TP_ENDL, xldr_teseo2_manager->binimg_info.bootMode));
	DRIVER_DEBUG(("destination address : 0x%X" TP_ENDL, xldr_teseo2_manager->binimg_info.destinationAddress));
	DRIVER_DEBUG(("entry point         : 0x%x" TP_ENDL, xldr_teseo2_manager->binimg_info.entryPoint));
	DRIVER_DEBUG(("erase NVM           : %s" TP_ENDL, xldr_teseo2_manager->binimg_info.eraseNVM?"true":"false"));
	DRIVER_DEBUG(("program only        : %s" TP_ENDL, xldr_teseo2_manager->binimg_info.programOnly ? "true" : "false"));
	DRIVER_DEBUG(("NVM size            : 0x%x" TP_ENDL, xldr_teseo2_manager->binimg_info.nvmEraseSize));
	DRIVER_DEBUG(("baud rate           : %d" TP_ENDL, xldr_teseo2_manager->binimg_info.baudRate));
	DRIVER_DEBUG(("erase only          : %s" TP_ENDL, xldr_teseo2_manager->binimg_info.eraseOnly?"true" : "false"));
	DRIVER_DEBUG(("nvm offset          : 0x%x" TP_ENDL, xldr_teseo2_manager->binimg_info.nvmOffset));
	DRIVER_DEBUG(("debug enable        : %s" TP_ENDL, xldr_teseo2_manager->binimg_info.debug?"true" : "false"));
	DRIVER_DEBUG(("debug action        : %s" TP_ENDL, xldr_teseo2_manager->binimg_info.debugAction?"dump":"set"));
	DRIVER_DEBUG(("debug address       : 0x%x" TP_ENDL, xldr_teseo2_manager->binimg_info.debugAddress));
	DRIVER_DEBUG(("debug data          : 0x%x" TP_ENDL, xldr_teseo2_manager->binimg_info.debugData));
	DRIVER_DEBUG(("Bootloader" TP_ENDL));
	DRIVER_DEBUG(("Size                : %d" TP_ENDL, xldr_teseo2_manager->bootloader.size = xldr_teseo2_bootloader_len));
	DRIVER_DEBUG(("Entry               : %x" TP_ENDL, xldr_teseo2_manager->bootloader.entry));

	return error;
}

trgt_status_t xldr_teseo2_check_baudrate(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_ERROR;
	tU8 data_in, data_out;
	tUInt read_chars = 0;

	data_out = TESEO2_BAUD_RATE_CHANGED;
	XLDR_WRITE(&data_out, sizeof(data_out));
	read_chars = XLDR_READ(&data_in, sizeof(data_in), TESEO2_WAIT_READ_DRIVER);

	if (0 < read_chars)
	{
		if (TESEO2_DEVICE_ACK == data_in)
		{
			error = TRGT_STATUS_OK;
		}
	}

	return error;
}

trgt_status_t xldr_teseo2_check_hostready(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tU8 data_out;

	data_out = TESEO2_HOST_READY;
	XLDR_WRITE(&data_out, sizeof(data_out));

	error = xldr_teseo2_waitack(xldr_teseo2_manager, TESEO2_WAIT_SHORT_ACK);

	return error;
}

trgt_status_t xldr_teseo2_send_preamble(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Sending preamble." TP_ENDL));
	XLDR_WRITE(&xldr_teseo2_manager->preamble, sizeof(xldr_teseo2_preamble_t));

	return error;
}

trgt_status_t xldr_teseo2_send_bootloader(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tU8 data_out;
	tUInt chunk_size = 0;
	tPU8  src_ptr = (tPU8)xldr_teseo2_manager->bootloader.buf_ptr;
	tUInt remaining_bytes = xldr_teseo2_manager->bootloader.size;

	DRIVER_STANDARD(("Sending bootloader." TP_ENDL));

	// Write the bootstrap CRC32
	XLDR_WRITE(&xldr_teseo2_manager->bootloader.crc32, sizeof(xldr_teseo2_manager->bootloader.crc32));
	// Write the bootstrap Size
	XLDR_WRITE(&xldr_teseo2_manager->bootloader.size, sizeof(xldr_teseo2_manager->bootloader.size));
	// Write the bootstrap Entry Point
	XLDR_WRITE(&xldr_teseo2_manager->bootloader.entry, sizeof(xldr_teseo2_manager->bootloader.entry));

	// Write bootloader binary image
	while ((TRGT_STATUS_OK == error) && (0 < remaining_bytes))
	{
		/* Check bytes to sent */
		if (TESEO2_FLASHER_CHUNKSIZE < remaining_bytes)
		{
			chunk_size = TESEO2_FLASHER_CHUNKSIZE;
			remaining_bytes -= TESEO2_FLASHER_CHUNKSIZE;
		}
		else
		{
			chunk_size = remaining_bytes;
			remaining_bytes = 0;
		}

		/* Send buffer */
		XLDR_WRITE(src_ptr, chunk_size);

		src_ptr += chunk_size;
	}

	error = xldr_teseo2_waitack(xldr_teseo2_manager, TESEO2_WAIT_ACK);

	if (TRGT_STATUS_OK == error)
	{
		DRIVER_STANDARD(("Waiting flasher." TP_ENDL));

		data_out = TESEO2_FLASHER_READY;
		XLDR_WRITE(&data_out, sizeof(data_out));

		error = xldr_teseo2_waitack(xldr_teseo2_manager, TESEO2_WAIT_ACK);
	}

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Flasher not ready!" TP_ENDL));
	}

	return error;
}

trgt_status_t xldr_teseo2_send_bininfo(xldr_teseo2_manager_t * xldr_teseo2_manager, const tBool waitForAck)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt read_chars = 0;

	DRIVER_STANDARD(("Sending binary info." TP_ENDL));

	XLDR_WRITE(&xldr_teseo2_manager->binimg_info, sizeof(xldr_teseo2_manager->binimg_info));

	if (TRUE == waitForAck)
	{
		error = xldr_teseo2_waitack(xldr_teseo2_manager, TESEO2_WAIT_SHORT_ACK);
	}

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Binary info sending failed!" TP_ENDL));
	}

	return error;
}

trgt_status_t xldr_teseo2_wait_devinit(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Initializing bootloader." TP_ENDL));

	error = xldr_teseo2_waitack(xldr_teseo2_manager,TESEO2_WAIT_SHORT_ACK);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Bootloader initialization failed!" TP_ENDL));
	}

	return error;
}

trgt_status_t xldr_teseo2_wait_eraseprog(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Erasing program flash." TP_ENDL));

	TP_REPORT_START(xldr_teseo2_manager->instance,ITEM_FLASH_ERASE);

	error = xldr_teseo2_waitack(xldr_teseo2_manager, TESEO2_WAIT_ERASING);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Program flash erase failed!" TP_ENDL));
		TP_REPORT_KO(xldr_teseo2_manager->instance, ITEM_FLASH_ERASE);
	}
	else
	{
		TP_REPORT_OK(xldr_teseo2_manager->instance, ITEM_FLASH_ERASE);
	}

	return error;
}

trgt_status_t xldr_teseo2_wait_erasenvm(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Erasing NVM flash." TP_ENDL));

	TP_REPORT_START(xldr_teseo2_manager->instance, ITEM_NVM);

	error = xldr_teseo2_waitack(xldr_teseo2_manager, TESEO2_WAIT_ERASING);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("NVM flash erase failed!" TP_ENDL));
		TP_REPORT_KO(xldr_teseo2_manager->instance, ITEM_NVM);
	}
	else
	{
		TP_REPORT_OK(xldr_teseo2_manager->instance, ITEM_NVM);
	}

	return error;
}

trgt_status_t xldr_teseo2_dump(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt chunk_size = 0;
	tU8  data_out = TESEO2_DEVICE_ACK;
	tU8  host_ready = TESEO2_FLASHER_READY;
	tUInt remaining_bytes = xldr_teseo2_manager->binimg_info.debugSize * 4;

	/// ADD ON
	XLDR_WRITE(&host_ready, 1);

	/* Create a buffer to read memory */
	xldr_teseo2_manager->io_mgr->out_buf = (tPChar)_clibs_malloc(TESEO2_FLASHER_CHUNKSIZE);

	if (NULL == xldr_teseo2_manager->io_mgr->out_buf)
	{
		DRIVER_ERROR(("Not enough memory!" TP_ENDL));
		error = TP_STATUS_ERROR;
	}

	if (NULL == xldr_teseo2_manager->io_mgr->out_file)
	{
		DRIVER_ERROR(("File handle NULL" TP_ENDL));
		error = TP_STATUS_ERROR;
	}

	while ((TRGT_STATUS_OK == error) && (0 < remaining_bytes))
	{
		DRIVER_DEBUG(("Remaining bytes : %d" TP_ENDL, remaining_bytes));

		/* Check bytes to sent */
		if (UART_REC_MAX_BUFFER < remaining_bytes)
		{
			chunk_size = UART_REC_MAX_BUFFER;
			remaining_bytes -= UART_REC_MAX_BUFFER;
		}
		else
		{
			chunk_size = remaining_bytes;
			remaining_bytes = 0;
		}


		/* read buffer */
		if (XLDR_READ(xldr_teseo2_manager->io_mgr->out_buf, chunk_size, TESEO2_WAIT_DUMP_CHUNK) != chunk_size)
		{
			error = TRGT_STATUS_ERROR;

			DRIVER_ERROR(("UART reception time out" TP_ENDL));
		}
		else
		{
			/* Copy buffer to file */
			if (fwrite((const void *)xldr_teseo2_manager->io_mgr->out_buf, 1, chunk_size, xldr_teseo2_manager->io_mgr->out_file) != chunk_size)
			{
				error = TRGT_STATUS_ERROR;

				DRIVER_ERROR(("Writing error in dump file : " TP_ENDL));
			}
		}

		TP_REPORT_PROGRESS(xldr_teseo2_manager->instance, ITEM_DUMP, (int)((float)(((float)100 / (float)xldr_teseo2_manager->binimg_info.debugSize)) * (float)(xldr_teseo2_manager->binimg_info.debugSize - remaining_bytes)));

		XLDR_WRITE(&data_out, 1);

		IS_ABORT(error);
	}

	// Free ressources
	if (xldr_teseo2_manager->io_mgr->out_buf != NULL)
		_clibs_free(xldr_teseo2_manager->io_mgr->out_buf);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("memory dump failed!" TP_ENDL));
		TP_REPORT_KO(xldr_teseo2_manager->instance, ITEM_DUMP);
	}
	else
	{
		TP_REPORT_OK(xldr_teseo2_manager->instance, ITEM_DUMP);
	}

	// Close file
	fclose(xldr_teseo2_manager->io_mgr->out_file);

	DRIVER_STANDARD(("Leave dump & close file" TP_ENDL));

	return error;
}

trgt_status_t xldr_teseo2_set(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	error = xldr_teseo2_waitack(xldr_teseo2_manager, TESEO2_WAIT_ACK);

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Set memory failed!" TP_ENDL));
		TP_REPORT_KO(xldr_teseo2_manager->instance, ITEM_SET);
	}
	else
	{
		TP_REPORT_OK(xldr_teseo2_manager->instance, ITEM_SET);
	}

	return error;
}


trgt_status_t xldr_teseo2_send_buffer(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;
	tUInt chunk_size = 0;
	tPU8  src_ptr = (tPU8)xldr_teseo2_manager->io_mgr->src_buf;
	tUInt remaining_bytes = xldr_teseo2_manager->io_mgr->src_size;

	DRIVER_STANDARD(("Sending buffer." TP_ENDL));

	while ((TRGT_STATUS_OK == error) && (0 < remaining_bytes))
	{
		/* Check bytes to sent */
		if (TESEO2_FLASHER_CHUNKSIZE < remaining_bytes)
		{
			chunk_size = TESEO2_FLASHER_CHUNKSIZE;
			remaining_bytes -= TESEO2_FLASHER_CHUNKSIZE;
		}
		else
		{
			chunk_size = remaining_bytes;
			remaining_bytes = 0;
		}

		/* Send buffer */
		XLDR_WRITE(src_ptr, chunk_size);

		error = xldr_teseo2_waitack(xldr_teseo2_manager, TESEO2_WAIT_ACK);

		IS_ABORT(error);

		TP_REPORT_PROGRESS(xldr_teseo2_manager->instance, ITEM_FLASH, (int)((float)(((float)100 / (float)xldr_teseo2_manager->io_mgr->src_size)) * (float)(xldr_teseo2_manager->io_mgr->src_size - remaining_bytes)));

		src_ptr += chunk_size;
	}

	if (TRGT_STATUS_ERROR == error)
	{
		DRIVER_ERROR(("Buffer sending failed!" TP_ENDL));
		TP_REPORT_KO(xldr_teseo2_manager->instance, ITEM_FLASH);
	}
	else
	{
		TP_REPORT_OK(xldr_teseo2_manager->instance, ITEM_FLASH);
	}

	return error;
}



trgt_status_t xldr_teseo2_check_crc(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
	trgt_status_t error = TRGT_STATUS_OK;

	DRIVER_STANDARD(("Checking CRC." TP_ENDL));

	error = xldr_teseo2_waitack(xldr_teseo2_manager, TESEO2_WAIT_CRC);

	if (TRGT_STATUS_ERROR == error)
	{
		TP_REPORT_KO(xldr_teseo2_manager->instance, ITEM_CRC);
		DRIVER_ERROR(("CRC check failed!" TP_ENDL));
	}
	else
	{
		TP_REPORT_OK(xldr_teseo2_manager->instance, ITEM_CRC);
	}

	return error;
}

tVoid xldr_teseo2_shutdown(xldr_teseo2_manager_t * xldr_teseo2_manager)
{
}
