#include <string.h>
#include "rtklibapi.h"
#include "fifo_buffer.h"
#include "share_memory.h"
#ifdef RTK330LA
#include "led.h"
#endif
#if GT_PACKET == 1
#include "m7_ipc.h"
#endif

static const cfg_t cfg_default = {
    "",                              /* product number*/
    "",                              /* serial number*/
    "10208",                         /* firmware version*/
    "",                              /* system priority */
    0,                               /* position engine mode: 1:SPP, 2:RTD, 4:RTK fixed, 5:RTK float */
    1,                               /* turn on/off ins aiding rtk */
    0,                               /* wide-lane fixing enabled*/
    0,                               /* glonass ambiguity fixing enabled*/
    0,                               /* exclude abnormal gnss sol*/
    1,                               /* RTK integrity enabled*/
    0,                               /* RTK integrity enabled*/
    6,                               /* min satellite */
    1,                               /* gnss solution rate */
    10.0,                            /* max hdop */
    300.0,                           /* max correction age */
    50.0,                            /* max horizontal position jump threhold(m) */
    2.0,                             /* max vertical position jump threhold(m) */
    50.0,                            /* max horizontal velocity threhold(m/s)  */
    2.0,                             /* max vertical velocity threhold(m/s)  */
    0.5,                             /* horizontal position alert limit(m)  */
    1.0,                             /* vertical position alert limit(m)  */
    0.5,                             /* lateral position alert limit(m)  */
    0.5,                             /* longitu position alert limit(m)  */
    0.5,                             /* horizontal velocity alert limit(m/s)*/
    1.0,                             /* vertical velocity alert limit(m/s)*/
    0.5,                             /* lateral velocity alert limit(m/s) */
    1.0,                             /* longitu velocity alert limit(m/s) */
    0.0,                             /* roll alert limit(deg)*/
    0.0,                             /* pitch alert limit(deg)*/
    0.0,                             /* heading alert limit(deg)*/
};

static gnss_insaid_sol_t insaid_t = {0};
static gnss_sol_t gnss_sol = { 0 };
static gnss_out_t gnss_out = { 0 };

#define RTCM_BUFF_LEN   (1024 * 4)
static uint8_t gnss_rtcm_buff[RTCM_BUFF_LEN];

#if GT_PACKET == 1
custom_data_t gnss_time_ipc =
{
    .clientId = 0x01u
};
#endif

static void copy_ins_sol(const insaid_solution_t* ins_data, gnss_insaid_sol_t* insaidsol)
{
    insaidsol->week = ins_data->gps_week;
    insaidsol->tow = (double)ins_data->gps_millisecs * 0.001;

    insaidsol->ins_status = (uint8_t)ins_data->ins_status;
    insaidsol->pos_type = (uint8_t)ins_data->pos_type;

    insaidsol->latitude = ins_data->latitude;
    insaidsol->longitude = ins_data->longitude;
    insaidsol->height = ins_data->height;

    insaidsol->north_velocity = ins_data->north_velocity;
    insaidsol->east_velocity = ins_data->east_velocity;
    insaidsol->up_velocity = ins_data->up_velocity;

    insaidsol->roll = ins_data->roll;
    insaidsol->pitch = ins_data->pitch;
    insaidsol->heading = ins_data->azimuth;

    insaidsol->latitude_std = ins_data->latitude_std;
    insaidsol->longitude_std = ins_data->longitude_std;
    insaidsol->height_std = ins_data->altitude_std;

    insaidsol->north_velocity_std = ins_data->north_velocity_std;
    insaidsol->east_velocity_std = ins_data->east_velocity_std;
    insaidsol->up_velocity_std = ins_data->up_velocity_std;

    insaidsol->roll_std = ins_data->roll_std;
    insaidsol->pitch_std = ins_data->pitch_std;
    insaidsol->heading_std = ins_data->azimuth_std;
}

