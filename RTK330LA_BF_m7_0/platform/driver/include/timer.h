#ifndef __TIMER_H_
#define __TIMER_H_

#include <stdint.h>
#include "cy_project.h"

#define SYS_TIMER_CNT         TCPWM0_GRP0_CNT0
#define SYS_TIMER_CNT_CLOCKS  PCLK_TCPWM0_CLOCKS0

#define SYS_TIMER_IRQn        tcpwm_0_interrupts_0_IRQn

void system_timer_init(void);
uint32_t system_timer_cnt(void);

#endif /* __TIMER_H_ */
