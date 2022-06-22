/***************************************************************************//**
* \file main_cm0plus.c
*
* \brief
* Main file for CM0+
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_project.h"
#include "cy_device_headers.h"
#include "led.h"


cy_stc_gpio_pin_config_t user_led1_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = USER_LED1_PIN_MUX,                           
    .intEdge = 0,                                  
    .intMask = 0,                                  
    .vtrip = 0,                                    
    .slewRate = 0,                                 
    .driveSel = 0,                                 
    .vregEn = 0,                                   
    .ibufMode = 0,                                 
    .vtripSel = 0,                                 
    .vrefSel = 0,                                  
    .vohSel = 0,                                   
};


cy_stc_gpio_pin_config_t user_led2_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = USER_LED2_PIN_MUX,                           
    .intEdge = 0,                                  
    .intMask = 0,                                  
    .vtrip = 0,                                    
    .slewRate = 0,                                 
    .driveSel = 0,                                 
    .vregEn = 0,                                   
    .ibufMode = 0,                                 
    .vtripSel = 0,                                 
    .vrefSel = 0,                                  
    .vohSel = 0,                                   
};


cy_stc_gpio_pin_config_t user_led3_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = USER_LED3_PIN_MUX,                           
    .intEdge = 0,                                  
    .intMask = 0,                                  
    .vtrip = 0,                                    
    .slewRate = 0,                                 
    .driveSel = 0,                                 
    .vregEn = 0,                                   
    .ibufMode = 0,                                 
    .vtripSel = 0,                                 
    .vrefSel = 0,                                  
    .vohSel = 0,                                   
};

void bsp_led_init(void)
{
    Cy_GPIO_Pin_Init(USER_LED1_PORT, USER_LED1_PIN, &user_led1_port_pin_cfg);
    Cy_GPIO_Pin_Init(USER_LED2_PORT, USER_LED2_PIN, &user_led2_port_pin_cfg);
    Cy_GPIO_Pin_Init(USER_LED3_PORT, USER_LED3_PIN, &user_led3_port_pin_cfg);

    Cy_GPIO_Set(USER_LED1_PORT, USER_LED1_PIN);
    Cy_GPIO_Set(USER_LED2_PORT, USER_LED2_PIN);
    Cy_GPIO_Set(USER_LED3_PORT, USER_LED3_PIN);
}

void bsp_led_on(uint8_t number)
{
    if (number == 1) {
        Cy_GPIO_Clr(USER_LED1_PORT, USER_LED1_PIN);
    } else if (number == 2) {
        Cy_GPIO_Clr(USER_LED2_PORT, USER_LED2_PIN);
    } else if (number == 3) {
        Cy_GPIO_Clr(USER_LED3_PORT, USER_LED3_PIN);
    }
}

void bsp_led_off(uint8_t number)
{
    if (number == 1) {
        Cy_GPIO_Set(USER_LED1_PORT, USER_LED1_PIN);
    } else if (number == 2) {
        Cy_GPIO_Set(USER_LED2_PORT, USER_LED2_PIN);
    } else if (number == 3) {
        Cy_GPIO_Set(USER_LED3_PORT, USER_LED3_PIN);
    }
}

void bsp_led_toggle(uint8_t number)
{
    if (number == 1) {
        Cy_GPIO_Inv(USER_LED1_PORT, USER_LED1_PIN);
    } else if (number == 2) {
        Cy_GPIO_Inv(USER_LED2_PORT, USER_LED2_PIN);
    } else if (number == 3) {
        Cy_GPIO_Inv(USER_LED3_PORT, USER_LED3_PIN);
    }
}

/* [] END OF FILE */
