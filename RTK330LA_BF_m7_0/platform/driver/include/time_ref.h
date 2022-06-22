#ifndef _TIME_REF_H_
#define _TIME_REF_H_

#include <stdint.h>
#include <time.h>
#include "gpsAPI.h"
//typedef struct mcu_time_base_t_
//{
//    time_t time;
//    time_t msec;
//} mcu_time_base_t;

int32_t time_driver_install(void);
void delay_ms(uint32_t time);

uint64_t platform_get_time_counter(void);

extern volatile mcu_time_base_t g_MCU_time;

#endif
