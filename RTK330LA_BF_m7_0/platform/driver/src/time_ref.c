/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "cy_project.h"
#include "cy_device_headers.h"
#include "time_ref.h"


static uint64_t platform_counter = 0;


#define TCPWM1_GRPx_CNTx_COUNTER                TCPWM0_GRP0_CNT0
#define PCLK_TCPWM1_CLOCKSx_COUNTER             PCLK_TCPWM0_CLOCKS0
volatile mcu_time_base_t g_MCU_time;

//125    1ms
//1250   10ms
//625    5ms
cy_stc_tcpwm_counter_config_t const MyCounter_config =
{
    .period             = 125 - 1,                             // 125000 / 125000 = 1s
    .clockPrescaler     = CY_TCPWM_COUNTER_PRESCALER_DIVBY_16,  // 2,000,000Hz / 16 = 125000Hz
    .runMode            = CY_TCPWM_PWM_CONTINUOUS,
    .countDirection     = CY_TCPWM_COUNTER_COUNT_UP,
    .debug_pause        = 0uL,
    .CompareOrCapture   = CY_TCPWM_COUNTER_MODE_COMPARE,
    .compare0           = 0,
    .compare0_buff      = 0,
    .compare1           = 0,
    .compare1_buff      = 0,
    .enableCompare0Swap = false,
    .enableCompare1Swap = false,
    .interruptSources   = 0uL,
    .capture0InputMode  = 3uL,
    .capture0Input      = 0uL,
    .reloadInputMode    = 3uL,
    .reloadInput        = 0uL,
    .startInputMode     = 3uL,
    .startInput         = 0uL,
    .stopInputMode      = 3uL,
    .stopInput          = 0uL,
    .capture1InputMode  = 3uL,
    .capture1Input      = 0uL,
    .countInputMode     = 3uL,
    .countInput         = 1uL,
    .trigger1           = CY_TCPWM_COUNTER_OVERFLOW,
};

cy_stc_sysint_irq_t irq_cfg = 
{
    .sysIntSrc  = tcpwm_0_interrupts_0_IRQn,
    .intIdx     = CPUIntIdx4_IRQn,
    .isEnabled  = true,
};


void Timer_Handler(void)
{
    if(Cy_Tcpwm_Counter_GetTC_IntrMasked(TCPWM1_GRPx_CNTx_COUNTER))
    {
        platform_counter++;
        if ((platform_counter % 1000) == 0) {

        }
        // process_user_commands();
        // g_MCU_time.msec += 1;
        // if(g_MCU_time.msec >= 1000)
        // {
        //     g_MCU_time.msec = 0;
        //     g_MCU_time.time ++;
        // }
        Cy_Tcpwm_Counter_ClearTC_Intr(TCPWM1_GRPx_CNTx_COUNTER);
    }
}

int time_driver_install(void)
{

    //Cy_SysClk_HfClkEnable(CY_SYSCLK_HFCLK_2); //CLK_HF2 enable
    
    /*--------------------------------*/
    /* Clock Configuration for TCPWMs */
    /*--------------------------------*/
    uint32_t periFreq = 60000000ul;
    uint32_t targetFreq = 2000000ul;
    uint32_t divNum = (periFreq / targetFreq);

	/* Assign a programmable divider for TCPWM1_GRP0_CNT0 */
    Cy_SysClk_PeriphAssignDivider(PCLK_TCPWM1_CLOCKSx_COUNTER, (cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, 1);
    Cy_SysClk_PeriphSetDivider(Cy_SysClk_GetClockGroup(PCLK_TCPWM1_CLOCKSx_COUNTER), CY_SYSCLK_DIV_16_BIT, 1, divNum - 1);
    Cy_SysClk_PeriphEnableDivider(Cy_SysClk_GetClockGroup(PCLK_TCPWM1_CLOCKSx_COUNTER), CY_SYSCLK_DIV_16_BIT, 1);

    Cy_SysInt_InitIRQ(&irq_cfg);
    Cy_SysInt_SetSystemIrqVector(irq_cfg.sysIntSrc, Timer_Handler);
    /* Set the Interrupt Priority & Enable the Interrupt */
    NVIC_SetPriority(CPUIntIdx4_IRQn, 1u);
    NVIC_ClearPendingIRQ(CPUIntIdx4_IRQn);
    
    
    NVIC_EnableIRQ(CPUIntIdx4_IRQn);
    
    
    /* Initialize TCPWM0_GPR0_CNT0 as Timer/Counter & Enable */
    Cy_Tcpwm_Counter_Init(TCPWM1_GRPx_CNTx_COUNTER, &MyCounter_config);
    Cy_Tcpwm_Counter_Enable(TCPWM1_GRPx_CNTx_COUNTER);
    Cy_Tcpwm_TriggerStart(TCPWM1_GRPx_CNTx_COUNTER);  
    /* Enable Interrupt */
    Cy_Tcpwm_Counter_SetTC_IntrMask(TCPWM1_GRPx_CNTx_COUNTER);
    return 1;
}

/** ***************************************************************************
 * @brief
 * @return tick
 ******************************************************************************/
uint16_t TIMER_GetRollingCount(void)
{
    return (uint16_t)platform_counter;
}

uint64_t platform_get_time_counter(void)
{
    return platform_counter;
}


void delay_ms(uint32_t time)
{    
   uint32_t i=0;  
   while(time--)
   {
      i = 18000;  //?1ms
      while(i--)
      {
        asm("nop");
      }
   }
}

/* [] END OF FILE */
