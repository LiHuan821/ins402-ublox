/** ***************************************************************************
 * @file user_config.h
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *****************************************************************************/
/*******************************************************************************
Copyright 2020 ACEINNA, INC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#ifndef _USER_CONFIG_H
#define _USER_CONFIG_H

#include <stdint.h>
#include "constants.h"
#include "user_message.h"

/// User defined configuration strucrture
#pragma pack(4)
typedef struct {
    uint16_t dataCRC;   /// CRC of user configuration structure CRC-16
    uint16_t dataSize;  /// Size of the user configuration structure 

    // user uart packets 
    uint8_t RAWIMU_packet_rate;
    uint8_t BESTGNSS_packet_rate;
    uint8_t INSPVA_packet_rate;
    uint8_t INSSTD_packet_rate;

    // leverarm
    float pri_lever_arm[3];
    float vrp_lever_arm[3];
    float user_lever_arm[3];
    float rotation_rbv[3];

} UserConfigurationStruct;
#pragma pack()

typedef enum {
    USER_DATA_CRC                   = 0,
    USER_DATA_SIZE                    ,

    RAWIMU_PACKET_RATE          ,
    BESTGNSS_PACKET_RATE        ,
    INSPVA_PACKET_RATE          ,
    INSSTD_PACKET_RATE          ,

    USER_PRI_LEVER_ARM_BX       ,
    USER_PRI_LEVER_ARM_BY       ,
    USER_PRI_LEVER_ARM_BZ       ,
    USER_VRP_LEVER_ARM_BX       ,
    USER_VRP_LEVER_ARM_BY       ,
    USER_VRP_LEVER_ARM_BZ       ,
    USER_USER_LEVER_ARM_BX      ,
    USER_USER_LEVER_ARM_BY      ,
    USER_USER_LEVER_ARM_BZ      ,
    USER_ROTATION_RBVX          ,
    USER_ROTATION_RBVY          ,
    USER_ROTATION_RBVZ          ,

    USER_MAX_PARAM
} UserConfigParamNumber;


#define INVALID_PARAM           -1
#define INVALID_VALUE           -2
#define INVALID_CATEGORY        -3

BOOL SaveUserConfig(void);
BOOL RestoreDefaultUserConfig(void);

void user_configure_init(void);
void update_sys_params(void);
int32_t update_user_parameter(uint32_t number, uint8_t* data, BOOL offset);
BOOL update_user_param(userParamPayload*  pld, uint8_t *payloadLen);
BOOL get_all_user_params(uint8_t *payload, uint8_t *payloadLen);
BOOL get_block_user_params(uint8_t *payload, uint8_t *payloadLen);
BOOL update_block_user_params(uint8_t *payload, uint8_t *payloadLen);

// api: get/set user parameter
uint8_t get_rawimu_packet_rate(void);
void set_rawimu_packet_rate(uint8_t rate);
uint8_t get_bestgnss_packet_rate(void);
void set_bestgnss_packet_rate(uint8_t rate);
uint8_t get_inspva_packet_rate(void);
void set_inspva_packet_rate(uint8_t rate);
uint8_t get_insstd_packet_rate(void);
void set_insstd_packet_rate(uint8_t rate);

void get_pri_lever_arm(float32_t data[]);
void get_vrp_lever_arm(float32_t data[]);
void get_user_lever_arm(float32_t data[]);
void get_rotation_rbv(float32_t data[]);

#endif /* _USER_CONFIG_H */
