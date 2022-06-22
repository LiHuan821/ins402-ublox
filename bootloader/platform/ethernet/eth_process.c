#include "eth_process.h"
#include "ethernet.h"
#include "crc16.h"

#include "eth_process.h"
#include <stdio.h>
#include "app_version.h"
#include "platform_version.h"
#include "platformAPI.h"
#include "fifo.h"
#include "eeprom.h"

static binary_packet_format_t binary_packet_tx;

#define MESGID_COUNT    11
const uint16_t eth_input_mesgid[MESGID_COUNT] = {0x0b01, 0x0b02, 0xcc01, 0xcc0b, 0xcc02, 0xcc03, 0xcc04, 0xaa01, 0xaa02, 0xaa03, 0xaa04};
const uint8_t Core0_Name[3] = {"rtk"};
const uint8_t Core1_Name[3] = {"ins"};
static uint32_t rtkBinSize = 0;
static uint32_t insBinSize = 0;
static BOOL CoreSetSucc = 0;

void _eth_tx_binary_packet(binary_packet_format_t* p_bi)
{
    uint16_t crc;

    p_bi->header = 0x5555;

    crc = CalculateCRC((uint8_t *)&p_bi->mesgid, p_bi->length + 6);
    p_bi->data[p_bi->length + 1] = (crc >> 8) & 0xff;
    p_bi->data[p_bi->length] = crc & 0xff;

    ethernet_send_data((uint8_t *)p_bi, p_bi->length + 10);
}



static void _eth_rx_request_deviceinfo_handler(binary_packet_format_t* pbiprx)
{
    uint32_t serialNum   = cal_get_unit_serialnum();
    pbiprx->length = snprintf((char*)pbiprx->data, 250, "%s %s SN:%u", PRODUCT_NAME_STRING, APP_VERSION_STRING, serialNum);
    _eth_tx_binary_packet(pbiprx);
}


static void _eth_rx_request_ji_handler(binary_packet_format_t* pbiprx)
{
    Cy_FlashInit(false);
    _eth_tx_binary_packet(pbiprx);
    
    Cy_SysTick_DelayInMs(10);
    HW_EnforceBootMode();
    Cy_SysTick_DelayInMs(10);
    NVIC_SystemReset();
}

static void _eth_rx_request_ja_handler(binary_packet_format_t* pbiprx)
{
	uint16_t rtkRecvCRC = 0;
	uint16_t insRecvCRC = 0;
	uint16_t rtkCalcCRC = 0;
	uint16_t insCalcCRC = 0;
	uint32_t *rtkStartAddr = (uint32_t*)CM7_0_CODE_ADDR;
	uint32_t *insStartAddr = (uint32_t*)CM7_1_CODE_ADDR;
	
	if(pbiprx->length > 0)
	{
		rtkRecvCRC = (uint16_t)(pbiprx->data[1] << 8) + pbiprx->data[0];
		insRecvCRC = (uint16_t)(pbiprx->data[3] << 8) + pbiprx->data[2];

		rtkCalcCRC = CalculateCRC((uint8_t*)CM7_0_CODE_ADDR, rtkBinSize);
		insCalcCRC = CalculateCRC((uint8_t*)CM7_1_CODE_ADDR, insBinSize);

		/** we will check the whole bin CRC
		 ** if Write Success, we return "C0S" or "C1S"
		 ** if Failed, we return "C0F" or "C1F"
		 **/
		pbiprx->length = 6;
		pbiprx->data[0] = 'C';
		pbiprx->data[1] = '0';
		pbiprx->data[2] = (rtkCalcCRC == rtkRecvCRC)?'S':'F';
		pbiprx->data[3] = 'C';
		pbiprx->data[4] = '1';
		pbiprx->data[5] = (insCalcCRC == insRecvCRC)?'S':'F'; 

		_eth_tx_binary_packet(pbiprx);
		Cy_SysTick_DelayInMs(10);
		if((rtkCalcCRC == rtkRecvCRC) && (insCalcCRC == insRecvCRC))
		{
			EEPROM_PrepareToEnteApp();
			NVIC_SystemReset();
		}
		/** else, we will wait in BootLoader Mode **/
	}
	else
	{
	    _eth_tx_binary_packet(pbiprx);
	    Cy_SysTick_DelayInMs(10);    
	    EEPROM_PrepareToEnteApp();
	    NVIC_SystemReset();
	}
}

extern uint8_t core_to_upgrade ;
extern uint32_t size_to_erase ;

