#include "timer.h"

static uint32_t systime_cnt = 0;

static const cy_stc_tcpwm_counter_config_t sys_timer_cfg =
{
    .period             = 125 - 1,                             // 125000 / 125 = 1000Hz
    .clockPrescaler     = CY_TCPWM_COUNTER_PRESCALER_DIVBY_16, // 2,000,000Hz / 16 = 125000Hz
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

cy_stc_sysint_irq_t sys_timer_irq_cfg = 
{
    .sysIntSrc  = SYS_TIMER_IRQn,
    .intIdx     = CPUIntIdx7_IRQn,
    .isEnabled  = true,
};

static void sys_timer_isr(void)
{
    if (Cy_Tcpwm_Counter_GetTC_IntrMasked(SYS_TIMER_CNT)) {
        Cy_Tcpwm_Counter_ClearTC_Intr(SYS_TIMER_CNT);

        systime_cnt++;
    }
}

void system_timer_init(void)
{
    uint32_t periFreq = 160000000ul;
    uint32_t targetFreq = 2000000ul;
    uint32_t divNum = (periFreq / targetFreq);

    Cy_SysClk_PeriphAssignDivider(SYS_TIMER_CNT_CLOCKS, (cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, 1);
    Cy_SysClk_PeriphSetDivider(Cy_SysClk_GetClockGroup(SYS_TIMER_CNT_CLOCKS), CY_SYSCLK_DIV_16_BIT, 1, divNum - 1);
    Cy_SysClk_PeriphEnableDivider(Cy_SysClk_GetClockGroup(SYS_TIMER_CNT_CLOCKS), CY_SYSCLK_DIV_16_BIT, 1);

    Cy_SysInt_InitIRQ(&sys_timer_irq_cfg);
    Cy_SysInt_SetSystemIrqVector(sys_timer_irq_cfg.sysIntSrc, sys_timer_isr);
    NVIC_SetPriority(sys_timer_irq_cfg.intIdx, 7u);
    NVIC_EnableIRQ(sys_timer_irq_cfg.intIdx);
    
    Cy_Tcpwm_Counter_Init(SYS_TIMER_CNT, &sys_timer_cfg);
    Cy_Tcpwm_Counter_Enable(SYS_TIMER_CNT);
    Cy_Tcpwm_TriggerStart(SYS_TIMER_CNT);  
    Cy_Tcpwm_Counter_SetTC_IntrMask(SYS_TIMER_CNT);
}

uint32_t system_timer_cnt(void)
{
    return systime_cnt;
}
