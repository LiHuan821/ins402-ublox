/** 
 * @file hwAPI.h
 *
 */

#ifndef HAL_API_H
#define HAL_API_H

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>
#include "constants.h"

/**********************************************
* @brief
*
***********************************************/
void    HW_Init();

// GPIO - related fucntions
/**********************************************
* @brief
*
***********************************************/
void    HW_TP1_Toggle();

/**********************************************
* @brief
*
***********************************************/
void    HW_TP2_Toggle();

/**********************************************
* @brief
*
***********************************************/
void    HW_TP1_On();

/**********************************************
* @brief
*
***********************************************/
void    HW_TP2_On();

/**********************************************
* @brief
*
***********************************************/
void    HW_TP1_Off();

/**********************************************
* @brief
*
***********************************************/
void    HW_TP2_Off();

/**********************************************
* @brief
*
***********************************************/
void    HW_EnableCANTransceiver();

/**********************************************
* @brief
*
***********************************************/
void    HW_DisableCANTransceiver();

/**********************************************
* @brief
*
***********************************************/
void    HW_ConfigureCANInterface();

/**********************************************
* @brief
*
***********************************************/
void    HW_InitWatchdod();

/**********************************************
* @brief
*
***********************************************/
void HW_FeedWatchdog();

//
//// UART - related functions
///**********************************************
//* @brief
//*
//* @param channel --
//* @param baudrate --
//* @return int32_t
//***********************************************/
//int32_t UART_Init(int32_t channel, uint32_t baudrate);
//
///**********************************************
//* @brief
//*
//* @param channel --
//* @param data --
//* @param len --
//* @return int32_t
//***********************************************/
//int32_t UART_Read(int32_t channel, uint8_t *data, uint32_t len);
//
///**********************************************
//* @brief
//*
//* @param channel --
//* @param data --
//* @param len --
//* @return int32_t
//***********************************************/
//int32_t UART_Write(int32_t channel, uint8_t *data, uint32_t  len);


// Timers -related functions
/**********************************************
* @brief
*
* @return uint8_t
***********************************************/
uint8_t        TIMER_IsDacqOverrun();

/**********************************************
* @brief
*
* @return
***********************************************/
void     TIMER_WaitForNewDacqTick();


/**********************************************
* @brief
*
* @return uint64_t
***********************************************/
uint64_t TIMER_GetCurrTimeStamp();

/**********************************************
* @brief
*
* @return uint64_t
***********************************************/
uint64_t TIMER_GetDacqTimeStamp();

/**********************************************
* @brief
*
* @param interval --
***********************************************/
void     TIMER_SetDacqInterval(uint32_t interval);

/**********************************************
* @brief
*
***********************************************/
void     TIMER_InitOSTick();

/**********************************************
* @brief
*
* @param Delay --
***********************************************/
void     TIMER_DelayMs(uint32_t Delay);


/**********************************************
* @brief
*
* @return uint16_t
***********************************************/
uint16_t TIMER_GetRollingCount(void);

/*******************************************
 * @brief
 *
********************************************/
void TIMER_Init();


// system related functions
/**********************************************
* @brief
*
***********************************************/
void HW_SystemReset(void);


/**********************************************
* @brief
*
***********************************************/
void HW_EnforceSerialBootMode(void);


/**********************************************
* @brief
*
* @return uint32_t
***********************************************/
uint32_t HW_GetResetRootCause(void);

/**********************************************
* @brief
*
***********************************************/
void HW_FillStackPattern(void);


/**********************************************
* @brief
*
* @return uint8_t
***********************************************/
uint8_t HW_IsStackOverflow(void);

/**********************************************
* @brief
*
* @param cause ---
***********************************************/
void HW_SetResetRootCause(uint8_t cause);

enum {
    NUM_SERIAL_PORTS = 1
};

///*******************************************
// * @brief
// *
//********************************************/
//void HW_InitADC(void);
//
///*******************************************
// * @brief
// *
//********************************************/
//void HW_StartADC(void);


/*******************************************
 * @brief
 *
 * @return uint8_t
********************************************/
uint8_t HW_IsADCConversionComplete(void);

/*******************************************
 * @brief
 *
 * @param voltages ==
********************************************/
void HW_GetADCReadings(float32_t voltages[]);

/*******************************************
 * @brief
 *
 * @return uint8_t
********************************************/
uint8_t HW_ReadUnitHwConfiguration(void);



extern uint32_t	const _ebss;
extern uint32_t const _Min_Heap_Size;
extern uint32_t const _estack;

#ifdef __cplusplus
}
#endif


#endif //__UART_H
