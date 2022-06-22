#ifndef _GNSS_H_
#define _GNSS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <time.h>
#include "typedefs.h"
#include "share_memory.h"
#include "utils.h"
#include "ins_interface_API.h"
#include "uart.h"

  
#define PI          3.1415926535897932   /* pi */
#define R2D         (57.295779513082320)
#define D2R         (0.017453292519943)
#define RE_WGS84    6378137.0            /* earth semimajor axis (WGS84) (m) */
#define FE_WGS84    (1.0/298.257223563)  /* earth flattening (WGS84) */

#define MAXLEAPS      64                  /* max number of leap seconds table */

#ifdef WIN32
typedef struct {                        /* time struct */
    time_t time;                        /* time (s) expressed by standard time_t */
    double sec;                         /* fraction of second under 1 s */
} gtime_t;
#else
typedef struct {                        /* time struct */
    time_t time;                        /* time (s) expressed by standard time_t */
    double sec;                         /* fraction of second under 1 s */
} gtime_t;
#endif /*WIN32*/

#pragma pack(1)
typedef struct mcu_time_base_t_
{
    long long time;
    long long  msec;
} mcu_time_base_t;



typedef struct {
    uint32_t pps_status : 1;         // 0 – normal; 1 – 1PPS pulse exception
    uint32_t gnss_data_status : 1;   // 0 – normal; 1 – GNSS chipset has NO data output
    uint32_t gnss_signal_status : 1; // 0 – normal; 1 – GNSS chipset has data output but no valid signal detected
} gnss_bitstatus_t;

#pragma pack()

extern fifo_type fifo_rtk_gnss_rover;
extern uint8_t fifo_rtk_gnss_rover_buf[RTCM_FIFO_DATA_SIZE];

extern fifo_type fifo_secondary_rtk_gnss_rover;
extern uint8_t fifo_secondary_rtk_gnss_rover_buf[RTCM_SECONDARY_FIFO_DATA_SIZE];

extern gnss_solution_t *g_ptr_gnss_sol;
extern gnss_solution_t g_gnss_sol;

extern uint8_t gnss_obs_update;

extern gnss_bitstatus_t gnss_bitstatus;

extern uint8_t gnss_valid;
extern uint8_t gnss_lose;

void rtk_fifo_init(void);
void gnss_solution_process(void);
void gnss_status_check(void);
void sens_GetGnssData_INS(GnssData* gnss_data_ptr);
float get_sta_temperature(void);
uint32_t get_sta_version(void);

#ifdef __cplusplus
}
#endif

#endif
