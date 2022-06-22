#ifndef __GPIO_H_
#define __GPIO_H_

#include <stdint.h>
#include "cy_project.h"

typedef struct {
    volatile stc_GPIO_PRT_t* portReg;
    uint8_t                  pinNum;
    cy_stc_gpio_pin_config_t cfg;
} stc_pin_config;


#endif
