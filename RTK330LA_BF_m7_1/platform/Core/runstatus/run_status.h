#ifndef _RUN_STATUS_H_
#define _RUN_STATUS_H_

#include <stdint.h>

#define UNDER_TEMERATURE_THRESHOLD_VALUE      -40
#define IMU_OVER_TEMERATURE_THRESHOLD_VALUE   85
#define OVER_TEMERATURE_THRESHOLD_VALUE       105

// payload structure of status data message
typedef struct {
    /* IMU */
    uint32_t master_fail : 1;       // 0 = normal; 1 = fatal error occured
    uint32_t hw_err : 1;            // 0 = normal; 1 = hardware exception detected
    uint32_t sw_err : 1;            // 0 = normal; 1 = software exception detected
    uint32_t config_err : 1;        // 0 = normal; 1 = config error detected by periodic self-test
    uint32_t calib_err : 1;         // 0 = normal; 1 = calibration data corrupted
    uint32_t accel_degradation : 1; // 0 = normal; 1 = accel data degradation due to sensor exception
    uint32_t gyro_degradation : 1;  // 0 = normal; 1 = gyro data degradation due to sensor exception
    uint32_t forced_restart : 1;    // 0 = normal; 1 = forced restart
    uint32_t crc_err : 1;           // 0 = normal; 1 = Application CRC error detected
    uint32_t tx_overflow_err : 1;   // 0 = normal; 1 = Tx Overflow occurred 10 consecutive cycles

    /* GNSS unit status */
    uint32_t pps_status : 1;         // 0 – normal; 1 – 1PPS pulse exception
    uint32_t gnss_data_status : 1;   // 0 – normal; 1 – GNSS chipset has NO data output
    uint32_t gnss_signal_status : 1; // 0 – normal; 1 – GNSS chipset has data output but no valid signal detected

    /* operation */
    uint32_t power : 1;      //  0 – normal; 1 - any component has no power
    uint32_t MCU_status : 1; // 0 – normal; 1 – MCU failure
    
    uint32_t temperature_under_mcu_flag:1;  // 0 – normal; 1 – under temperature
    uint32_t temperature_under_sta_flag:1;  // 0 – normal; 1 – under temperature
    uint32_t temperature_under_imu_flag:1;  // 0 – normal; 1 – under temperature
    
    uint32_t temperature_over_mcu_flag:1;   // 0 – normal; 1 – over temperature
    uint32_t temperature_over_sta_flag:1;   // 0 – normal; 1 – over temperature
    uint32_t temperature_over_imu_flag:1;   // 0 – normal; 1 – over temperature

    uint32_t rexerved : 11;
} device_status_bit_t;

typedef struct {
    device_status_bit_t bitstatus;
    float imu_temp;
    float mcu_temp;
    float sta_temp;
} run_status_t;

extern run_status_t run_status;

void runstatus_init(void);
void runstatus_mcu_temperature_update(void);
void runstatus_imu_status_update(void);
void runstatus_gnss_status_update(void);

#endif