void ucb_set_core_by_ethernet(binary_packet_format_t* pbiprx,uint8_t* core_set,uint32_t* bin_size)
{
	if((pbiprx->data[0] == 'C') && (pbiprx->data[1] == '0'))
	{
		*core_set = CORE_CM7_0;
	}
	else if((pbiprx->data[0] == 'C') && (pbiprx->data[1] == '1'))
	{
		*core_set = CORE_CM7_1;
	}
	else
	{
		pbiprx->mesgid 	= 0x1515;
	}

    *bin_size = (pbiprx->data[2] << 24) | (pbiprx->data[3] << 16) | \
                (pbiprx->data[4] << 8) | (pbiprx->data[5] );

	if(pbiprx->length > 6)
	{
		if((*core_set == CORE_CM7_0) 
			&& (pbiprx->data[6] == 'r') && (pbiprx->data[7] == 't') && (pbiprx->data[8] == 'k'))
		{
			pbiprx->length = 3;
			pbiprx->data[2] = 'S';
			rtkBinSize = *bin_size;
		}
		else if((*core_set == CORE_CM7_1) 
			&& (pbiprx->data[6] == 'i') && (pbiprx->data[7] == 'n') && (pbiprx->data[8] == 's'))
		{
			pbiprx->length = 3;
			pbiprx->data[2] = 'S';
			insBinSize = *bin_size;
		}
		else
		{
			pbiprx->mesgid 	= 0x1515;	
			pbiprx->length = 3;
			pbiprx->data[2] = 'F';
		}
	}
}
static void _eth_rx_request_cs_handler(binary_packet_format_t* pbiprx)
{
    ucb_set_core_by_ethernet(pbiprx,&core_to_upgrade,&size_to_erase);    
    _eth_tx_binary_packet(pbiprx);
}

static void _eth_rx_packet_handler(binary_packet_format_t* pbiprx)
{
    switch (pbiprx->mesgid)
    {
        case 0xcc01: // request product device information
            _eth_rx_request_deviceinfo_handler(pbiprx);
            break;
        case 0xcc0b: // request product device information
            _eth_rx_request_deviceinfo_handler(pbiprx);
            break;
        case 0xaa01:
            _eth_rx_request_ji_handler(pbiprx);
            break;
        case 0xaa02:
            _eth_rx_request_ja_handler(pbiprx);
            break;
        case 0xaa03:
            ucb_write_app_by_ethernet(core_to_upgrade,pbiprx);
            break;
        case 0xaa04:
             _eth_rx_request_cs_handler(pbiprx);
            break;           
        default:
            break;
    }
}

void eth_recieve_process(void)
{
    static binary_packet_format_t binary_packet_rx;
    static uint16_t bytesInBuffer = 0, pos = 0;
    static uint8_t state = 0;
    static uint32_t sync = 0U, length = 0U;
	static uint8_t dataBuffer[2048U];
    static uint32_t len;
    uint8_t tmp;
    uint16_t header, mesgid, i, crc;

	while (1) {
        if (!bytesInBuffer) {
            bytesInBuffer = ethernet_get_data(dataBuffer, 2048U);
            if (!bytesInBuffer) {
                return; // nothing to do
            }
            pos = 0; 
        }

        tmp = dataBuffer[pos];
        pos++;
        bytesInBuffer--;

        switch (state)
        {
        case 0:
            sync = (sync << 8U) | tmp;
            header = sync >> 16U;
            if (header == 0x5555U) { // header
                mesgid = ((sync >> 8U) & 0xffU) | ((sync & 0xffU) << 8U);
                for (i = 0; i < MESGID_COUNT; i++) {
                    if (mesgid == eth_input_mesgid[i]) {
                        binary_packet_rx.header = header;
                        binary_packet_rx.mesgid = mesgid;
                        state = 1U;
                        len = 0U;
                        break;
                    }
                }
            }
            break;

        case 1:
            len++;
            length = (length >> 8U) | (tmp << 24U);
            if (len == 4U) {
                binary_packet_rx.length = length;
                if (length > BI_PACKET_MAX_PAYLOAD) {
                    state = 0U;
                } else {
                    state = 2U;
                }
                len = 0;
            }
            break;
        
        case 2:
            binary_packet_rx.data[len] = tmp;
            len++;
            if (len == (binary_packet_rx.length + 2U)) {
                crc = CalculateCRC((uint8_t*)&binary_packet_rx.mesgid, binary_packet_rx.length + 6);
                if (crc == (binary_packet_rx.data[binary_packet_rx.length] + (binary_packet_rx.data[binary_packet_rx.length+1] << 8U))) {
                    _eth_rx_packet_handler(&binary_packet_rx);
                }

                state = 0U;
                sync = 0U;
            }
            break;
        
        default:
            state = 0U;
            break;
        }
    }
}
