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


#include "cy_syslib.h"

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

#include "cy_cpu.h"
#include "cy_gpio.h"
#include "cy_ipc_drv.h"
#include "cy_ipc_pipe.h"
#include "cy_ipc_sema.h"
#include "cy_ipc_config.h"
#include "cy_mcwdt.h"
#include "cy_prot.h"
#include "cy_scb_common.h"
#include "cy_scb_ezi2c.h"
#include "cy_scb_i2c.h"
#include "cy_scb_spi.h"
#include "cy_scb_uart.h"
#include "cy_sysint.h"
#include "cy_sysclk.h"
#include "cy_syswdt.h"
#include "cy_sysrtc.h"
#include "cy_tcpwm.h"
#include "cy_tcpwm_counter.h"
#include "cy_tcpwm_pwm.h"
#include "cy_tcpwm_quaddec.h"
#include "cy_tcpwm_sr.h"
#include "cy_syspm.h"
#include "cy_syslib.h"
#include "cy_trigmux.h"
#include "cy_systick.h"
#include "cy_mpu.h"
#include "cy_pdma.h"
#include "cy_mdma.h"
#include "cy_adc.h"
#include "cy_crypto.h"
#include "cy_crypto_server.h"
#include "cy_lin.h"
#include "cy_canfd.h"
#include "cy_evtgen.h"
#include "cy_flash.h"
#include "cy_sysflt.h"
#include "cy_lvd.h"
#include "cy_smartio.h"
#include "cy_sysreset.h"
#include "cy_srom.h"
  
#if defined (tviibe2m) || defined (tviic2d6m) || defined (tviic2d4m)
    #include "cxpi/cy_cxpi.h"
#endif

#if defined (tviibh8m)    
    #include "flexray/cy_fr.h"
    #include "cy_sd_host.h"
    #include "cy_i2s.h"
    #include "cy_smif.h"
    #include "cy_smif_memslot.h"
    #include "cy_ethif.h"
    #include "cy_sysreghc.h"
#endif

#if defined (tviibh4m)
    #include "cy_sd_host.h"
    #include "cy_i2s.h"
    #include "cy_smif.h"
    #include "cy_smif_memslot.h"
    #include "cy_ethif.h"
    #include "cy_sysreghc.h"
#endif

#if defined (tviic2d6m)
    #include "mixer/cy_mixer.h"
    #include "i2s/cy_i2s.h"
    #include "pwm/cy_pwm.h"
    #include "dac/cy_dac.h"
    #include "cy_smif.h"
    #include "cy_smif_memslot.h"
    #include "tdm/cy_tdm.h"
    #include "sg/cy_sg.h"
    #include "fpdlink/cy_fpdlink.h"
    #include "mipicsi2/cy_mipicsi2.h"    
    #include "syspmic/cy_syspmic.h"
    #include "cy_ethif.h"
#endif
  
#if defined (tviic2d4m)
    #include "mixer/cy_mixer.h"
    #include "pwm/cy_pwm.h"
    #include "dac/cy_dac.h"
    #include "axidma/cy_axidma.h"
    #include "cy_smif.h"
    #include "cy_smif_memslot.h"
    #include "tdm/cy_tdm.h"
    #include "sg/cy_sg.h"
    #include "fpdlink/cy_fpdlink.h"
    #include "mipicsi2/cy_mipicsi2.h"    
    #include "syspmic/cy_syspmic.h"
    #include "cy_ethif.h"
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

// #include "cy_button.h"
// #include "cy_semihosting.h"
#include "cy_sw_tmr.h" 
#include "cy_mw_flash.h" 

#if defined (tviibh8m) || defined (tviibh4m)
    #include "cy_power.h"
    // #include "cy_smif_device_common.h"
    // #include "cy_smif_hb_flash.h"
    // #include "cy_smif_s25fl.h"
    // #include "cy_smif_semp.h"
    // #include "cy_smif_s25fs.h"
    //#include "cy_aic261.h"
#endif
#if defined (tviic2d4m) || defined (tviic2d6m)
    #include "gfx_env/cy_gfx_env.h"
    #include "cy_smif_device_common.h"
    #include "cy_smif_hb_flash.h"
    #include "cy_smif_s25fl.h"
    #include "cy_smif_semp.h"
    #include "cy_smif_s25fs.h"
    #include "cy_power.h"
    #include "cy_mipi_sensor.h"
    #include "aic261/cy_aic261.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* CY_PROJECT_H */

/* [] END OF FILE */
