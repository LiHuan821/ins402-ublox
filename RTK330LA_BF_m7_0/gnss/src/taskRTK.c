/** ***************************************************************************
 * @file taskRTK.c
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
* 16/10/2019  |                                             | Daich
* Description: package some _hal function
* 28/10/2019  |                                             | Daich
* Description: modify the position of "osSemaphoreWait(RTDStartSem, 1000)"
* 06/11/2019  | changed file name back to taskRTK.c         | DW
*******************************************************************************/

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdarg.h"
#include "stdio.h"
#include "time_ref.h"
#include "nav_math.h"
#include "uart.h"
#include "user_message.h"
#include "taskRTK.h"
#include "taskGnssDataAcq.h"
#include "rtcm.h"
#include "share_memory.h"
#include "gpsAPI.h"
#include "app_version.h"

#define GNSS_MSR
#define RTK_INS

gnss_solution_t g_gnss_sol = {0};
gnss_solution_t *g_ptr_gnss_sol = &g_gnss_sol;
ins_sol_t insol = { 0 };
gnss_sol_t gnss_sol = { 0 };
gnss_out_t gnss_out = { 0 };
double lastgnsstime = -1;
GnssData g_gnss_data = {0};
int32_t g_gnss_start_week = -1;
char gga_buff[NMEA_S_LEN] = {0};
char gsa_buff[NMEA_M_LEN] = {0};
char rmc_buff[NMEA_S_LEN] = {0};
char gsv_buff[NMEA_M_LEN] = {0};
char zda_buff[NMEA_S_LEN] = {0};
char uart_buff[256];
char test_buff[256], sol_buff[512];
char nema_update_flag = 0;

GpsData_t gGpsData = {0};
RTK_DTCM_DATA GpsData_t *gGpsDataPtr = &gGpsData;  // TODO:1
RTK_DTCM_DATA epoch_t gRov = {0};
epoch_t gRef = {0}, lastgRov = {0};
nav_t nav_m;
obs_t rov_m; 
obs_t ref_m; 
int gfixID = 0;
extern char rtk_start_flag;
extern char rtk_end_flag;
uint64_t timestamp1, timestamp2;

static void read_ins_sol(double curtime, ins_solution_t *ins_data, ins_sol_t* ins_sol)
{
    int i;
    double insTime, dt = 0.0, blh_pos[3], enu_vel[3];

    insTime = ins_data->gps_millisecs*0.001;
    dt = curtime - insTime;
    if(fabs(dt) < 0.1)
    {
        blh_pos[0] = ins_data->latitude * D2R;
        blh_pos[1] = ins_data->longitude * D2R;
        blh_pos[2] = ins_data->height;
        enu_vel[0] = ins_data->east_velocity;
        enu_vel[1] = ins_data->north_velocity;
        enu_vel[2] = ins_data->up_velocity;
        pos2ecef(blh_pos, ins_sol->pos_xyz);
        enu2ecef(blh_pos, enu_vel, ins_sol->vel_xyz);
        for(i = 0; i < 3; i++)
        {
            ins_sol->pos_xyz[i] += ins_sol->vel_xyz[i]*dt;
        }

        ins_sol->week = (int)ins_data->gps_week;
        ins_sol->tow = ins_data->gps_millisecs * 0.001;
        ins_sol->ins_status = (unsigned char)ins_data->ins_status;
        ins_sol->pos_type = (unsigned char)ins_data->pos_type;
        ins_sol->roll = ins_data->roll;
        ins_sol->pitch = ins_data->pitch;
        ins_sol->heading = ins_data->azimuth;
        if (ins_sol->ins_status > 2 && ins_sol->pos_type > 1)
        {
            ins_sol->available = (unsigned char)1;
        }
    }
    else
    {
        memset(ins_sol, 0, sizeof(ins_sol_t) * 1);
    }
    
    return ;
}

int32_t get_gnss_start_week(void)
{
    return g_gnss_start_week;
}

