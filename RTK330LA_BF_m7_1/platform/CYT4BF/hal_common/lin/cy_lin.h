/***************************************************************************//**
* \file lin.h
* \version 1.0
*
* \brief
* Provides an API declaration of the LIN driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_LIN_H)
#define CY_LIN_H

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 *****************************************************************************
 ** \defgroup LinGroup Local Interconnect Network (LIN)
 **
 ** \brief This section describes the interface for the Local Interconnect Network.
 **
 *****************************************************************************/
/** @{ */


/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/
/** Driver major version */
#define CY_LIN_DRV_VERSION_MAJOR    1

/** Driver minor version */
#define CY_LIN_DRV_VERSION_MINOR    0

/** LIN driver ID */
#define CY_LIN_ID CY_PDL_DRV_ID(0x37u)

/** LIN API status definition */
typedef enum
{
    CY_LIN_SUCCESS   = 0x00u,                                     /**< Returned successful */
    CY_LIN_BAD_PARAM = CY_LIN_ID | CY_PDL_STATUS_ERROR | 0x01u,   /**< Bad parameter was passed */
    CY_LIN_BUSY      = CY_LIN_ID | CY_PDL_STATUS_ERROR | 0x02u,   /**< Change settings while tx/rx on-going */
    CY_LIN_FRAME_NOT_RECEIVED = CY_LIN_ID | CY_PDL_STATUS_ERROR | 0x03u,   /**< No frame received */
} cy_en_lin_status_t;

/** Maximum ID */
#define LIN_ID_MAX                              (0x3Fu)
/** Maximum data length */
#define LIN_DATA_LENGTH_MAX                     (8u)
/** Minimum data length */
#define LIN_DATA_LENGTH_MIN                     (1u)
/** Maximum time out length */
#define LIN_TIMEOUT_LENGTH_MAX                  (255u)
/** Minimum wakeup peroid value = 250usec*/
#define LIN_WAKEUP_PERIOD_MIN                   (250u)
/** Maximum break_wakeup period bit length */
#define LIN_BREAK_WAKEUP_LENGTH_BITS_MAX        (31u)
/** LIN Master minimum break filed detection length */
#define LIN_MASTER_BREAK_FILED_LENGTH_MIN       (13u)

/**
* \addtogroup group_lin_cmd_macro LIN command type definition
* \{
* Specifies the parameter values passed to LIN command API
*/
#define LIN_CMD_TX_HEADER                   (LIN_CH_CMD_TX_HEADER_Msk)
#define LIN_CMD_TX_RESPONSE                 (LIN_CH_CMD_TX_RESPONSE_Msk)
#define LIN_CMD_RX_RESPONSE                 (LIN_CH_CMD_RX_RESPONSE_Msk)
#define LIN_CMD_TX_HEADER_TX_RESPONSE       (LIN_CH_CMD_TX_HEADER_Msk | LIN_CH_CMD_TX_RESPONSE_Msk)
#define LIN_CMD_TX_HEADER_RX_RESPONSE       (LIN_CH_CMD_TX_HEADER_Msk | LIN_CH_CMD_RX_RESPONSE_Msk)
#define LIN_CMD_RX_HEADER_RX_RESPONSE       (LIN_CH_CMD_RX_HEADER_Msk | LIN_CH_CMD_RX_RESPONSE_Msk)
#define LIN_CMD_TX_WAKEUP                   (LIN_CH_CMD_TX_WAKEUP_Msk)
/** \} group_lin_cmd_macro */

