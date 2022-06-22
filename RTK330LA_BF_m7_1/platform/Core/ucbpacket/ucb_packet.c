#include "ucb_packet.h"
#include "ucb_packet_struct.h"
#include "gnss.h"
#include "ins_interface_API.h"
#include "crc16.h"
#include "fw_ins.h"
#include <stdio.h>
#include <math.h>
#include "share_memory.h"

void ucb_fill_packet_struct(UcbPacketStruct *const ptr_ucb, uint16_t const packet_type)
{
    uint16_t crc;

    if (ptr_ucb != NULL) {
        // packet head
        ptr_ucb->sync_MSB = 0x55;
        ptr_ucb->sync_LSB = 0x55;

        // packet type
        ptr_ucb->code_MSB = (packet_type >> 8 ) & 0xff;
        ptr_ucb->code_LSB = packet_type & 0xff;

        // packet len and payload
        // ptr_ucb->payloadLength = datalen;
        // memcpy(ptr_ucb->payload, ptr_data, datalen);

        // packet crc
        crc = CalculateCRC((uint8_t *)&ptr_ucb->code_MSB, ptr_ucb->payloadLength + 3);
        ptr_ucb->payload[ptr_ucb->payloadLength + 1] = (crc >> 8) & 0xff;
        ptr_ucb->payload[ptr_ucb->payloadLength] = crc & 0xff;
    }
}

void ucb_fill_s1_packet(UcbPacketStruct *const ptr_ucb, ImuData *const ptr_imu_sol)
{
    ucb_s1_struct *pld = (ucb_s1_struct *)ptr_ucb->payload;
    ptr_ucb->payloadLength = sizeof(ucb_s1_struct);
    
    pld->week       =   ptr_imu_sol->week;
    pld->timeOfWeek =   ptr_imu_sol->timestamp;

    // double* acc = get_imu_acc();
    // double* gyro = get_imu_gyro();

    // pld->accel_g[0]  =   *acc++;
    // pld->accel_g[1]  =   *acc++;
    // pld->accel_g[2]  =   *acc;
    // pld->rate_dps[0] =   (*gyro++) * R2D;
    // pld->rate_dps[1] =   (*gyro++) * R2D;
    // pld->rate_dps[2] =   (*gyro) * R2D;
    pld->accel_g[0]  =   ptr_imu_sol->Accx;
    pld->accel_g[1]  =   ptr_imu_sol->Accy;
    pld->accel_g[2]  =   ptr_imu_sol->Accz;
    pld->rate_dps[0] =   ptr_imu_sol->Gyrox;
    pld->rate_dps[1] =   ptr_imu_sol->Gyroy;
    pld->rate_dps[2] =   ptr_imu_sol->Gyroz;    
    ucb_fill_packet_struct(ptr_ucb, 0x7331);
}

void ucb_fill_gN_packet(UcbPacketStruct *const ptr_ucb, gnss_solution_t *const ptr_bestgnss_sol)
{
    ucb_gN_struct *pld = (ucb_gN_struct *)ptr_ucb->payload;
    ptr_ucb->payloadLength = sizeof(ucb_gN_struct);

    pld->week = ptr_bestgnss_sol->gps_week;
    pld->timeOfWeek = (double)ptr_bestgnss_sol->gps_millisecs / 1000;
    pld->positionMode = ptr_bestgnss_sol->gnss_fix_type;
    pld->latitude = (int32_t) (ptr_bestgnss_sol->latitude * R2D/180*2147483648.0);
    pld->longitude = (int32_t) (ptr_bestgnss_sol->longitude * R2D/180*2147483648.0);

    pld->height = (float) ptr_bestgnss_sol->height;
    pld->numberOfSVs = ptr_bestgnss_sol->nsat_use;
    pld->hdop = ptr_bestgnss_sol->dops[2];
    pld->diffage = (uint16_t) ptr_bestgnss_sol->sol_age;
    pld->north_vel = (int16_t) (ptr_bestgnss_sol->vel_ned[0] * 100);
    pld->east_vel = (int16_t) (ptr_bestgnss_sol->vel_ned[1] * 100);
    pld->up_vel = (int16_t) (-ptr_bestgnss_sol->vel_ned[2] * 100);
    pld->latitude_std = (int16_t) (ptr_bestgnss_sol->std_lat * 1000);
    pld->longitude_std = (int16_t) (ptr_bestgnss_sol->std_lon * 1000);
    pld->height_std = (int16_t) (ptr_bestgnss_sol->std_hgt * 1000);

    ucb_fill_packet_struct(ptr_ucb, 0x674E);
}

