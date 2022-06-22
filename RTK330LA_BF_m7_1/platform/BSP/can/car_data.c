#include "car_data.h"
#include "can.h"
#include "string.h"
#include "gnss.h"
#include "ins_interface_API.h"
#include "rtkcmn.h"
#include "constants.h"
#include "timer.h"

WHEEL_SPEED_STRUCT wheel_speed;

extern int32_t gps_start_week;

void car_can_initialize(void)
{
    can_init();

    memset(&wheel_speed, 0, sizeof(WHEEL_SPEED_STRUCT));
    wheel_speed.fwd = 1;
}


/*
Now the default data is Toyota Corolla 2019.
If it is another vehicle, modify this function to fit the communication protocol
*/
uint8_t wheel_speed_update = 0;

void car_can_data_process(uint32_t stdId, uint8_t* data)
{
    gps_time_t odo_time;
    get_gps_time(&odo_time);

    if (stdId == CAR_CAN_ID_WHEEL_SPEED) {
        wheel_speed.week = odo_time.week;
        wheel_speed.timestamp = odo_time.timeofweek;
        
        wheel_speed.speed_FR = (((data[0] << 8) + data[1]) - 6767) * 0.01 * 1000 / 3600;
        wheel_speed.speed_FL = (((data[2] << 8) + data[3]) - 6767) * 0.01 * 1000 / 3600;
        wheel_speed.speed_RR = (((data[4] << 8) + data[5]) - 6767) * 0.01 * 1000 / 3600;
        wheel_speed.speed_RL = (((data[6] << 8) + data[7]) - 6767) * 0.01 * 1000 / 3600;
        wheel_speed.update = 1;
    }
    wheel_speed_update = 1;
}



uint8_t car_get_wheel_speed(double *car_speed, uint8_t *fwd, uint32_t *week, double *timestamp)
{
    if (wheel_speed_update == 1)
    {
        *week = wheel_speed.week;
        *timestamp = wheel_speed.timestamp;
        *fwd = wheel_speed.fwd;
        // if ((wheel_speed.update & 3) == 3) 
        {
            *car_speed = (wheel_speed.speed_RR + wheel_speed.speed_RL) * 0.5;
        }
        wheel_speed_update = 0;
        return 1;
    }
    return 0;
}