RTK_RAM_CODE static void copy_rtk_result(gtime_t rov_time, gnss_sol_t *gnss_sol,
    gnss_solution_t *gnss_solution_ptr, GnssData *p_gnss_data)
{
    int i = 0, j = 0;
    double pos_dsf = 1.0, pos_sf0 = 9.0, pos_sig0 = 0.5;

    if (gnss_solution_ptr != NULL)
    {
        gnss_solution_ptr->gps_week = (uint16_t)gnss_sol->week;
        gnss_solution_ptr->gps_tow = (uint32_t)(gnss_sol->tow*1000.0);
        gnss_solution_ptr->gnss_fix_type = (uint8_t)gnss_sol->fixType;
        gnss_solution_ptr->vel_mode  = (uint8_t)gnss_sol->fixType;
        gnss_solution_ptr->num_sats  = (uint8_t)gnss_sol->numSat;
        gnss_solution_ptr->latitude  = (double)gnss_sol->latitude;
        gnss_solution_ptr->longitude = (double)gnss_sol->longitude;
        gnss_solution_ptr->height  = (double)gnss_sol->height;
        gnss_solution_ptr->geo_sep = (double)gnss_sol->geo_sep;

        gnss_solution_ptr->std_lat = (float)gnss_sol->latitude_std;
        gnss_solution_ptr->std_lon = (float)gnss_sol->longitude_std;
        gnss_solution_ptr->std_hgt = (float)gnss_sol->height_std;

        gnss_solution_ptr->vel_ned[0] = (float)gnss_sol->north_vel;
        gnss_solution_ptr->vel_ned[1] = (float)gnss_sol->east_vel;
        gnss_solution_ptr->vel_ned[2] = -(float)gnss_sol->up_vel;
        gnss_solution_ptr->dops[2] = (float)gnss_sol->HDOP;
        gnss_solution_ptr->dops[3] = (float)gnss_sol->VDOP;
        gnss_solution_ptr->dops[4] = (float)gnss_sol->TDOP;
        gnss_solution_ptr->dops[1] = (float)gnss_sol->PDOP;
        gnss_solution_ptr->sol_age = (float)gnss_sol->sol_age;

        gnss_solution_ptr->std_vn = gnss_sol->north_vel_std;
        gnss_solution_ptr->std_ve = gnss_sol->east_vel_std;
        gnss_solution_ptr->std_vd = gnss_sol->up_vel_std;

        gnss_solution_ptr->hor_pos_pl = gnss_sol->hor_pos_pl;
        gnss_solution_ptr->ver_pos_pl = gnss_sol->ver_pos_pl;
        gnss_solution_ptr->hor_vel_pl = gnss_sol->hor_vel_pl;
        gnss_solution_ptr->ver_vel_pl = gnss_sol->ver_vel_pl;
        gnss_solution_ptr->pos_pl_status = gnss_sol->pos_pl_status;
        gnss_solution_ptr->vel_pl_status = gnss_sol->vel_pl_status;
        
    }

    double time_gnss_now = (double)gnss_solution_ptr->gps_tow / 1000;
    if (get_gnss_start_week() != -1) {
        time_gnss_now += (gnss_solution_ptr->gps_week - get_gnss_start_week()) * SECONDS_IN_WEEK;
    }

    double dt2 = time_gnss_now - lastgnsstime;
    if (dt2 > 0) {
        gtime_t gt = gpst2time(gnss_solution_ptr->gps_week, gnss_solution_ptr->gps_tow * 0.001);
        p_gnss_data->week = get_gnss_start_week();
        p_gnss_data->itow = (uint32_t)((time_gnss_now + 0.0001) * 1000);
        p_gnss_data->timestamp = time_gnss_now;
        double dt1 = g_MCU_time.time - gt.time;
        dt1 = dt1 + g_MCU_time.msec * 0.001 - gt.sec;
        p_gnss_data->timestampd = dt1 + p_gnss_data->timestamp;
        p_gnss_data->latitude = gnss_solution_ptr->latitude;
        p_gnss_data->longitude = gnss_solution_ptr->longitude;
        p_gnss_data->altitude = gnss_solution_ptr->height;
        p_gnss_data->latitude_std = gnss_solution_ptr->std_lat;
        p_gnss_data->longitude_std = gnss_solution_ptr->std_lon;
        p_gnss_data->altitude_std = gnss_solution_ptr->std_hgt;
        p_gnss_data->north_velocity = gnss_solution_ptr->vel_ned[0];
        p_gnss_data->east_velocity = gnss_solution_ptr->vel_ned[1];
        p_gnss_data->up_velocity = -gnss_solution_ptr->vel_ned[2];
        p_gnss_data->numSatellites = gnss_solution_ptr->num_sats;
        p_gnss_data->HDOP = gnss_solution_ptr->dops[2];
        p_gnss_data->VDOP = gnss_solution_ptr->dops[3];
        p_gnss_data->TDOP = gnss_solution_ptr->dops[4];
        p_gnss_data->sol_age = gnss_solution_ptr->sol_age;
        p_gnss_data->Mode = gnss_solution_ptr->gnss_fix_type;
        p_gnss_data->gpsFixType = gnss_solution_ptr->gnss_fix_type;
        p_gnss_data->flag = 1;

        // INSADDGNSSDATA(*p_gnss_data);
        lastgnsstime = p_gnss_data->timestamp;
    }

}

