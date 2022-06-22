#ifndef _STATUS_HANDLER_H
#define _STATUS_HANDLER_H

#include <stdint.h>

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
    status_bit_t status_bit;
    float imu_temp;
    float mcu_temp;
} status_t;

void init_status_struct(void);
void status_imu_chip_update(uint16_t activeSensors);
void status_imu_data_update(void);

extern status_t g_status;

#endif /* _STATUS_HANDLER_H */
