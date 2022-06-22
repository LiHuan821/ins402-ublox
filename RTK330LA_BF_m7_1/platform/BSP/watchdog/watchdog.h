#ifndef _WATCHDOG_H
#define _WATCHDOG_H

#include <stdint.h>
#include "cy_device_headers.h"
#include "cy_project.h"

#define WATCHDOG_SET_PIN                     7U
#define WATCHDOG_SET_PORT                    GPIO_PRT17
#define WATCHDOG_SET_PIN_MUX                 P17_7_GPIO

#define WATCHDOG_WDI_PIN                     6U
#define WATCHDOG_WDI_PORT                    GPIO_PRT17
#define WATCHDOG_WDI_PIN_MUX                 P17_6_TCPWM1_LINE260


/* PWM Mode Configuration def */
#define TCPWMx_GRPx_CNTx_WDI                TCPWM1_GRP1_CNT4
#define PCLK_TCPWMx_CLOCKSx_WDI             PCLK_TCPWM1_CLOCKS260
#define TCPWM_PERI_CLK_DIVIDER_WDI          0u

#define TCPWMx_PWM_PRESCALAR_DIV_x_WDI      CY_TCPWM_PWM_PRESCALER_DIVBY_4
#define TCPWMx_PERIOD_WDI                   2994
#define TCPWMx_COMPARE0_WDI                 1497


void watchdog_init(void);
void watchdog_start(void);


#endif
