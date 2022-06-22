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

/// User defined configuration strucrture
#pragma pack(4)
typedef struct {
    uint16_t dataCRC;   /// CRC of user configuration structure CRC-16
    uint16_t dataSize;  /// Size of the user configuration structure 

    // leverarm
    float pri_lever_arm[3];
    float vrp_lever_arm[3];
    float user_lever_arm[3];
    float rotation_rbv[3];
    
    uint32_t ins_odometer_enable;

} UserConfigurationStruct;
#pragma pack()

typedef enum {
    USER_PRI_LEVER_ARM_BX       = 1,
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
    
    USER_ODOMETER_ENABLE        ,
    
    USER_MAX_PARAM
} UserConfigParamNumber;


BOOL SaveUserConfig(void);
BOOL RestoreDefaultUserConfig(void);
void user_configure_init(void);


int32_t get_user_register(uint32_t param_id, uint8_t* data);
int32_t set_user_register(uint32_t param_id, uint8_t* data);


// api: get/set user parameter
void get_pri_lever_arm(float32_t data[]);
void get_vrp_lever_arm(float32_t data[]);
void get_user_lever_arm(float32_t data[]);
void get_rotation_rbv(float32_t data[]);

uint32_t get_ins_odometer_enable(void);


#endif /* _USER_CONFIG_H */