/**
* \addtogroup group_lin_intr_mask_macro LIN ALL error mask definition
* \{
* Specifies the mask value for interrupt status/mask
*/
#define LIN_INTR_TX_HEADER_DONE             (LIN_CH_INTR_TX_HEADER_DONE_Msk)
#define LIN_INTR_TX_RESPONSE_DONE           (LIN_CH_INTR_TX_RESPONSE_DONE_Msk)
#define LIN_INTR_TX_WAKEUP_DONE             (LIN_CH_INTR_TX_WAKEUP_DONE_Msk)
#define LIN_INTR_RX_HEADER_DONE             (LIN_CH_INTR_RX_HEADER_DONE_Msk)
#define LIN_INTR_RX_RESPONSE_DONE           (LIN_CH_INTR_RX_RESPONSE_DONE_Msk)
#define LIN_INTR_RX_WAKEUP_DONE             (LIN_CH_INTR_RX_WAKEUP_DONE_Msk)
#define LIN_INTR_RX_HEADER_SYNC_DONE        (LIN_CH_INTR_RX_HEADER_SYNC_DONE_Msk)
#define LIN_INTR_RX_NOISE_DETECT            (LIN_CH_INTR_RX_NOISE_DETECT_Msk)
#define LIN_INTR_TIMEOUT                    (LIN_CH_INTR_TIMEOUT_Msk)
#define LIN_INTR_TX_HEADER_BIT_ERROR        (LIN_CH_INTR_TX_HEADER_BIT_ERROR_Msk)
#define LIN_INTR_TX_RESPONSE_BIT_ERROR      (LIN_CH_INTR_TX_RESPONSE_BIT_ERROR_Msk)
#define LIN_INTR_RX_HEADER_FRAME_ERROR      (LIN_CH_INTR_RX_HEADER_FRAME_ERROR_Msk)
#define LIN_INTR_RX_HEADER_SYNC_ERROR       (LIN_CH_INTR_RX_HEADER_SYNC_ERROR_Msk)
#define LIN_INTR_RX_HEADER_PARITY_ERROR     (LIN_CH_INTR_RX_HEADER_PARITY_ERROR_Msk)
#define LIN_INTR_RX_RESPONSE_FRAME_ERROR    (LIN_CH_INTR_RX_RESPONSE_FRAME_ERROR_Msk)
#define LIN_INTR_RX_RESPONSE_CHECKSUM_ERROR (LIN_CH_INTR_RX_RESPONSE_CHECKSUM_ERROR_Msk)
#define LIN_INTR_ALL_ERROR_MASK_SLAVE       (LIN_CH_INTR_RX_NOISE_DETECT_Msk           |\
                                             LIN_CH_INTR_TIMEOUT_Msk                   |\
                                             LIN_CH_INTR_TX_RESPONSE_BIT_ERROR_Msk     |\
                                             LIN_CH_INTR_RX_HEADER_FRAME_ERROR_Msk     |\
                                             LIN_CH_INTR_RX_HEADER_SYNC_ERROR_Msk      |\
                                             LIN_CH_INTR_RX_HEADER_PARITY_ERROR_Msk    |\
                                             LIN_CH_INTR_RX_RESPONSE_FRAME_ERROR_Msk   |\
                                             LIN_CH_INTR_RX_RESPONSE_CHECKSUM_ERROR_Msk)
#define LIN_INTR_ALL_ERROR_MASK_MASTER      (LIN_INTR_ALL_ERROR_MASK_SLAVE | LIN_CH_INTR_TX_HEADER_BIT_ERROR_Msk)
/** \} group_lin_intr_mask_macro */

/*****************************************************************************/
/* Global type definitions ('typedef')                                       */
/*****************************************************************************/
/**
 *****************************************************************************
 ** \brief LIN break delimiter length
 **
 ** This configuration is effective only when corresponding channel = master mode.
 *****************************************************************************/
typedef enum en_lin_break_delimiter_length{
    LinBreakDelimiterLength1bits = 0,   /*!< 1-bit length */
    LinBreakDelimiterLength2bits = 1,   /*!< 2-bit length */
    LinBreakDelimiterLength3bits = 2,   /*!< 3-bit length */
    LinBreakDelimiterLength4bits = 3    /*!< 4-bit length */
}en_lin_break_delimiter_length_t;

/**
 *****************************************************************************
 ** \brief Stop bit selection.
 **
 *****************************************************************************/
typedef enum en_lin_stopbit
{
    LinOneStopBit           = 1,    /*!< 1 stop bit */
    LinTwoStopBit           = 3     /*!< 2 stop bits */
} en_lin_stopbit_t;

/**
 *****************************************************************************
 ** \brief Checksum type selection.
 **
 *****************************************************************************/
typedef enum en_lin_checksum_type
{
    LinChecksumTypeNormal = 0,      /*!< Normal (classic) checksum */
    LinChecksumTypeExtended = 1     /*!< Extended (enhanced) checksum */
}en_lin_checksum_type_t;

/**
 *****************************************************************************
 ** \brief timeout type selection.
 **
 *****************************************************************************/