void ucb_fill_iN_packet(UcbPacketStruct *const ptr_ucb, ins_solution_t *const ptr_ins_sol)
{
    ucb_iN_struct *pld = (ucb_iN_struct *)ptr_ucb->payload;
    ptr_ucb->payloadLength = sizeof(ucb_iN_struct);

    pld->week = ptr_ins_sol->gps_week;
    pld->timeOfWeek = (double)ptr_ins_sol->gps_millisecs / 1000;
    pld->insStatus = ptr_ins_sol->ins_status;
    pld->insPositionType = ptr_ins_sol->pos_type;
    pld->latitude = (int32_t)(ptr_ins_sol->latitude / 180 * 2147483648.0);
    pld->longitude = (int32_t)(ptr_ins_sol->longitude / 180 * 2147483648.0);
    pld->height = (float)ptr_ins_sol->height;
    pld->north_vel = (int16_t)(ptr_ins_sol->north_velocity * 100);
    pld->east_vel = (int16_t)(ptr_ins_sol->east_velocity * 100);
    pld->up_vel = (int16_t)(ptr_ins_sol->up_velocity * 100);
    pld->roll = (int16_t)(ptr_ins_sol->roll * 100);
    pld->pitch = (int16_t)(ptr_ins_sol->pitch * 100);
    if (ptr_ins_sol->azimuth > 180) {
        pld->heading = (int16_t)((ptr_ins_sol->azimuth - 360) * 100);
    } else {
        pld->heading = (int16_t)(ptr_ins_sol->azimuth * 100);
    }

    double todsec = pld->timeOfWeek - floor(pld->timeOfWeek + 0.5);
    if(fabs(todsec) < 0.005)
    {
        //memcpy(ins_data_to_gnss, ptr_ins_sol, sizeof(ins_solution_t));
        ins_data_to_gnss->gps_week          = ptr_ins_sol->gps_week;
        ins_data_to_gnss->gps_millisecs     = ptr_ins_sol->gps_millisecs;
        ins_data_to_gnss->ins_status        = ptr_ins_sol->ins_status;
        ins_data_to_gnss->pos_type          = ptr_ins_sol->pos_type;
        ins_data_to_gnss->latitude          = ptr_ins_sol->latitude;
        ins_data_to_gnss->longitude         = ptr_ins_sol->longitude;
        ins_data_to_gnss->height            = ptr_ins_sol->height;
        ins_data_to_gnss->north_velocity    = ptr_ins_sol->north_velocity;
        ins_data_to_gnss->east_velocity     = ptr_ins_sol->east_velocity;
        ins_data_to_gnss->up_velocity       = ptr_ins_sol->up_velocity;
        ins_data_to_gnss->roll              = ptr_ins_sol->roll;
        ins_data_to_gnss->pitch             = ptr_ins_sol->pitch;
        ins_data_to_gnss->azimuth           = ptr_ins_sol->azimuth;
        ins_data_to_gnss->latitude_std      = ptr_ins_sol->latitude_std;
        ins_data_to_gnss->longitude_std     = ptr_ins_sol->longitude_std;
        ins_data_to_gnss->altitude_std       = ptr_ins_sol->altitude_std;
        ins_data_to_gnss->north_velocity_std = ptr_ins_sol->north_velocity_std;
        ins_data_to_gnss->east_velocity_std  = ptr_ins_sol->east_velocity_std;
        ins_data_to_gnss->up_velocity_std    = ptr_ins_sol->up_velocity_std;
        ins_data_to_gnss->roll_std           = ptr_ins_sol->roll_std;
        ins_data_to_gnss->pitch_std          = ptr_ins_sol->pitch_std;
        ins_data_to_gnss->azimuth_std        = ptr_ins_sol->azimuth_std;
        ins_data_to_gnss->ins_update = 1;
    }
    ucb_fill_packet_struct(ptr_ucb, 0x694E);
}

void ucb_fill_d1_packet(UcbPacketStruct *const ptr_ucb, ins_solution_t *const ptr_ins_sol)
{
    ucb_d1_struct *pld = (ucb_d1_struct *)ptr_ucb->payload;
    ptr_ucb->payloadLength = sizeof(ucb_d1_struct);

    pld->week = ptr_ins_sol->gps_week;
    pld->timeOfWeek = (double)ptr_ins_sol->gps_millisecs / 1000;
    pld->latitude_std = (int16_t)(ptr_ins_sol->latitude_std * 100);
    pld->longitude_std = (int16_t)(ptr_ins_sol->longitude_std * 100);
    pld->height_std = (int16_t)(ptr_ins_sol->altitude_std * 100);
    pld->north_vel_std = (int16_t)(ptr_ins_sol->north_velocity_std * 100);
    pld->east_vel_std = (int16_t)(ptr_ins_sol->east_velocity_std * 100);
    pld->up_vel_std = (int16_t)(ptr_ins_sol->up_velocity_std * 100);
    pld->roll_std = (int16_t)(ptr_ins_sol->roll_std * 100);
    pld->pitch_std = (int16_t)(ptr_ins_sol->pitch_std * 100);
    pld->heading_std = (int16_t)(ptr_ins_sol->azimuth_std * 100);

    ucb_fill_packet_struct(ptr_ucb, 0x6431);
}

void ucb_fill_o1_packet(UcbPacketStruct *const ptr_ucb)
{
    ucb_o1_struct *pld = (ucb_o1_struct *)ptr_ucb->payload;
    ptr_ucb->payloadLength  = sizeof(ucb_o1_struct);

    pld->gps_week           =   get_odo_gps_week();
    pld->gps_millisecs      =   (uint32_t)(get_odo_gps_timestamp() * 1000);
    pld->mode               =   get_odo_mode();
    pld->speed              =   get_odo_speed();
    pld->fwd                =   get_odo_fwd();
    pld->wheel_tick         =   get_odo_wheel_tick();

    ucb_fill_packet_struct(ptr_ucb, 0x6F31);
}
#if GT_PACKET == 1
void ucb_fill_GT_packet(UcbPacketStruct *const ptr_ucb, ucb_GT_struct *const gnss_time)
{
    ptr_ucb->payloadLength = sizeof(ucb_GT_struct);

    ptr_ucb->payload[0] = (gnss_time->time_now >> 24) & 0xff;
    ptr_ucb->payload[1] = (gnss_time->time_now >> 16) & 0xff;
    ptr_ucb->payload[2] = (gnss_time->time_now >> 8) & 0xff;
    ptr_ucb->payload[3] = (gnss_time->time_now >> 0) & 0xff;

    ptr_ucb->payload[4] = (gnss_time->week >> 8) & 0xff;
    ptr_ucb->payload[5] = (gnss_time->week >> 0) & 0xff;

    
    ucb_fill_packet_struct(ptr_ucb, 0x4754);
}
#endif
