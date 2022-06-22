/** ***************************************************************************
 * @file handle_packet.c functions for handling serial UCB packets and CRM
 *       packets
 * @brief some of the callbacks for message handling of non-sensor data to be
 *        sent to Nav-View eg "PING".
 * @Author
 * @date   September, 2008
 * @brief  Copyright (c) 2013, 2014 All Rights Reserved.
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 *****************************************************************************/

//********************************
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "cy_project.h"
#include "app_version.h"
#include "platform_version.h"
#include "ucb_packet.h"
#include "eeprom.h"
#include "eepromAPI.h"
#include "crc16.h"
#include "platformAPI.h"
#include "uart.h"
#include "systick/cy_systick.h"
#include "core_cm0plus.h"
#include "eth_process.h"

ucbPacketTableEntry_t ucbPackets[] = {
    {UCB_PING_PRODUCT,       0x7047},   //  "pG"
    {UCB_GET_APPVERSION,     0x6756},   //  "gV"
    {UCB_J2IAP,              0x4A49},   //  "JI"
    {UCB_J2APP,              0x4A41},   //  "JA"
    {UCB_CORE_SET,           0x4353},   //  "CS"
    {UCB_WRITE_APP,          0x5741},   //  "WA"
    {UCB_PACKET_LAST,        0x0000}    //  " "
};

/** ****************************************************************************
 * @name _SetNak
 * @brief set up UCB error NAK packet. Return NAK with requested packet type in
 *        data field. HandleUcbTx() needs to be called.
 * Trace:
 * @param [in] port - port type UCB or CRM
 * @param [out] packetPtr - filled in packet from the mapped physical port
 * @retval N/A
 ******************************************************************************/
static void _SetNak(UcbPacketStruct *ptrUcbPacket)
{

	/// return NAK, requested packet type placed in data field by external port
	ptrUcbPacket->packetType 	= UCB_NAK;
	ptrUcbPacket->payloadLength = UCB_PACKET_TYPE_LENGTH;
}

static void _ucb_ping_product(UcbPacketStruct *ptrUcbPacket)
{
    int32_t len; 
    uint32_t serialNum   = cal_get_unit_serialnum();
    len = snprintf((char*)ptrUcbPacket->payload, 250, "%s %s SN:%u", PRODUCT_NAME_STRING, APP_VERSION_STRING, serialNum );
    ptrUcbPacket->payloadLength = len;
    HandleUcbTx(ptrUcbPacket);
}

static void _ucb_get_appversion(UcbPacketStruct *ptrUcbPacket)
{
    int32_t len = snprintf((char*)ptrUcbPacket->payload, 250, "%s", APP_VERSION_STRING);
    ptrUcbPacket->payloadLength = len;
    HandleUcbTx(ptrUcbPacket);
}

/** ****************************************************************************
 * @name _ucb_jump2boot
 * @brief
 * Trace:
 *	[SDD_UCB_UNLOCK_EEPROM <-- SRC_UCB_UNLOCK_EEPROM]
 *
 * @param [in] port -  number request came in on, the reply will go out this port
 * @param [out] packetPtr - data part of packet
 * @retval N/A
 ******************************************************************************/
static void _ucb_jump2boot(UcbPacketStruct *ptrUcbPacket)
{
    Cy_FlashInit(false /*blocking*/);
	HandleUcbTx(ptrUcbPacket);

    Cy_SysTick_DelayInMs(10);
    HW_EnforceBootMode();
    Cy_SysTick_DelayInMs(10);
    NVIC_SystemReset();
} 

static void _ucb_jump2app(UcbPacketStruct *ptrUcbPacket)
{
    HandleUcbTx(ptrUcbPacket);
    Cy_SysTick_DelayInMs(10);    
    EEPROM_PrepareToEnteApp();
    NVIC_SystemReset();
} 

void _ucb_set_core(UcbPacketStruct *ptrUcbPacket,uint8_t* core_set,uint32_t* bin_size)
{
	HandleUcbTx(ptrUcbPacket);
    if((ptrUcbPacket->payload[0] == 'C') && (ptrUcbPacket->payload[1] == '0'))
    {
        *core_set = CORE_CM7_0;
    }
    else if((ptrUcbPacket->payload[0] == 'C') && (ptrUcbPacket->payload[1] == '1'))
    {
        *core_set = CORE_CM7_1;
    }
    *bin_size = (ptrUcbPacket->payload[2] << 24) | (ptrUcbPacket->payload[3] << 16) | \
                (ptrUcbPacket->payload[4] << 8) | (ptrUcbPacket->payload[5] );
}

/** ****************************************************************************
 * @name _ucb_write_app
 * @brief Write data as 16 bit cells into an unlocked EEPROM.
 *
 * @param [in] port -  number request came in on, the reply will go out this port
 * @param [out] packetPtr - data part of packet
 * @retval N/A
 ******************************************************************************/
