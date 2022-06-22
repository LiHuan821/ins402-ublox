#ifndef ETH_PROCESS_H
#define ETH_PROCESS_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "cy_project.h"
#include "cy_device_headers.h"

#include "status_handler.h"
#define BI_PACKET_MAX_SIZE  2048
#define BI_PACKET_MAX_PAYLOAD  (BI_PACKET_MAX_SIZE-2)

/********************************************************/
#pragma pack(4)

typedef struct {
    uint16_t header;
    uint16_t mesgid;
    uint32_t length;
    uint8_t data[BI_PACKET_MAX_SIZE]; // crc
} binary_packet_format_t;

#pragma pack()

/********************************************************/
#pragma pack(1)

typedef struct {
    uint16_t gps_week;
    uint32_t gps_millisecs;
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
} binary_rawimu_t;

typedef struct {
    uint16_t gps_week;
    uint32_t gps_millisecs;
    uint8_t position_type;
    double latitude;
    double longitude;
    double height;
    float latitude_std;
    float longitude_std;
    float height_std;
    uint8_t numberOfSVs;
    uint8_t numberOfSVs_in_solution;
    float hdop;
    float diffage;
    float north_vel;
    float east_vel;
    float up_vel;
    float north_vel_std;
    float east_vel_std;
    float up_vel_std;
} binary_gnss_sol_t;

typedef struct {
    uint16_t gps_week;
    uint32_t gps_millisecs;
    uint8_t ins_status;
    uint8_t ins_position_type;
    double latitude;
    double longitude;
    double height;
    double north_velocity;
    double east_velocity;
    double up_velocity;
    double roll;
    double pitch;
    double heading;
    float latitude_std;
    float longitude_std;
    float height_std;
    float north_velocity_std;
    float east_velocity_std;
    float up_velocity_std;
    float roll_std;
    float pitch_std;
    float heading_std;
} binary_ins_sol_t;

typedef struct {
    uint16_t gps_week;
    uint32_t gps_millisecs;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    status_t status;
} binary_device_diagnostic_t;

#pragma pack()

/********************************************************/
void eth_recieve_process(void);
void _eth_tx_binary_packet(binary_packet_format_t* p_bi);

#ifdef __cplusplus
}
#endif

#endif
