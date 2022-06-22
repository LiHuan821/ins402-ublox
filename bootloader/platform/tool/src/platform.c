/** ***************************************************************************
 * @file xbowsp_init.c Initialization for UCB's Comm. and Cal.
 * @Author dan
 * @date   2011-02-09 22:02:39 -0800 (Wed, 09 Feb 2011)
 * @brief  Copyright (c) 2013, 2014 All Rights Reserved.
 * @rev 17479
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * @version
 * DKH 10.02.14 set sensor range based on EEPROM config
 *****************************************************************************/
#include <string.h>
#include <stdint.h>

//*****************************
#include "app_version.h"
#include "platform_version.h"
#include "eeprom.h"

uint32_t cal_get_unit_serialnum(void)
{
    uint32_t sn = 1234567890;
    memcpy(&sn, (uint8_t *)EEPROM_CAL_ADDR1, 4);
    return sn;
}

const uint8_t* cal_get_unit_version(void)
{
    return APP_VERSION_STRING;
}

void platformGetVersionBytes(uint8_t *bytes)
{
    bytes[0] = (uint8_t)VER_MAJOR_NUM;
    bytes[1] = (uint8_t)VER_MINOR_NUM;
    bytes[2] = (uint8_t)VER_PATCH_NUM;
    bytes[3] = (uint8_t)VER_STAGE_NUM;
    bytes[4] = (uint8_t)VER_BUILD_NUM;
}

/*end void initConfigureUnit(void) */