RTK_RAM_CODE static void copy_skyview(obs_t *rov, gnss_sol_t *gnss_sol,
                  gnss_solution_t *gnss_solution_ptr)
{
    satellite_struct *pld = gnss_solution_ptr->rov_satellite;
    uint8_t sys = 0;
    int prn = 0;
    gnss_solution_ptr->rov_n = rov->n;

    for (size_t i = 0; i < gnss_solution_ptr->rov_n; i++) {
        // pld -> satelliteId = gRov.vec[i].sat;
        sys = satsys(gRov.vec[i].sat, &prn);
        pld->satelliteId = (uint8_t)prn;

        if (sys == _SYS_GPS_) {
            pld->systemId = 0;
        } else if (sys == _SYS_GLO_) {
            pld->systemId = 1;
        } else if (sys == _SYS_GAL_) {
            pld->systemId = 2;
        } else if (sys == _SYS_QZS_) {
            pld->systemId = 3;
        } else if (sys == _SYS_BDS_) {
            pld->systemId = 4;
        } else if (sys == _SYS_SBS_) {
            pld->systemId = 5;
        } else {
            pld->systemId = 5;
        }
        pld->antennaId = 0;  // 0,1...
        pld->l1cn0 = gRov.obs.data[i].SNR[0] / 4;
        pld->l2cn0 = gRov.obs.data[i].SNR[1] / 4;
        pld->azimuth = gRov.vec[i].azel[0] * R2D;
        pld->elevation = gRov.vec[i].azel[1] * R2D;
        pld->snr = gGpsData.rov.data[i].SNR[0] * 0.25;
        pld++;
    }
}

RTK_RAM_CODE static int copy_gnss_result(obs_t *rov, gnss_sol_t *gnss_sol,
    gnss_solution_t *gnss_solution_ptr, GnssData *p_gnss_data) 
{
    // int fixID = rtk->fixType;
    gtime_t rov_time = rov->time;
    
    copy_rtk_result(rov_time, gnss_sol, gnss_solution_ptr, &g_gnss_data);

    copy_skyview(rov, gnss_sol, gnss_solution_ptr);
    gnss_solution_ptr->gnss_update = 1;
    gnss_solution_ptr->fw_version = gRov.obs.fwver;
    gnss_solution_ptr->sta_temperature = 0.0;
    memcpy(gnss_solution_ptr->nema_gga_buff, gnss_out.gga, sizeof(char)*NMEA_S_LEN);
    memcpy(gnss_solution_ptr->nema_zda_buff, zda_buff, sizeof(char)*NMEA_S_LEN);
    memcpy(gnss_data_to_ins, gnss_solution_ptr, sizeof(gnss_solution_t));
    gnss_data_to_ins->gnss_to_ins_flag = 1;
    return 1;
}

RTK_RAM_CODE static void RTKAlgorithm(void)
{
    // gnss_rtcm_t rtcm;     /* store RTCM data struct for RTK and PPP */
    nav_t *nav = &nav_m;
    obs_t *rov = &rov_m;
    obs_t *ref = &ref_m;
    double curTime = 0.0;

    /* rover data */
    gRov.obs = *rov;
    
    /* base data */
    if (ref->staid == ref->staid_obs)
    {        
        gRef.obs = *ref;
    }

    curTime = time2gpst(rov->time, NULL);
    /*INS solution data*/
    if (ins_data_to_gnss)
    {   
        read_ins_sol(curTime, ins_data_to_gnss, &insol);
    }

    memset(&gnss_out, 0, sizeof(gnss_out_t)*1);      
    timestamp1 =  platform_get_time_counter();

    gfixID = rtk_processor(&gRov, &gRef, &lastgRov, &insol, nav, &gnss_out, &gnss_sol);
    if (gfixID < SOLQ_NONE)
    {
        return;
    }        
    timestamp2 =  platform_get_time_counter();
    sprintf(test_buff, "begin:\nCPU time:%ld\r\n", timestamp2 - timestamp1);
    // UART7_Printf("%s", test_buff);

    // UART7_Printf("%s\r\n", test_buff);
    // UART7_Printf("%s\r\n", test_buff);

    // UART7_Printf("%s\r\n", gga_buff);

    copy_gnss_result(&gRov.obs, &gnss_sol, g_ptr_gnss_sol, &g_gnss_data);

}

RTK_RAM_CODE void rtk_run(void)
{
    process_gnss_data();
    nav_m = gGpsDataPtr->nav;
    rov_m = gGpsDataPtr->rov;
    ref_m = gGpsDataPtr->ref;
    if (rtk_start_flag == 1)
    {
        rtk_start_flag = 0;
        rtk_end_flag = 0;

        timestamp1 = platform_get_time_counter();
        // UART7_Printf("rtk_s:%lld\r\n", timestamp);
        RTKAlgorithm();
        rtk_end_flag = 1;
        timestamp2 =  platform_get_time_counter();
        g_gnss_sol.alo_time = timestamp2 - timestamp1;
        // UART7_Printf("rtk_e:%lld\r\n", timestamp);

        // time2epoch(gpst2utc(gRov.obs.time), ep);
        /* FOR DEBUG, to get running time measure */
    }
}