typedef enum en_lin_timeout_sel_type
{
    LinTimeoutNone = 0,                     /*!< No timeout */
    LinTimeoutFromBreakToChecksum = 1,      /*!< Frame mode: from the start of break field to checksum field STOP bits */
    LinTimeoutFromBreakToPID = 2,           /*!< Frame header mode: detects timeout from the start of break field to PID field STOP bits */
    LinTimeoutFromResponseToChecksum = 3    /*!< Frame response mode: detects timeout from the first data field START bit to checksum field STOP bits. */
}en_lin_timeout_sel_type_t;

/**
 *****************************************************************************
 ** \brief LIN configuration
 **        This settings are per LIN instance.
 *****************************************************************************/
typedef struct stc_lin_config
{
    /** If TRUE, corresponding channel = master mode, If FALSE, slave mode. */
    bool                            bMasterMode;
    /** If TRUE, corresponding LIN channel transceiver is enabled automatically,
     ** If FALSE, firmware has to handle the transceiver enable signal manually */
    bool                            bLinTransceiverAutoEnable;
    /** Break field length. */
    uint8_t                         u8BreakFieldLength;
    /** Break delimiter length. See #en_lin_break_delimiter_length_t */
    en_lin_break_delimiter_length_t enBreakDelimiterLength;
    /** Stop bit length. See #en_lin_stopbit_t. */
    en_lin_stopbit_t                enStopBit;
    /** If TRUE, lin_rx_in filter operates.
     *  Median 3 operates on the last three "lin_rx_in" values.
     *  The sequences '000', '001', '010' and '100' result in a filtered value '0'.
     *  The sequences '111', '110', '101' and '011' result in a filtered value '1'.
     */
    bool                            bFilterEnable;
}stc_lin_config_t;

/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source)           */
/*****************************************************************************/


/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source)             */
/*****************************************************************************/
cy_en_lin_status_t Lin_DeInit( volatile stc_LIN_CH_t* pstcLin );
cy_en_lin_status_t Lin_Init( volatile stc_LIN_CH_t* pstcLin, const stc_lin_config_t *pstcConfig);
cy_en_lin_status_t Lin_ReadData( volatile stc_LIN_CH_t* pstcLin, uint8_t *u8Data);
cy_en_lin_status_t Lin_WriteData( volatile stc_LIN_CH_t* pstcLin, const uint8_t *au8Data,
                                  uint8_t u8DataLength );
cy_en_lin_status_t Lin_Enable(volatile stc_LIN_CH_t* pstcLin);
cy_en_lin_status_t Lin_Disable(volatile stc_LIN_CH_t* pstcLin);
cy_en_lin_status_t Lin_SetBreakWakeupFieldLength(volatile stc_LIN_CH_t* pstcLin, uint8_t length);
cy_en_lin_status_t Lin_SetDataLength(volatile stc_LIN_CH_t* pstcLin, uint8_t length);
cy_en_lin_status_t Lin_SetChecksumType(volatile stc_LIN_CH_t* pstcLin, en_lin_checksum_type_t type);
cy_en_lin_status_t Lin_SetCmd(volatile stc_LIN_CH_t* pstcLin, uint32_t command);

cy_en_lin_status_t Lin_SetHeader(volatile stc_LIN_CH_t* pstcLin, uint8_t id);
cy_en_lin_status_t Lin_GetHeader(volatile stc_LIN_CH_t* pstcLin, uint8_t *id, uint8_t *parity);
cy_en_lin_status_t Lin_SetInterruptMask(volatile stc_LIN_CH_t* pstcLin, uint32_t mask);
cy_en_lin_status_t Lin_GetInterruptMask(volatile stc_LIN_CH_t* pstcLin, uint32_t *mask);
cy_en_lin_status_t Lin_GetInterruptMaskedStatus(volatile stc_LIN_CH_t* pstcLin, uint32_t *status);
cy_en_lin_status_t Lin_GetInterruptStatus(volatile stc_LIN_CH_t* pstcLin, uint32_t *status);
cy_en_lin_status_t Lin_ClearInterrupt(volatile stc_LIN_CH_t* pstcLin, uint32_t mask);
cy_en_lin_status_t Lin_GetStatus(volatile stc_LIN_CH_t* pstcLin, uint32_t *status);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __CY_LIN_H__ */
/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
