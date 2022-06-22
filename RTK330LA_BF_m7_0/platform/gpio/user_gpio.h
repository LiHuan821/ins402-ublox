
#ifndef USER_GPIO_H
#define USER_GPIO_H

#include "cy_device_headers.h"
#ifdef __cplusplus
 extern "C" {
#endif
#include "cy_gpio.h"


#define BGET(bit,Register)  (((Register->unIN.u32Register) >> (bit)) & CY_GPIO_IN_MASK)  //获得寄存器一位的状态

#define SENSOR1_CS_PORT 		GPIO_PRT32
#define SENSOR1_CS_PIN 			7U
#define SENSOR1_CS_PIN_MUX		P32_7_GPIO

#define SENSOR1_CLK_PORT 		GPIO_PRT32
#define SENSOR1_CLK_PIN 		2U
#define SENSOR1_CLK_PIN_MUX		P32_2_GPIO

#define SENSOR1_MOSI_PORT 		GPIO_PRT32
#define SENSOR1_MOSI_PIN 		3U
#define SENSOR1_MOSI_PIN_MUX	P32_3_GPIO

#define SENSOR1_MISO_PORT 		GPIO_PRT32
#define SENSOR1_MISO_PIN 		0U
#define SENSOR1_MISO_PIN_MUX	P32_0_GPIO

#define SENSOR2_CS_PORT 		GPIO_PRT28
#define SENSOR2_CS_PIN 			4U
#define SENSOR2_CLK_PIN_MUX		P28_4_GPIO

#define SENSOR2_CLK_PORT 		GPIO_PRT28
#define SENSOR2_CLK_PIN 		6U
#define SENSOR2_CS_PIN_MUX		P28_6_GPIO

#define SENSOR2_MOSI_PORT 		GPIO_PRT28
#define SENSOR2_MOSI_PIN 		3U
#define SENSOR2_MOSI_PIN_MUX	P28_3_GPIO

#define SENSOR2_MISO_PORT 		GPIO_PRT28
#define SENSOR2_MISO_PIN 		5U
#define SENSOR2_MISO_PIN_MUX	P28_5_GPIO

#define SENSOR3_CS_PORT 		GPIO_PRT8
#define SENSOR3_CS_PIN 			4U
#define SENSOR3_CS_PIN_MUX		P8_4_GPIO

#define SENSOR3_CLK_PORT 		GPIO_PRT8
#define SENSOR3_CLK_PIN 		4U
#define SENSOR3_CLK_PIN_MUX		P8_4_GPIO

#define SENSOR3_MOSI_PORT 		GPIO_PRT8
#define SENSOR3_MOSI_PIN 		4U
#define SENSOR3_MOSI_PIN_MUX	P8_4_GPIO

#define SENSOR3_MISO_PORT 		GPIO_PRT8
#define SENSOR3_MISO_PIN 		4U
#define SENSOR3_MISO_PIN_MUX	P8_4_GPIO


void sensor_spi_gpio_init();

void gpio_set(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
void gpio_reset(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
void gpio_toggle(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
uint8_t gpio_get(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
void PINS_DRV_SetPins(volatile stc_GPIO_PRT_t* base, uint32_t pins);
void PINS_DRV_ClearPins(volatile stc_GPIO_PRT_t* base, uint32_t pins);

#ifdef __cplusplus
}
#endif

#endif