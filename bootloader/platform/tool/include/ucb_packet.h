/** ***************************************************************************
  * @file ucb_packet.h utility functions for interfacing with Memsic proprietary
  *       UCB (unified code base) packets.  UCB packet structure
  * @Author rhilles
  * @date  2010-08-03 10:20:52 -0700 (Tue, 03 Aug 2010)
  * @rev 15924
  * @brief  Copyright (c) 2013, 2014 All Rights Reserved.
  *
  * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY O ANY
  * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  * PARTICULAR PURPOSE.
  *
  * @brief these are in ucb_packet_types.def on the 440 these were in
  *        xbowProtocol.h
  *****************************************************************************/
#ifndef UCB_PACKET_H
#define UCB_PACKET_H

#include <stdint.h>

#define UCB_SYNC_LENGTH				2
#define UCB_PACKET_TYPE_LENGTH		2
#define UCB_PAYLOAD_LENGTH_LENGTH	1
#define UCB_CRC_LENGTH				2

/// preamble sync bytes
static const uint8_t UCB_SYNC [UCB_SYNC_LENGTH] = { 0x55, 0x55 };

/// packet field type definitions
#define UCB_MAX_PAYLOAD_LENGTH		255

enum {
    UCB_PING_PRODUCT,
    UCB_GET_APPVERSION,
    UCB_J2IAP,
    UCB_J2APP,
    UCB_CORE_SET,
    UCB_WRITE_APP,
//*************************************************
    UCB_NAK,
    UCB_PACKET_LAST
};

typedef struct {
    uint8_t   ptype;
    uint16_t  psync;
} ucbPacketTableEntry_t;

typedef struct {
     uint8_t       packetType;      // 0
     uint8_t       systemType;      // 1
     uint8_t       spiAddress;      // 2
     uint8_t       sync_MSB;        // 3
     uint8_t       sync_LSB;        // 4
     uint8_t       code_MSB;        // 5
     uint8_t       code_LSB;        // 6
     uint8_t	   payloadLength;   // 7
     uint8_t       payload[UCB_MAX_PAYLOAD_LENGTH + 3]; // aligned to 4 bytes 
} UcbPacketStruct;

extern ucbPacketTableEntry_t ucbPackets[];

/**
 * @brief 
 * 
 * @param port 
 * @param ptrUcbPacket 
 */
extern void HandleUcbTx (UcbPacketStruct *ptrUcbPacket);


#endif
