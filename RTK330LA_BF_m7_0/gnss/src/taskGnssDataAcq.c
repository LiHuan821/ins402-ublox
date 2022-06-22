/** ***************************************************************************
 * @file taskGps.c handle GPS data, make sure the GPS handling function gets
 *  called on a regular basis
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *****************************************************************************/
/*******************************************************************************
Copyright 2018 ACEINNA, INC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
* HISTORY***********************************************************************
* 17/10/2019  |                                             | Daich
* Description: new line: update_fifo_in(UART_GPS);
               remove gpsUartBuf etc.
* 06/01/2020  |                                             | Daich
* Description: parse bt data
*******************************************************************************/
#include "taskRTK.h"
#include "string.h"
#include "utils.h"
#include "time_ref.h"
#include "user_message.h"
#include "taskGnssDataAcq.h"
#include "m7_ipc.h"
#include "share_memory.h"
#include "rtcm.h"
#include "gpsAPI.h"

#define BASE        1
#define ROVER       0
#define GNSS_MSR    
#if 1
//GpsData_t gGpsData = {0};
GNSS_MSR gnss_rtcm_t gRtcmData;
gnss_rtcm_t *rtcm = &gRtcmData;
//gnss_rtcm_t *rtcm;

uint8_t stnID = 0;
/* output GGA to COM2 to request RTK data */
uint8_t gnss_signal_flag = 0;
extern GpsData_t gGpsData;
RTK_DTCM_DATA extern GpsData_t *gGpsDataPtr;
static gtime_t obstimecpy;
volatile mcu_time_base_t g_obs_rcv_time;

char rtk_start_flag = 0;
char rtk_end_flag = 0;

extern GnssData g_gnss_data;

static uint64_t gnss_rcv_data_timecount = 0;

GpsData_t* g_ptr_gnss_data = &gGpsData;
#if GT_PACKET == 1
custom_data_t gnss_time_ipc =
{
    .clientId = 0x01u
};
#endif
RTK_RAM_CODE void _handleGpsMessages(tU8 *RtcmBuff, tU32 length)
{
    int pos = 0, tn;
    double tbase;
    gnss_rtcm_t *rtcm = &gGpsData.rtcm;
    obs_t *obs = rtcm->obs + stnID;
    __attribute__((unused)) rtcm_t *rcv = rtcm->rcv + stnID;

    int8_t ret_val = 0;
    static mcu_time_base_t base_rcv_time = {0};
    static char ref_copy_flag = 0;
    while (length)
    {
        length--;
        ret_val = input_rtcm3(RtcmBuff[pos++], stnID, rtcm);

        if (stnID == BASE) {
            if (ret_val != 0) {
                rtcm->rcv[BASE].time = rtcm->rcv[ROVER].time;

                tbase = timediff(rtcm->obs[BASE].time, gGpsDataPtr->ref.time);
                tn = gGpsDataPtr->ref.n - rtcm->obs[BASE].n;

                if (tn <= 5 || tbase > 5.0) {
                  if(rtk_end_flag == 1)
                  {
                      gGpsDataPtr->ref = *(rtcm->obs + BASE);
                  }
                  else
                  {
                      ref_copy_flag = 1;
                  }
                    
                }
            }
            if (ret_val == 1) {
                base_rcv_time = g_MCU_time;
            }
        }
        
        if(rtk_end_flag == 1 && ref_copy_flag == 1)
        {
            gGpsDataPtr->ref = *(rtcm->obs + BASE);
            ref_copy_flag = 0;
        }
#if 1
        if (stnID == ROVER) {
            if (ret_val == 1) {
                gnss_rcv_data_timecount = platform_get_time_counter();
                

                if (obs->pos[0] == 0.0 || obs->pos[1] == 0.0 || obs->pos[2] == 0.0) {
                    // TOD
                } else {
                    if ((obstimecpy.sec == obs->time.sec && obstimecpy.time == obs->time.time) || obs->n < 4) {
                        gnss_signal_flag = 0;
                    } else {
                        // if (obs->time.time != obstimecpy.time) {
                        // }
                        g_obs_rcv_time = g_MCU_time;
                        obstimecpy = obs->time;
                        if (obs->n >= 4) {
                            gnss_signal_flag = 1;
                        }
                    }

                    // double tow = time2gpst(rcv->time, NULL);
                    if ((gnss_signal_flag)) {
                        if ((obs->time.sec * 1000) < 5)     //1Hz
                        // if(fmod(obs->time.sec+0.001,0.2) < 0.005)//5Hz
                        // if(fmod(obs->time.sec+0.001,0.5) < 0.005)//2Hz
                        {
#if GT_PACKET == 1
                            int week;
                            double gt_time_now = time2gpst(obs->time, &week);
                            gnss_time_ipc.gnss_time.week = week;
                            gnss_time_ipc.gnss_time.time_now = gt_time_now * 1000;
                            send_ipc_pipe_data(&gnss_time_ipc);
#endif
                            gGpsDataPtr->rov = *obs;
                            gGpsDataPtr->nav = rtcm->nav;
                            rtk_start_flag = 1;
                        }
                    }
                }
            }
        }
#endif
    }
}
/* end _handleGpsMessages */

uint32_t rover_lose_second = 0;
#define RTCM_BUFF_LEN   (1024 * 4)
uint8_t gnss_rtcm_buff[RTCM_BUFF_LEN];
extern fifo_type uart6_rx_fifo;
fifo_type fifo_user_uart;


RTK_RAM_CODE void process_gnss_data(void)
{
    uint16_t rtcm_len = 0U;

    // rtcm_len = fifo_get(&uart6_rx_fifo, gnss_rtcm_buff, RTCM_BUFF_LEN);
    rtcm_len = fifo_get(fifo_gps_uart_rover, gnss_rtcm_buff, RTCM_BUFF_LEN);
    
    if (rtcm_len > 0) {
        rover_lose_second = 0;
        // uart7_write_dma(gnss_rtcm_buff, rtcm_len);
        stnID = ROVER;
        _handleGpsMessages(gnss_rtcm_buff, rtcm_len);
        // g_status.status_bit.gnss_data_status = 1;
    }
    if(rover_lose_second > 100)
    {
        // g_status.status_bit.gnss_data_status = 0;
    }
#ifdef STA9100_RESET
    if (rover_lose_second > 1000) {
        bsp_sta9100_reset();
    }
#endif


    rtcm_len = fifo_get(fifo_user_uart_base, gnss_rtcm_buff, RTCM_BUFF_LEN);
    // rtcm_len = fifo_get(&fifo_user_uart, gnss_rtcm_buff, RTCM_BUFF_LEN);
    if (rtcm_len > 0) {
        stnID = BASE;
        _handleGpsMessages(gnss_rtcm_buff, rtcm_len);
#if 1
        //uart7_write_dma(gnss_rtcm_buff, rtcm_len);
#endif
    }

    uint64_t count = platform_get_time_counter();
    if (count - gnss_rcv_data_timecount >= 1000) {
        // g_status.status_bit.gnss_signal_status = 0;
    }
}

RTK_RAM_CODE time_t get_obs_time()
{
    return obstimecpy.time;
}

RTK_RAM_CODE uint8_t is_gnss_signal_valid(void)
{
    return gnss_signal_flag;
}

#endif
