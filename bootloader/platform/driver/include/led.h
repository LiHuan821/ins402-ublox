#ifndef _LED_H_
#define _LED_H_

#include "cy_project.h"
#include "cy_device_headers.h"

#define USER_LED1_PORT           GPIO_PRT29
#define USER_LED1_PIN            6
#define USER_LED1_PIN_MUX        P29_6_GPIO 

#define USER_LED2_PORT           GPIO_PRT29
#define USER_LED2_PIN            5
#define USER_LED2_PIN_MUX        P29_5_GPIO 

#define USER_LED3_PORT           GPIO_PRT29
#define USER_LED3_PIN            2
#define USER_LED3_PIN_MUX        P29_2_GPIO 

void bsp_led_init(void);
void bsp_led_on(uint8_t number);
void bsp_led_off(uint8_t number);
void bsp_led_toggle(uint8_t number);

#endif
