/***************************************************************************//**
* \file cy_project.h
*
* \brief
* Project specific header
*
* \note
* It contains references to all generated header files and should 
* not be modified.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_PROJECT_H
#define CY_PROJECT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "syslib/cy_syslib.h"

// Accomodate for different system frequencies of PSVP and silicon for the simple LED toggle delay
#if (CY_USE_PSVP == 1)
  #define CY_LED_TOGGLE_DELAY     (150000UL)
#else
  #if CY_CPU_CORTEX_M0P
    #define CY_LED_TOGGLE_DELAY     (400000UL * 6)
  #else
    #define CY_LED_TOGGLE_DELAY     (400000UL * 12)
  #endif
#endif

#include "cpu/cy_cpu.h"
#include "gpio/cy_gpio.h"
#include "ipc/cy_ipc_drv.h"
#include "ipc/cy_ipc_pipe.h"
#include "ipc/cy_ipc_sema.h"
#include "ipc/cy_ipc_config.h"
#include "mcwdt/cy_mcwdt.h"
#include "prot/cy_prot.h"
#include "scb/cy_scb_common.h"
#include "scb/cy_scb_ezi2c.h"
#include "scb/cy_scb_i2c.h"
#include "scb/cy_scb_spi.h"
#include "scb/cy_scb_uart.h"
#include "sysint/cy_sysint.h"
#include "sysclk/cy_sysclk.h"
#include "syswdt/cy_syswdt.h"
#include "sysrtc/cy_sysrtc.h"
#include "tcpwm/cy_tcpwm.h"
#include "tcpwm/cy_tcpwm_counter.h"
#include "tcpwm/cy_tcpwm_pwm.h"
#include "tcpwm/cy_tcpwm_quaddec.h"
#include "tcpwm/cy_tcpwm_sr.h"
#include "syspm/cy_syspm.h"
#include "syslib/cy_syslib.h"
#include "trigmux/cy_trigmux.h"
#include "systick/cy_systick.h"
#include "mpu/cy_mpu.h"
#include "dma/cy_pdma.h"
#include "dma/cy_mdma.h"
#include "adc/cy_adc.h"
#include "crypto/cy_crypto.h"
#include "crypto/cy_crypto_server.h"
#include "lin/cy_lin.h"
#include "canfd/cy_canfd.h"
#include "evtgen/cy_evtgen.h"
#include "flash/cy_flash.h"
#include "sysflt/cy_sysflt.h"
#include "lvd/cy_lvd.h"
#include "smartio/cy_smartio.h"
#include "sysreset/cy_sysreset.h"
#include "srom/cy_srom.h"
  
#if defined (tviibe2m) || defined (tviic2d6m) || defined (tviic2d4m)
    #include "cxpi/cy_cxpi.h"
#endif

#if defined (tviibh8m)    
    #include "flexray/cy_fr.h"
    #include "sd_host/cy_sd_host.h"
    #include "audioss/cy_i2s.h"
    #include "smif/cy_smif.h"
    #include "smif/cy_smif_memslot.h"
    #include "ethernet/cy_ethif.h"
    #include "sysreghc/cy_sysreghc.h"
#endif

#if defined (tviibh4m)
    #include "sd_host/cy_sd_host.h"
    #include "audioss/cy_i2s.h"
    #include "smif/cy_smif.h"
    #include "smif/cy_smif_memslot.h"
    #include "ethernet/cy_ethif.h"
    #include "sysreghc/cy_sysreghc.h"
#endif

#if defined (tviic2d6m)
    #include "audioss/mixer/cy_mixer.h"
    #include "audioss/i2s/cy_i2s.h"
    #include "audioss/pwm/cy_pwm.h"
    #include "audioss/dac/cy_dac.h"
    #include "smif/cy_smif.h"
    #include "smif/cy_smif_memslot.h"
    #include "audioss/tdm/cy_tdm.h"
    #include "audioss/sg/cy_sg.h"
    #include "fpdlink/cy_fpdlink.h"
    #include "mipicsi2/cy_mipicsi2.h"    
    #include "syspmic/cy_syspmic.h"
    #include "ethernet/cy_ethif.h"
#endif
  
#if defined (tviic2d4m)
    #include "audioss/mixer/cy_mixer.h"
    #include "audioss/pwm/cy_pwm.h"
    #include "audioss/dac/cy_dac.h"
    #include "axidma/cy_axidma.h"
    #include "smif/cy_smif.h"
    #include "smif/cy_smif_memslot.h"
    #include "audioss/tdm/cy_tdm.h"
    #include "audioss/sg/cy_sg.h"
    #include "fpdlink/cy_fpdlink.h"
    #include "mipicsi2/cy_mipicsi2.h"    
    #include "syspmic/cy_syspmic.h"
    #include "ethernet/cy_ethif.h"
#endif

//=============================================

#if defined (tviibe512k)
    #include "bb_bsp_tviibe512k.h"
#elif defined (tviibe1m)
    #include "bb_bsp_tviibe1m.h"
#elif defined (tviibe2m)
    #include "bb_bsp_tviibe2m.h"
#elif defined (tviibe4m)
    #include "bb_bsp_tviibe4m.h"
#elif defined (tviibh4m)
    #include "bb_bsp_tviibh4m.h"
#elif defined (tviibh8m)
    #include "bb_bsp_tviibh8m.h"
#elif defined (tviic2d4m)
    #include "bb_bsp_tviic2d4m.h"
#elif defined (tviic2d6m)
    #include "bb_bsp_tviic2d6m.h"
#endif

//============ Middleware =====================

//#include "button/cy_button.h"
//#include "semihosting/cy_semihosting.h"
#include "cy_sw_tmr.h" 
#include "cy_mw_flash.h" 

#if defined (tviibh8m) || defined (tviibh4m)
    #include "cy_power.h"
//    #include "mw/smif_mem/cy_smif_device_common.h"
//    #include "mw/smif_mem/cy_smif_hb_flash.h"
//    #include "mw/smif_mem/cy_smif_s25fl.h"
//    #include "mw/smif_mem/cy_smif_semp.h"
//    #include "mw/smif_mem/cy_smif_s25fs.h"
//    #include "aic261/cy_aic261.h"
#endif
#if defined (tviic2d4m) || defined (tviic2d6m)
    #include "mw/gfx_env/cy_gfx_env.h"
    #include "mw/smif_mem/cy_smif_device_common.h"
    #include "mw/smif_mem/cy_smif_hb_flash.h"
    #include "mw/smif_mem/cy_smif_s25fl.h"
    #include "mw/smif_mem/cy_smif_semp.h"
    #include "mw/smif_mem/cy_smif_s25fs.h"
    #include "mw/power/cy_power.h"
    #include "mw/mipi_sensor/cy_mipi_sensor.h"
    #include "aic261/cy_aic261.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* CY_PROJECT_H */

/* [] END OF FILE */
