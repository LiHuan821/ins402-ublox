#include "watchdog.h"


cy_stc_gpio_pin_config_t watchdog_set_port_pin_cfg =             
{                                                  
    .outVal = 0x00,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = WATCHDOG_SET_PIN_MUX,
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

cy_stc_gpio_pin_config_t watchdog_wdi_port_pin_cfg = 
{
    .outVal    = 0ul,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom     = WATCHDOG_WDI_PIN_MUX,
    .intEdge   = 0ul,
    .intMask   = 0ul,
    .vtrip     = 0ul,
    .slewRate  = 0ul,
    .driveSel  = 0ul,
};

cy_stc_tcpwm_pwm_config_t const watchdog_wdi_pwm_config =
{
    .pwmMode            = CY_TCPWM_PWM_MODE_PWM,
    .clockPrescaler     = TCPWMx_PWM_PRESCALAR_DIV_x_WDI,
    .debug_pause        = false,
    .Cc0MatchMode       = CY_TCPWM_PWM_TR_CTRL2_CLEAR,
    .OverflowMode       = CY_TCPWM_PWM_TR_CTRL2_SET,
    .UnderflowMode      = CY_TCPWM_PWM_TR_CTRL2_NO_CHANGE,
    .Cc1MatchMode       = CY_TCPWM_PWM_TR_CTRL2_NO_CHANGE,
    .deadTime           = 0u,
    .deadTimeComp       = 0u,
    .runMode            = CY_TCPWM_PWM_CONTINUOUS,
    .period             = TCPWMx_PERIOD_WDI - 1ul,
    .period_buff        = 0ul,
    .enablePeriodSwap   = false,
    .compare0           = TCPWMx_COMPARE0_WDI,
    .compare1           = 0ul,
    .enableCompare0Swap = false,
    .enableCompare1Swap = false,
    .interruptSources   = 0ul,
    .invertPWMOut       = 0ul,
    .invertPWMOutN      = 0ul,
    .killMode           = CY_TCPWM_PWM_STOP_ON_KILL,
    .switchInputMode    = 3ul,
    .switchInput        = 0ul,
    .reloadInputMode    = 3ul,
    .reloadInput        = 0ul,
    .startInputMode     = 3ul,
    .startInput         = 0ul,
    .kill0InputMode     = 3ul,
    .kill0Input         = 0ul,
    .kill1InputMode     = 3ul,
    .kill1Input         = 0ul,
    .countInputMode     = 3ul,
    .countInput         = 1ul,
};

void watchdog_init(void)
{
    Cy_GPIO_Pin_Init(WATCHDOG_SET_PORT, WATCHDOG_SET_PIN, &watchdog_set_port_pin_cfg);
    Cy_GPIO_Clr(WATCHDOG_SET_PORT, WATCHDOG_SET_PIN);
    /*--------------------------------*/
    /* Clock Configuration for TCPWMs */
    /*--------------------------------*/
    uint32_t sourceFreq = 80000000ul;
    uint32_t targetFreq = 2000000ul;
    uint32_t divNum = (sourceFreq / targetFreq);

    /* Assign a programmable divider  for TCPWMx_GRPx_CNTx_PWM */
    Cy_SysClk_PeriphAssignDivider(PCLK_TCPWMx_CLOCKSx_WDI, CY_SYSCLK_DIV_16_BIT, TCPWM_PERI_CLK_DIVIDER_WDI);
    /* Sets the 16-bit divider */
    Cy_SysClk_PeriphSetDivider(Cy_SysClk_GetClockGroup(PCLK_TCPWMx_CLOCKSx_WDI), CY_SYSCLK_DIV_16_BIT, TCPWM_PERI_CLK_DIVIDER_WDI, (divNum-1ul));
    /* Enable the divider */
    Cy_SysClk_PeriphEnableDivider(Cy_SysClk_GetClockGroup(PCLK_TCPWMx_CLOCKSx_WDI), CY_SYSCLK_DIV_16_BIT, TCPWM_PERI_CLK_DIVIDER_WDI);

    /*------------------------------*/
    /* Port Configuration for TCPWM */
    /*------------------------------*/    
    Cy_GPIO_Pin_Init(WATCHDOG_WDI_PORT, WATCHDOG_WDI_PIN, &watchdog_wdi_port_pin_cfg);

    /* Initialize TCPWM0_GRPx_CNTx_PWM_PR as PWM Mode & Enable */
    Cy_Tcpwm_Pwm_Init(TCPWMx_GRPx_CNTx_WDI, &watchdog_wdi_pwm_config);
    Cy_Tcpwm_Pwm_Enable(TCPWMx_GRPx_CNTx_WDI);
}

void watchdog_start(void)
{
    Cy_GPIO_Set(WATCHDOG_SET_PORT, WATCHDOG_SET_PIN);
    Cy_Tcpwm_TriggerStart(TCPWMx_GRPx_CNTx_WDI);
}