static void _ucb_write_app(UcbPacketStruct *ptrUcbPacket,uint8_t core_to_upgrade)
{
    uint32_t startAddress;
    uint8_t  wordsToWrite;
    uint16_t bytesToWrite;
// packet structure 
// header   code  payload len  start addr  numbytes              crc 
// 5555     5747  [x]          [yyyy]     [z]        [payload]   [cc]
    startAddress = (uint32_t)((ptrUcbPacket->payload[0] << 24) |
                               ptrUcbPacket->payload[1] << 16 |
                               ptrUcbPacket->payload[2] << 8 |
                               ptrUcbPacket->payload[3]);
    wordsToWrite = ptrUcbPacket->payload[4];
    bytesToWrite = (uint16_t)wordsToWrite;
    if((ptrUcbPacket->payload[1] == 0x01) && (ptrUcbPacket->payload[2] == 0x9f) && (ptrUcbPacket->payload[3] ==  0x80))
    {

    }

    /// verify that the packet length matches packet specification
    if ((ptrUcbPacket->payloadLength == (bytesToWrite + 5))) {

		//if(!EEPROM_WriteApp(startAddress,&ptrUcbPacket->payload[5],bytesToWrite)){           //TODO:
		if(EEPROM_WriteApp(startAddress,bytesToWrite,&(ptrUcbPacket->payload[5]),core_to_upgrade) == TRUE)
                {    //TODO:
            ptrUcbPacket->payloadLength = 5;
        } else {
            _SetNak(ptrUcbPacket);
        }
    } else {
        _SetNak(ptrUcbPacket);
    }
	HandleUcbTx(ptrUcbPacket);
}


void ucb_write_app_by_ethernet(uint8_t core_to_upgrade,binary_packet_format_t* pbiprx)
{
    uint8_t* data = (uint8_t *)pbiprx->data;
    uint32_t startAddress;
    uint32_t  wordsToWrite;
    uint16_t bytesToWrite;
    uint32_t packet_len = pbiprx->length;
// packet structure 
// header   code  payload len  start addr  numbytes              crc 
// 5555     5747  [x]          [yyyy]     [z]        [payload]   [cc]
    startAddress = (uint32_t)((data[3] ) |
                               (data[2] << 8) |
                               (data[1] << 16) |
                               (data[0] << 24));
    wordsToWrite = (data[4] << 24) |
                               (data[5] << 16) |
                               (data[6] << 8) |
                               (data[7]);
    bytesToWrite = (uint16_t)wordsToWrite;

    /// verify that the packet length matches packet specification
    if ((packet_len == (bytesToWrite + 8))) 
	{
		if(EEPROM_WriteApp(startAddress,bytesToWrite,&(data[8]),core_to_upgrade) == TRUE)
        {    //TODO:
            pbiprx->length = 0;
        } 
		else 
		{
            pbiprx->mesgid = 0;
        }
    } 
    else 
    {
        pbiprx->mesgid = 0;
    }
    pbiprx->length = 0;
    _eth_tx_binary_packet(pbiprx);
}
/** ****************************************************************************
 * @name _UcbError
 * @brief UCB error packet
 * Trace: [SDD_UCB_UNKNOWN_02 <-- SRC_UCB_UNKNOWN]
 *        [SDD_UCB_TIMEOUT_02 <-- SRC_UCB_TIMEOUT_REPLY]
 *        [SDD_UCB_CRC_FAIL_02 <-- SRC_UCB_CRCFAIL_REPLY]
 * @param [in] port - port type UCB or CRM
 * @param [out] packetPtr - filled in packet from the mapped physical port
 * @retval N/A
 ******************************************************************************/
static void _UcbError(UcbPacketStruct *ptrUcbPacket)
{
	/// return NAK, requested packet type placed in data field by external port
	ptrUcbPacket->packetType 	= UCB_NAK;
	ptrUcbPacket->payloadLength = UCB_PACKET_TYPE_LENGTH;
    HandleUcbTx(ptrUcbPacket);
}

/** ****************************************************************************
 * @name HandleUcbPacket - API
 * @brief general handler
 * Trace: [SDD_HANDLE_PKT <-- SRC_HANDLE_PACKET]
 * @param [in] port - port type UCB or CRM
 * @param [out] packetPtr - filled in packet from the mapped physical port
 * @retval N/A
 ******************************************************************************/
uint8_t core_to_upgrade = 0xff;		/**< The default value is 0xff to prevent the RTK from continuing to burn when the setting is wrong **/
uint32_t size_to_erase = 0;

uint32_t get_bin_size()
{
    return size_to_erase;
}

