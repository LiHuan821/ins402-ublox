/** ***************************************************************************
 * @file   user_config.c
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 ******************************************************************************/
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

#include "string.h"
#include "app_version.h"
#include "eepromAPI.h"
#include "user_config.h"
#include "typedefs.h"
#include "crc16.h"
#include "ins_interface_API.h"
#include <stdlib.h>
#include "fw_ins.h"

// Default user configuration structure
// Do Not remove - just add extra parameters if needed
// Change default settings  if desired
const UserConfigurationStruct gDefaultUserConfig = {
    .dataCRC             =  0,
    .dataSize            =  sizeof(UserConfigurationStruct),

    // add default parameter values here, if desired
    .pri_lever_arm[0]       = 0.0,
    .pri_lever_arm[1]       = 0.0,
    .pri_lever_arm[2]       = 0.0,
    .vrp_lever_arm[0]       = 0.0,
    .vrp_lever_arm[1]       = 0.0,
    .vrp_lever_arm[2]       = 0.0,
    .user_lever_arm[0]      = 0.0,
    .user_lever_arm[1]      = 0.0,
    .user_lever_arm[2]      = 0.0,
    .rotation_rbv[0]        = 0.0,
    .rotation_rbv[1]        = 0.0,
    .rotation_rbv[2]        = 0.0,
    
    .ins_odometer_enable    = 1,
};

static UserConfigurationStruct gUserConfiguration;
BOOL configValid = FALSE;

/** ***************************************************************************
 * @name SaveUserConfig - saving of user configuration structure on the 
 *       predefined flash sector
 * @brief
 *
 * @param [in] N/A
 * @retval error (0), no error (1)
 ******************************************************************************/
BOOL  SaveUserConfig(void)
{
    uint16_t size;
    BOOL status;

    size   = (uint16_t)sizeof(UserConfigurationStruct);
    status = EEPROM_SaveUserConfiguration((uint8_t *)&gUserConfiguration, size);

    if(status){
        return TRUE; 
    }

    return FALSE;
}

/** ***************************************************************************
 * @name RestoreDefaultUserConfig - restore user configuration structure from 
 *       the default one
 * @brief
 *
 * @param [in] N/A
 * @retval error (0), no error (1)
 ******************************************************************************/
BOOL RestoreDefaultUserConfig(void)
{
    BOOL valid = TRUE;

    memcpy((void*)&gUserConfiguration, (void*)&gDefaultUserConfig, sizeof(UserConfigurationStruct));

    if(!SaveUserConfig()){
        valid = FALSE;
    }
    return valid;
}

/** ***************************************************************************
 * @name user_configure_init
 * @brief init user configuration structure 
 * @param N/A
 * @retval N/A
 ******************************************************************************/
void user_configure_init(void)
{
    uint16_t size = sizeof(gUserConfiguration);

    // Validate checksum of user configuration structure
    configValid = EEPROM_ValidateUserConfig(&size);

    if (configValid == TRUE) {
        // Here we have validated User configuration image.
        // Load it from eeprom into ram on top of the default configuration
        EEPROM_LoadUserConfig((uint8_t*)&gUserConfiguration, &size);
    } else {
        RestoreDefaultUserConfig();
    }
    
}

int32_t get_user_register(uint32_t param_id, uint8_t* data)
{
    int32_t retval = -1;

    if (param_id <= (sizeof(UserConfigurationStruct) / 4)) {
        memcpy(data, (uint8_t*)&gUserConfiguration + param_id * 4, 4);
        retval = 0;
    }
    return retval;
}

int32_t set_user_register(uint32_t param_id, uint8_t* data)
{
    int32_t retval = -1;

    if (param_id <= (sizeof(UserConfigurationStruct) / 4)) {
        memcpy((uint8_t*)&gUserConfiguration + param_id * 4, data, 4);
        ins_init();
        retval = 0;
    }
    return retval;
}

// api for get/set parameter of configuration
/*******************************************
 * @brief 
 * 
 * @param data ==
********************************************/
void get_pri_lever_arm(float32_t data[])
{
    for (int32_t i = 0; i < 3; i++){
        data[i] = gUserConfiguration.pri_lever_arm[i];
    }
}

/*******************************************
 * @brief 
 * 
 * @param data ==
********************************************/
void get_vrp_lever_arm(float32_t data[])
{
    for (int32_t i = 0; i < 3; i++){
        data[i] = gUserConfiguration.vrp_lever_arm[i];
    }
}

/*******************************************
 * @brief 
 * 
 * @param data ==
********************************************/
void get_user_lever_arm(float32_t data[])
{
    for (int32_t i = 0; i < 3; i++){
        data[i] = gUserConfiguration.user_lever_arm[i];
    }
}

/*******************************************
 * @brief 
 * 
 * @param data ==
********************************************/
void get_rotation_rbv(float32_t data[])
{
    for (int32_t i = 0; i < 3; i++){
        data[i] = gUserConfiguration.rotation_rbv[i];
    }
}

/*******************************************
 * @brief 
 * 
 * @param data ==
********************************************/
uint32_t get_ins_odometer_enable(void)
{
    return gUserConfiguration.ins_odometer_enable;
}