static void copy_gnss_sol(gnss_sol_t* gs_t, gnss_out_t* go_t, gnss_solution_t* gnss_data)
{
    //gnss_data->alo_time = gs_t->alo_time;

    gnss_data->gps_week      = gs_t->week;
    gnss_data->gps_millisecs = (uint32_t)(gs_t->tow * 1000);
    gnss_data->gnss_fix_type = gs_t->fixType;
    gnss_data->vel_mode      = gs_t->fixType;

    gnss_data->nsat_view = gs_t->nsat_view;
    gnss_data->nsat_use  = gs_t->nsat_use;

    gnss_data->latitude  = gs_t->latitude;
    gnss_data->longitude = gs_t->longitude;
    gnss_data->height    = gs_t->height;
    gnss_data->geo_sep   = gs_t->geo_sep;

    // gnss_data->pos_ecef[0] = 
    // gnss_data->pos_ecef[1] = 
    // gnss_data->pos_ecef[2] = 

    gnss_data->vel_ned[0] = gs_t->north_vel;
    gnss_data->vel_ned[1] = gs_t->east_vel;
    gnss_data->vel_ned[2] = -gs_t->up_vel;

    // gnss_data->heading = 

    // gnss_data->dops[0] = 
    gnss_data->dops[1] = gs_t->PDOP;
    gnss_data->dops[2] = gs_t->HDOP;
    gnss_data->dops[3] = gs_t->VDOP;
    gnss_data->dops[4] = gs_t->TDOP;
    
    gnss_data->sol_age = gs_t->sol_age;

    gnss_data->std_lat = gs_t->latitude_std;
    gnss_data->std_lon = gs_t->longitude_std;
    gnss_data->std_hgt = gs_t->height_std;
    gnss_data->std_vn  = gs_t->north_vel_std;
    gnss_data->std_ve  = gs_t->east_vel_std;
    gnss_data->std_vd  = gs_t->up_vel_std;

    gnss_data->hor_pos_pl = gs_t->hor_pos_pl;
    gnss_data->ver_pos_pl = gs_t->ver_pos_pl;
    gnss_data->hor_vel_pl = gs_t->hor_vel_pl;
    gnss_data->ver_vel_pl = gs_t->ver_vel_pl;

    gnss_data->pos_pl_status = gs_t->pos_pl_status;
    gnss_data->vel_pl_status = gs_t->vel_pl_status;

    memcpy(gnss_data->nema_gga_buff, go_t->gga, sizeof(char)*NMEA_S_LEN);
    memcpy(gnss_data->nema_zda_buff, go_t->zda, sizeof(char)*NMEA_S_LEN);

    gnss_data->fw_version = gs_t->fwver;
    gnss_data->sta_temperature = 0.0;
    gnss_data->ins_aid = cfg_default.ins_aid;

    gnss_data->gnss_update = 1;
}

/**
 * @brief 
 * 
 */
void gnss_rtk_task(void)
{
    uint16_t rtcm_len = 0U;
    uint8_t rtksol_retval = RTKSOL_RETV_NONE;

    rtkcore_initfromconfig(&cfg_default);
    gnss_data_to_ins->ins_aid = cfg_default.ins_aid;

    rtkcore_set_output(&gnss_sol, &gnss_out);

    for (;;) {
        rtcm_len = fifo_get(fifo_gps_uart_rover, gnss_rtcm_buff, RTCM_BUFF_LEN);
        if (rtcm_len > 0) {
            rtksol_retval = rtkcore_input_rover_rtcm(gnss_rtcm_buff, rtcm_len);
            if (rtksol_retval == RTKSOL_RETV_SOL_VALID) {
                copy_gnss_sol(&gnss_sol, &gnss_out, gnss_data_to_ins); // output rtk results

#if GT_PACKET == 1
    gnss_time_ipc.gnss_time.week = gnss_sol.week;
    gnss_time_ipc.gnss_time.time_now = (uint32_t)(gnss_sol.tow * 1000);
    send_ipc_pipe_data(&gnss_time_ipc);
#endif
          
#ifdef RTK330LA
    bsp_led_toogle(1);
#endif
            }
        }

        rtcm_len = fifo_get(fifo_user_uart_base, gnss_rtcm_buff, RTCM_BUFF_LEN);
        if (rtcm_len > 0) {
            rtksol_retval = rtkcore_input_base_rtcm(gnss_rtcm_buff, rtcm_len);
            if (rtksol_retval == RTKSOL_RETV_BASE_VALID) {

#ifdef RTK330LA
    bsp_led_toogle(2);
#endif
            }
        }

        if (ins_data_to_gnss->ins_update == 1) { // 1Hz
            copy_ins_sol((const insaid_solution_t*)ins_data_to_gnss, &insaid_t);
            
            rtkcore_add_inssol(&insaid_t);
            ins_data_to_gnss->ins_update = 0;
        }
    }
}
