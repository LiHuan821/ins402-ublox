/** ***************************************************************************
 * @file send_packet.c UCB callbacks for assembling then sending serial packets
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 *****************************************************************************/
/*******************************************************************************
 *  Copyright 2020 ACEINNA, INC
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
*******************************************************************************/

//****************************
#include "ucb_packet.h"
#include "platform_version.h"
#include "platformAPI.h"
#include "crc16.h"
#include "string.h"
#include "uart.h"
#include <stdlib.h>
#include "app_version.h"
#include "cy_project.h"

/******************************************************************************
 * Function name:	UcbPacketTypeToPacketBytes
* @brief Convert the packet type enum into bytes ['P'] ['K'] -> 0x504B

 * @param [in] byte array, containing one byte
 * @return BOOL
 ******************************************************************************/
static BOOL UcbPacketTypeToPacketBytes(int32_t type, uint8_t bytes[])
{
    ucbPacketTableEntry_t *packetEntry = &ucbPackets[0];

    while (packetEntry->ptype != UCB_PACKET_LAST) {
        if (packetEntry->ptype == type) {
            bytes[0] = (uint8_t)((packetEntry->psync >> 8U) & 0xFFU);
            bytes[1] = (uint8_t)(packetEntry->psync & 0xFFU);
            return TRUE;
        }
        packetEntry++;
    }

    bytes[0] = 0;
    bytes[1] = 0;

    return FALSE;
}

/** ****************************************************************************
 * @name HandleUcbTx
 * @brief builds a UCB packet and then triggers transmission of it. Packet:
 *  Preamble = 0x5555
 *  Packet Type 0x####
 *  Length 0x##
 *  payload (uint8_t)data[Length]
 *  CRC 0x####
 * Trace: [SDD_UCB_PROCESS_OUT <-- SRC_UCB_OUT_PKT]
 * @param [in] port - port type UCB or CRM
 * @param [in] packetPtr -- buffer structure with payload, type and size
 * @retval valid packet in packetPtr TRUE
 ******************************************************************************/
void HandleUcbTx (UcbPacketStruct *ptrUcbPacket)
{
    uint16_t crc;
    uint8_t data[2];
    BOOL ret;

	/// get byte representation of packet type, index adjust required since sync
    /// isn't placed in data array
    ptrUcbPacket->sync_MSB = 0x55;
	ptrUcbPacket->sync_LSB = 0x55;

    ret = UcbPacketTypeToPacketBytes(ptrUcbPacket->packetType, data);
    if (ret) {
        ptrUcbPacket->code_MSB = data[0];
	    ptrUcbPacket->code_LSB = data[1];
    } else {
        ptrUcbPacket->payloadLength = 2;
        ptrUcbPacket->payload[0]    =  ptrUcbPacket->code_MSB;
        ptrUcbPacket->payload[1]    =  ptrUcbPacket->code_LSB;
        ptrUcbPacket->code_MSB = 0x15; // NAK
        ptrUcbPacket->code_LSB = 0x15;
    }

    crc = CalculateCRC((uint8_t *)&ptrUcbPacket->code_MSB, ptrUcbPacket->payloadLength + 3);
    ptrUcbPacket->payload[ptrUcbPacket->payloadLength+1]   = (crc >> 8) & 0xff;
    ptrUcbPacket->payload[ptrUcbPacket->payloadLength]     =  crc  & 0xff;

    uart7_write_dma((uint8_t *)&ptrUcbPacket->sync_MSB, ptrUcbPacket->payloadLength + 7);
}
/* end HandleUcbTx */