void HandleUcbPacket (UcbPacketStruct* const ptrUcbPacket)
{
    if (ptrUcbPacket) {
        switch (ptrUcbPacket->packetType) {
            case UCB_PING_PRODUCT:
                _ucb_ping_product(ptrUcbPacket);
                break;
            case UCB_GET_APPVERSION:
                _ucb_get_appversion(ptrUcbPacket);
                break;
            case UCB_J2IAP:
                _ucb_jump2boot(ptrUcbPacket);
                break;
            case UCB_J2APP:
                _ucb_jump2app(ptrUcbPacket);
                break;
            case UCB_CORE_SET:
                _ucb_set_core(ptrUcbPacket,&core_to_upgrade,&size_to_erase);
                break;                
            case UCB_WRITE_APP:
                _ucb_write_app(ptrUcbPacket,core_to_upgrade);
                break;

            default:
                _UcbError(ptrUcbPacket);
                break;  /// default handler - unknown send NAK
        }
    }
} /* end HandleUcbPacket() */

/** ****************************************************************************
 * @name HandleUcbRx
 * @brief handles received ucb packets
 * Trace:
 *	[SDD_UCB_TIMEOUT_01 <-- SRC_HANDLE_UCB_RX]
 *	[SDD_UCB_PACKET_CRC <-- SRC_HANDLE_UCB_RX]
 *	[SDD_UCB_CONVERT_DATA <-- SRC_HANDLE_UCB_RX]
 *	[SDD_UCB_STORE_DATA <-- SRC_HANDLE_UCB_RX]
 *	[SDD_UCB_UNKNOWN_01 <-- SRC_HANDLE_UCB_RX]
 *	[SDD_UCB_CRC_FAIL_01 <-- SRC_HANDLE_UCB_RX]
 *	[SDD_UCB_VALID_PACKET <-- SRC_HANDLE_UCB_RX]
 *
 * @param [in] port - logical port type
 * @param [out] packetPtr - UCB packet to read the packet into
 * @retval TRUE if a full packet has been seen (can fail CRC)
 *         FALSE if needing more to fill in a packet
 ******************************************************************************/
static int32_t get_ucb_packet (void)
{
    static UcbPacketStruct ucbPacket;
    static uint8_t dataBuffer[512];
    static int bytesInBuffer = 0, state = 0, crcError = 0, len = 0;
    static uint8_t *ptr = &ucbPacket.payload[0];
    static uint16_t crcMsg = 0, code;
	static uint32_t sync = 0;
    unsigned char tmp;
	unsigned int  pos = 0, synced = 0, type;
	uint16_t crcCalc;
    ucbPacketTableEntry_t *syncTable;
	static uint32_t loop_count = 0;
    
	while(1){
        loop_count++;
        if (loop_count == 50000) {
            loop_count = 0;
        }

        if(!bytesInBuffer){
            bytesInBuffer = fifo_get(&uart_user_rx_fifo, dataBuffer, sizeof(dataBuffer));
            if(!bytesInBuffer){
                return 0; // nothing to do
            }
            pos = 0; 
        }
        tmp = dataBuffer[pos++];
        bytesInBuffer--;
        sync   = (sync << 8) | tmp;
        synced = 0;
        if(state == 0)
        {
            if((sync & 0xFFFF0000) == 0x55550000){
                code = sync & 0xffff;
                syncTable = ucbPackets;
                while (syncTable->ptype != UCB_PACKET_LAST)
                {
                    if (syncTable->psync == code){
                        synced = 1;
                        type   = syncTable->ptype;
                        break;
                    }
                    syncTable++;
                }
            }
        }
        if(synced){
            ucbPacket.packetType    = type;
            ucbPacket.payloadLength = 0;
            ucbPacket.code_MSB      = (sync >> 8) & 0xff;
            ucbPacket.code_LSB      = sync & 0xff;
        	state  = 1;
		    len    = 0;
            synced = 0;
            continue;
        }
        switch(state){
        case 0:
            break;
        case 1:
            ucbPacket.payloadLength = tmp;
            if(tmp == 0){
                state = 3;  // crc next
            }else{
                state = 2;  // data next
                len   = 0;
            }
            ptr   = ucbPacket.payload;
            break;
        case 2:
            if(len++ > UCB_MAX_PAYLOAD_LENGTH){
                state = 0;
                break;
            }
            *ptr++ = tmp;
            if(len == ucbPacket.payloadLength){
                //crc next
                state  = 3;
                crcMsg = 0; 
            }
            break;
        case 3:
            crcMsg = tmp;
            *ptr++ = tmp;   
            state = 4;
            break;
        case 4:
            state   = 0;
            crcMsg  = crcMsg | ((uint16_t)tmp << 8);
            *ptr++  = tmp;   
            crcCalc = CalculateCRC((uint8_t*)&ucbPacket.code_MSB, len + 3);
            if(crcMsg != crcCalc){
                crcError++;
            }else {
                // process message here
               HandleUcbPacket (&ucbPacket);
               return 0;   // will come back later
            }
            break;
        default:
            while(1){}; // should not be here
        }
    }

} /* end HandleUcbRx */

void process_user_commands(void)
{
    /// check received packets and handle appropriately
    get_ucb_packet();

} /* end ProcessUcbCommands() */
