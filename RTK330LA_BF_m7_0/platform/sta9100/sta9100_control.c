#include "sta9100_control.h"
#include "cy_project.h"
#include "systick/cy_systick.h"
#include "time_ref.h"
// #include "rtklib_core.h"
#include "gpsAPI.h"
#include "serialAPI.h"

extern volatile mcu_time_base_t g_MCU_time;

static void delayms(uint32_t ms_tick)
{
  Cy_SysTick_DelayInUs(ms_tick*1000);
}


cy_stc_gpio_pin_config_t st_mode_port_pin_cfg =             
{                                                  
    .outVal = 0x00,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = ST_MODE_PIN_MUX,
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

cy_stc_gpio_pin_config_t st_buf_control_port_pin_cfg =             
{                                                  
    .outVal = 0x00,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = ST_PROG_BUF_CTL_PIN_MUX,
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

cy_stc_gpio_pin_config_t st_reset_port_pin_cfg =             
{                                                  
    .outVal = 0x00,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = ST_RESET_PIN_MUX,
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

cy_stc_gpio_pin_config_t st_stdby_port_pin_cfg =             
{                                                  
    .outVal = 0x00,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = ST_STDBY_PIN_MUX,
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

cy_stc_gpio_pin_config_t st_wkup_port_pin_cfg =             
{                                                  
    .outVal = 0x00,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = ST_WKUP_PIN_MUX,
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

cy_stc_gpio_pin_config_t st_uart2_tx_port_pin_cfg =             
{                                                  
    .outVal = 0x00,
    .driveMode = CY_GPIO_DM_OD_DRIVESHIGH,
    .hsiom = ST_UART2_TX_PIN_MUX,
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

cy_stc_gpio_pin_config_t st_uart2_rx_port_pin_cfg =             
{                                                  
    .outVal = 0x00,
    .driveMode = CY_GPIO_DM_OD_DRIVESHIGH,
    .hsiom = ST_UART2_RX_PIN_MUX,
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


cy_stc_gpio_pin_config_t boot_switch_port_pin_cfg =             
{                                                  
    .outVal = 0x00,
    .driveMode = CY_GPIO_DM_HIGHZ,
    .hsiom = BOOT_SWITCH_PIN_MUX,
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

void bsp_sta9100_gpio_init(void)
{
    Cy_GPIO_Pin_Init(BOOT_SWITCH_PORT, BOOT_SWITCH_PIN, &boot_switch_port_pin_cfg);

    Cy_GPIO_Pin_Init(ST_RESET_PORT, ST_RESET_PIN, &st_reset_port_pin_cfg);
    Cy_GPIO_Pin_Init(ST_MODE_PORT, ST_MODE_PIN, &st_mode_port_pin_cfg);
    Cy_GPIO_Pin_Init(ST_STDBY_PORT, ST_STDBY_PIN, &st_stdby_port_pin_cfg);
    Cy_GPIO_Pin_Init(ST_WKUP_PORT, ST_WKUP_PIN, &st_wkup_port_pin_cfg);

    Cy_GPIO_Pin_Init(ST_PROG_BUF_CTL_PORT, ST_PROG_BUF_CTL_PIN, &st_buf_control_port_pin_cfg);

    Cy_GPIO_Set(ST_RESET_PORT, ST_RESET_PIN);
    Cy_GPIO_Set(ST_MODE_PORT, ST_MODE_PIN);
    Cy_GPIO_Clr(ST_STDBY_PORT, ST_STDBY_PIN);
    Cy_GPIO_Clr(ST_WKUP_PORT, ST_WKUP_PIN);

    Cy_GPIO_Set(ST_PROG_BUF_CTL_PORT, ST_PROG_BUF_CTL_PIN); // 9100 connect com4
}

uint32_t bsp_bootmode_detect(void)
{
    uint32_t state = Cy_GPIO_Read(BOOT_SWITCH_PORT, BOOT_SWITCH_PIN);
    return state;
}

void bsp_sta9100_gpio_setting(uint32_t mode)
{
    if (mode) {
        // IAP
        st_stdby_port_pin_cfg.driveMode = CY_GPIO_DM_HIGHZ;
        st_wkup_port_pin_cfg.driveMode = CY_GPIO_DM_HIGHZ;
        Cy_GPIO_Pin_Init(ST_STDBY_PORT, ST_STDBY_PIN, &st_stdby_port_pin_cfg);
        Cy_GPIO_Pin_Init(ST_WKUP_PORT, ST_WKUP_PIN, &st_wkup_port_pin_cfg);

        Cy_GPIO_Pin_Init(ST_UART2_TX_PORT, ST_UART2_TX_PIN, &st_uart2_tx_port_pin_cfg);
        Cy_GPIO_Pin_Init(ST_UART2_RX_PORT, ST_UART2_RX_PIN, &st_uart2_rx_port_pin_cfg);    

        Cy_GPIO_Clr(ST_MODE_PORT, ST_MODE_PIN);

        Cy_GPIO_Clr(ST_RESET_PORT, ST_RESET_PIN);
        delayms(100);
        Cy_GPIO_Set(ST_RESET_PORT, ST_RESET_PIN);
        delayms(100);

        Cy_GPIO_Clr(ST_PROG_BUF_CTL_PORT, ST_PROG_BUF_CTL_PIN); // 9100 connect com4
    } else {
        // app
        Cy_GPIO_Clr(ST_WKUP_PORT, ST_WKUP_PIN);
        Cy_GPIO_Set(ST_STDBY_PORT, ST_STDBY_PIN);
        Cy_GPIO_Set(ST_MODE_PORT,ST_MODE_PIN);

        Cy_GPIO_Clr(ST_RESET_PORT,ST_RESET_PIN);
        delayms(100);
        Cy_GPIO_Set(ST_RESET_PORT,ST_RESET_PIN);
        delayms(100);
        
        Cy_GPIO_Clr(ST_PROG_BUF_CTL_PORT, ST_PROG_BUF_CTL_PIN); // 9100 connect com4
    }
}


void bsp_sta9100_reset(void)
{
    Cy_GPIO_Clr(ST_RESET_PORT, ST_RESET_PIN);
    delayms(100);
    Cy_GPIO_Set(ST_RESET_PORT, ST_RESET_PIN);
    delayms(100);
}

const cy_stc_gpio_pin_config_t sta9100_pps_port_pin_cfg =
{
    .outVal    = 0ul,
    .driveMode = CY_GPIO_DM_HIGHZ,
    .hsiom     = ST_PPS1_PIN_MUX,
    .intEdge   = CY_GPIO_INTR_BOTH,
    .intMask   = 1ul,
    .vtrip     = 0ul,
    .slewRate  = 0ul,
    .driveSel  = 0ul,
};


static const cy_stc_sysint_irq_t irq_cfg =
{
    .sysIntSrc  = ST_PPS1_IRQn,
    .intIdx     = CPUIntIdx7_IRQn,
    .isEnabled  = true,
};

extern time_t get_obs_time();
extern uint8_t is_gnss_signal_valid(void);
extern volatile mcu_time_base_t g_obs_rcv_time;

//static uint8_t buff[256];

void PPS_IntHandler(void)
{
    uint32_t intStatus;

    /* If falling edge detected */
    intStatus = Cy_GPIO_GetInterruptStatusMasked(ST_PPS1_PORT, ST_PPS1_PIN);
    if (intStatus != 0ul)
    {
        uint8_t PPSstate = Cy_GPIO_Read(ST_PPS1_PORT,ST_PPS1_PIN);
        if (PPSstate == 0) {
            if (g_MCU_time.msec < 500) {
                // snprintf(buff, 256, "1: %d %d\r\n", g_MCU_time.time, g_MCU_time.msec);
                // uart7_write_dma(buff, strlen(buff));
                g_MCU_time.msec = 490;
                TCPWM1_GRP0_CNT0->unCOUNTER.u32Register = 1250 - 1;
                TCPWM0_GRP0_CNT0->unCOUNTER.u32Register = 125 - 1;
            } else if (g_MCU_time.msec > 500) {
                // snprintf(buff, 256, "2: %d %d\r\n", g_MCU_time.time, g_MCU_time.msec);
                // uart7_write_dma(buff, strlen(buff));
                g_MCU_time.msec = 500;
                TCPWM1_GRP0_CNT0->unCOUNTER.u32Register = 1;
                TCPWM0_GRP0_CNT0->unCOUNTER.u32Register = 1;
            }

            if (is_gnss_signal_valid() && (g_MCU_time.time == g_obs_rcv_time.time)) {
                g_MCU_time.time = get_obs_time();

                if (!is_gnss_start_week_valid()) {
                    gtime_t time;
                    time.time = g_MCU_time.time;
                    time.sec = (double)g_MCU_time.msec / 1000;
                    int week = 0;
                    time2gpst(time, &week);
                    set_gnss_start_week(week);
                }
            }
        }
        Cy_GPIO_ClearInterrupt(ST_PPS1_PORT, ST_PPS1_PIN);
    }
}

void sta9100_pps_init()
{
    Cy_GPIO_Pin_Init(ST_PPS1_PORT, ST_PPS1_PIN, &sta9100_pps_port_pin_cfg);

    Cy_SysInt_InitIRQ(&irq_cfg);
    Cy_SysInt_SetSystemIrqVector(irq_cfg.sysIntSrc, PPS_IntHandler);

    NVIC_SetPriority(irq_cfg.intIdx, 2ul);
    NVIC_EnableIRQ(irq_cfg.intIdx);
}