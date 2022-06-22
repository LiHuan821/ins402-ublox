#include "user_gpio.h"
#include "cy_device_headers.h"


cy_stc_gpio_pin_config_t user_sensor1_cs_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = SENSOR1_CS_PIN_MUX,                           
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

cy_stc_gpio_pin_config_t user_sensor1_clk_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = SENSOR1_CLK_PIN_MUX,                           
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

cy_stc_gpio_pin_config_t user_sensor1_mosi_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = SENSOR1_MOSI_PIN_MUX,                           
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

cy_stc_gpio_pin_config_t user_sensor1_miso_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
//    .driveMode = CY_GPIO_DM_HIGHZ,    
    .driveMode = CY_GPIO_DM_OD_DRIVESHIGH,
    .hsiom = SENSOR1_MISO_PIN_MUX,                           
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



cy_stc_gpio_pin_config_t user_sensor2_cs_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = SENSOR2_CS_PIN_MUX,                           
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

cy_stc_gpio_pin_config_t user_sensor2_clk_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = SENSOR2_CLK_PIN_MUX,                           
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

cy_stc_gpio_pin_config_t user_sensor2_mosi_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = SENSOR2_MOSI_PIN_MUX,                           
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

cy_stc_gpio_pin_config_t user_sensor2_miso_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_HIGHZ,    
    .hsiom = SENSOR2_MISO_PIN_MUX,                           
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




cy_stc_gpio_pin_config_t user_sensor3_cs_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = SENSOR3_CS_PIN_MUX,                           
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

cy_stc_gpio_pin_config_t user_sensor3_clk_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = SENSOR3_CLK_PIN_MUX,                           
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

cy_stc_gpio_pin_config_t user_sensor3_mosi_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,    
    .hsiom = SENSOR3_MOSI_PIN_MUX,                           
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

cy_stc_gpio_pin_config_t user_sensor3_miso_port_pin_cfg =             
{                                                  
    .outVal = 0x00,                                
    .driveMode = CY_GPIO_DM_HIGHZ,    
    .hsiom = SENSOR3_MISO_PIN_MUX,                           
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


void sensor_spi_gpio_init()
{
    Cy_GPIO_Pin_Init(SENSOR1_CS_PORT, SENSOR1_CS_PIN, &user_sensor1_cs_port_pin_cfg);
    Cy_GPIO_Pin_Init(SENSOR1_CLK_PORT, SENSOR1_CLK_PIN, &user_sensor1_clk_port_pin_cfg);
    Cy_GPIO_Pin_Init(SENSOR1_MOSI_PORT, SENSOR1_MOSI_PIN, &user_sensor1_mosi_port_pin_cfg);
    Cy_GPIO_Pin_Init(SENSOR1_MISO_PORT, SENSOR1_MISO_PIN, &user_sensor1_miso_port_pin_cfg);
#if 1
    Cy_GPIO_Pin_Init(SENSOR2_CS_PORT, SENSOR2_CS_PIN, &user_sensor2_cs_port_pin_cfg);
    Cy_GPIO_Pin_Init(SENSOR2_CLK_PORT, SENSOR2_CLK_PIN, &user_sensor2_clk_port_pin_cfg);
    Cy_GPIO_Pin_Init(SENSOR2_MOSI_PORT, SENSOR2_MOSI_PIN, &user_sensor2_mosi_port_pin_cfg);
    Cy_GPIO_Pin_Init(SENSOR2_MISO_PORT, SENSOR2_MISO_PIN, &user_sensor2_miso_port_pin_cfg);

    // Cy_GPIO_Pin_Init(SENSOR3_CS_PORT, SENSOR3_CS_PIN, &user_sensor3_cs_port_pin_cfg);
    // Cy_GPIO_Pin_Init(SENSOR3_CLK_PORT, SENSOR3_CLK_PIN, &user_sensor3_clk_port_pin_cfg);
    // Cy_GPIO_Pin_Init(SENSOR3_MOSI_PORT, SENSOR3_MOSI_PIN, &user_sensor3_mosi_port_pin_cfg);
    // Cy_GPIO_Pin_Init(SENSOR3_MISO_PORT, SENSOR3_MISO_PIN, &user_sensor3_miso_port_pin_cfg);
#endif
}

/**********************************************
* @brief 
* 
* @param  
* @return  
***********************************************/
void gpio_set(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
  Cy_GPIO_Set(base, pinNum);
}

void gpio_reset(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
  Cy_GPIO_Clr(base, pinNum);
}

void gpio_toggle(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
	Cy_GPIO_Inv(base, pinNum);
}

uint8_t gpio_get(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
	return Cy_GPIO_Read(base,pinNum);
}

void PINS_DRV_SetPins(volatile stc_GPIO_PRT_t* base, uint32_t pins)
{
    base->unOUT_SET.u32Register = pins;
}

void PINS_DRV_ClearPins(volatile stc_GPIO_PRT_t* base, uint32_t pins)
{
    base->unOUT_CLR.u32Register = pins;
}
