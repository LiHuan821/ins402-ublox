
#ifndef _STA9100_CONTROL_H_
#define _STA9100_CONTROL_H_

#include "cy_device_headers.h"
#ifdef __cplusplus
 extern "C" {
#endif
#include "cy_project.h"


//#define ST_MODE_PIN                         2U
//#define ST_MODE_PORT                        GPIO_PRT7
//#define ST_MODE_PIN_MUX                     P7_2_GPIO

#define ST_MODE_PIN                             7U
#define ST_MODE_PORT                            GPIO_PRT29
#define ST_MODE_PIN_MUX                         P29_7_GPIO

#define ST_RESET_PIN                            2U
#define ST_RESET_PORT                           GPIO_PRT31
#define ST_RESET_PIN_MUX                        P31_2_GPIO


#define ST_STDBY_PIN                            1U
#define ST_STDBY_PORT                           GPIO_PRT29
#define ST_STDBY_PIN_MUX                        P29_1_GPIO

#define ST_WKUP_PIN                             0U
#define ST_WKUP_PORT                            GPIO_PRT29
#define ST_WKUP_PIN_MUX                         P29_0_GPIO


#define ST_UART2_TX_PIN                         1U
#define ST_UART2_TX_PORT                        GPIO_PRT28
#define ST_UART2_TX_PIN_MUX                     P28_1_GPIO

#define ST_UART2_RX_PIN                         0U
#define ST_UART2_RX_PORT                        GPIO_PRT28
#define ST_UART2_RX_PIN_MUX                     P28_0_GPIO


#define ST_PPS1_PIN                             3U
#define ST_PPS1_PORT                            GPIO_PRT5
#define ST_PPS1_PIN_MUX                         P5_3_GPIO
#define ST_PPS1_IRQn                            ioss_interrupts_gpio_dpslp_5_IRQn


#define ST_PPS2_PIN                             5U
#define ST_PPS2_PORT                            GPIO_PRT5
#define ST_PPS2_PIN_MUX                         P5_5_GPIO


#define ST_PROG_BUF_CTL_PIN                     5U
#define ST_PROG_BUF_CTL_PORT                    GPIO_PRT20
#define ST_PROG_BUF_CTL_PIN_MUX                 P20_5_GPIO

   
//#define BOOT_SWITCH_PIN                     7U
//#define BOOT_SWITCH_PORT                    GPIO_PRT29
//#define BOOT_SWITCH_PIN_MUX		            P29_7_GPIO   


#define BOOT_SWITCH_PIN                         2U
#define BOOT_SWITCH_PORT                        GPIO_PRT7
#define BOOT_SWITCH_PIN_MUX                     P7_2_GPIO   
   
#if 0
#define ST_BOOT_PIN                         2U
#define ST_BOOT_PORT                        GPIO_PRT7
#define ST_BOOT_PIN_MUX		                P8_4_GPIO
#define ST_PROG_BUF_CTL_PIN                 2U
#define ST_PROG_BUF_CTL_PORT                GPIO_PRT7
#define ST_PROG_BUF_CTL_PIN_MUX		        P8_4_GPIO
#endif

void bsp_sta9100_gpio_init(void);

uint32_t bsp_bootmode_detect(void);

void bsp_sta9100_gpio_setting(uint32_t mode);

void bsp_sta9100_reset(void);

void sta9100_pps_init();

#ifdef __cplusplus
}
#endif

#endif