#include "run_status.h"
#include <string.h>
#include "mcu_temperature.h"
#include "imu330za.h"
#include "gnss.h"
#include "fd_public.h"

run_status_t run_status;


void runstatus_init(void)
{
    memset(&run_status, 0, sizeof(run_status_t));
}

void runstatus_mcu_temperature_update(void)
{
    run_status.mcu_temp = (float)get_mcu_temperature();
    if(run_status.mcu_temp < UNDER_TEMERATURE_THRESHOLD_VALUE) 
    {
      run_status.bitstatus.temperature_under_mcu_flag = 1;
    }
    else
    {
      run_status.bitstatus.temperature_under_mcu_flag = 0;
    }
    
    if(run_status.mcu_temp > OVER_TEMERATURE_THRESHOLD_VALUE) 
    {
      run_status.bitstatus.temperature_over_mcu_flag = 1;
    }
    else
    {
      run_status.bitstatus.temperature_over_mcu_flag = 0;
    }
}

void runstatus_imu_status_update(void)
{
    run_status.imu_temp                 = S2_packet.temp;
    run_status.bitstatus.master_fail        = S2_packet.bitstatus.master_fail;
    run_status.bitstatus.hw_err             = S2_packet.bitstatus.hw_err;
    run_status.bitstatus.sw_err             = S2_packet.bitstatus.sw_err;
    run_status.bitstatus.config_err         = S2_packet.bitstatus.config_err;
    run_status.bitstatus.calib_err          = S2_packet.bitstatus.calib_err;
    run_status.bitstatus.accel_degradation  = S2_packet.bitstatus.accel_degradation;
    run_status.bitstatus.gyro_degradation   = S2_packet.bitstatus.gyro_degradation;
    run_status.bitstatus.forced_restart     = S2_packet.bitstatus.forced_restart;
    run_status.bitstatus.crc_err            = S2_packet.bitstatus.crc_err;
    run_status.bitstatus.tx_overflow_err    = S2_packet.bitstatus.tx_overflow_err;

    run_status.bitstatus.MCU_status = (fd_hwRst.SrtUp.all > 0) ? 1 : 0;   /*1: fault; 0: OK*/
    
    if(run_status.imu_temp < UNDER_TEMERATURE_THRESHOLD_VALUE) 
    {
      run_status.bitstatus.temperature_under_imu_flag = 1;
    }
    else
    {
      run_status.bitstatus.temperature_under_imu_flag = 0;
    }
    
    if(run_status.imu_temp > IMU_OVER_TEMERATURE_THRESHOLD_VALUE) 
    {
      run_status.bitstatus.temperature_over_imu_flag = 1;
    }
    else
    {
      run_status.bitstatus.temperature_over_imu_flag = 0;
    }
}

void runstatus_gnss_status_update(void)
{
    run_status.sta_temp = get_sta_temperature();    //get sta temperature
    run_status.bitstatus.pps_status = gnss_bitstatus.pps_status;
    run_status.bitstatus.gnss_data_status = gnss_bitstatus.gnss_data_status;
    run_status.bitstatus.gnss_signal_status = gnss_bitstatus.gnss_signal_status;
    
    if(run_status.sta_temp < UNDER_TEMERATURE_THRESHOLD_VALUE) 
    {
      run_status.bitstatus.temperature_under_sta_flag = 1;
    }
    else
    {
      run_status.bitstatus.temperature_under_sta_flag = 0;
    }
    
    if(run_status.sta_temp > OVER_TEMERATURE_THRESHOLD_VALUE) 
    {
      run_status.bitstatus.temperature_over_sta_flag = 1;
    }
    else
    {
      run_status.bitstatus.temperature_over_sta_flag = 0;
    }
}
