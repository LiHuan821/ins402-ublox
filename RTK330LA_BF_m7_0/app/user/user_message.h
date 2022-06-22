/** ***************************************************************************
 * @file   user_message.h
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

#ifndef _USER_MESSAGE_H
#define _USER_MESSAGE_H

#include <stdint.h>

#include "constants.h"
#include "ucb_packet.h"
#include "constants.h"


// total size of user packet structure should not exceed 255 bytes
#pragma pack(1)

// example of user payload structure
typedef struct {
    uint32_t   paramNum;                                             // parameter number in parameters structure   (little endian)
    uint8_t    parameter[64];                                        // up to 30 64-bit parameters  (little endian)
} userParamPayload;

#pragma pack()

typedef enum
{
    USR_IN_NONE = 0,
    USR_IN_PING,
    USR_IN_GET_VERSION,
    USR_IN_UPDATE_PARAM,
    USR_IN_SAVE_CONFIG,
    USR_IN_GET_ALL,
    USR_IN_RESTORE_DEFAULTS,
    USR_IN_GET_BLOCK,
    USR_IN_UPDATE_BLOCK,
    USR_IN_MAX,
} UserInPacketType;

// User output packet codes, change at will
typedef enum {
    USR_OUT_NONE  = 0,
    USR_OUT_STRSS,
    USR_OUT_SCALED1,
    USR_OUT_INSPVA,
    USR_OUT_STD1,
    USR_OUT_GNSS,
    USR_OUT_STATUS,
    USR_OUT_FM,
    USR_OUT_MAX
} UserOutPacketType;

//  user out packet struct
#pragma pack(1)

typedef struct {
    uint32_t  week;
    double    timeOfWeek;
    uint8_t   safety_info;
    uint8_t   version_flags;
    uint32_t firmware_version;
    uint8_t safe_state;
    uint8_t sis_error_code;
    uint8_t hw_error_code;
    uint8_t pps_status;
    uint8_t time_validity;
    uint32_t constellation_alarm_mask;
    uint32_t monitor_alarm_mask;
    uint32_t gnss_constellation_mask;
    uint32_t gnss_multi_frequency_constellation_mask;
} out_st_rss_struct;

// payload structure of alternative IMU data message
typedef struct {
    uint32_t  week;
    double    timeOfWeek;
    float     accel_g[3];
    float     rate_dps[3];
} out_s1_struct;

// payload structure of INSPVA data message
typedef struct {
    uint32_t week;
    double   timeOfWeek;
    uint8_t  insStatus;
    uint8_t  insPositionType;
    int32_t  latitude;
    int32_t  longitude;
    float    height;
    int16_t  north_vel;
    int16_t  east_vel;
    int16_t  up_vel;
    int16_t  roll;
    int16_t  pitch;
    int16_t  heading;
} out_iN_struct;

// payload structure of std1 data message
typedef struct {
    uint32_t week;
    double   timeOfWeek;
    int16_t  latitude_std;
    int16_t  longitude_std;
    int16_t  height_std;
    int16_t  north_vel_std;
    int16_t  east_vel_std;
    int16_t  up_vel_std;
    int16_t  roll_std;
    int16_t  pitch_std;
    int16_t  heading_std;
} out_d1_struct;

// payload structure of gnss data message
typedef struct {
    uint32_t week;
    double   timeOfWeek;
    uint8_t  positionMode;
    int32_t  latitude;
    int32_t  longitude;
    float    height;
    uint8_t  numberOfSVs;
    float    hdop;
    uint16_t diffage;
    int16_t  north_vel;
    int16_t  east_vel;
    int16_t  up_vel;
} out_gN_struct;

// payload structure of status data message
typedef struct {
    /* IMU */
    uint32_t imu_temp_status : 1;  // imu temperature status
    uint32_t imu_acce_status : 1;  // imu accelerometer status
    uint32_t imu_gyro_status : 1;  // imu gyro status

    uint32_t imu_sensor_status1 : 1;  // imu sensor (#1, #2， #3) status
    uint32_t imu_sensor_status2 : 1;  // imu sensor (#1, #2， #3) status
    uint32_t imu_sensor_status3 : 1;  // imu sensor (#1, #2， #3) status
    uint32_t imu_overall_status : 1;
    /* GNSS status */
    uint32_t gnss_data_status : 1;
    uint32_t gnss_signal_status : 1;
    /* operation */
    uint32_t power : 1;  // for the whole device, any component has no power then 0
    uint32_t MCU_status : 1;
    uint32_t pps_status : 1;
    uint32_t rexerved : 20;
} status_bit_t;
typedef struct {
    uint32_t week;
    double timeOfWeek;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    status_bit_t status_bit;
    float imu_temp;
    float mcu_temp;
} status_t;

typedef struct {
    uint32_t  week;
    double    timeOfWeek;
    int32_t   s1_data[7];
    int32_t   s2_data[7];
    int32_t   s3_data[7];
    uint16_t  index;
} out_fm_struct;

#pragma pack()

extern status_t g_status;

int checkUserInPacketType(uint16_t receivedCode);
int checkUserOutPacketType(uint16_t receivedCode);
void userPacketTypeToBytes(uint8_t bytes[]);

int  HandleUserInputPacket (UcbPacketStruct *ptrUcbPacket);


#endif /* _USER_MESSAGE_H */
