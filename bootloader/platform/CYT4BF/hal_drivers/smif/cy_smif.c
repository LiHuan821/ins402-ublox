/***************************************************************************//**
* \file cy_smif.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* \brief
*  This file provides the source code for the SMIF driver APIs.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_smif.h"

#ifdef CY_IP_MXSMIF

#ifdef CY_SMIF_NEW_VERSION
  #undef CRYPTO
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
* Function Name: Cy_SMIF_Init
****************************************************************************//**
*
* This function initializes the SMIF block as a communication block. The user
* must ensure that the SMIF interrupt is disabled while this function
* is called. Enabling the interrupts can lead to triggering in the middle
* of the initialization operation, which can lead to erroneous initialization.
*
* As parameters, this function takes the SMIF register base address and a
* context structure along with the configuration needed for the SMIF block,
* stored in a config
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param config
* Passes a configuration structure that configures the SMIF block for operation.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \param timeout
* A timeout in microseconds for blocking APIs in use.
*
*
* \note Make sure that the interrupts are initialized and disabled.
*
* \return
*     - \ref CY_SMIF_BAD_PARAM
*     - \ref CY_SMIF_SUCCESS
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Init(volatile stc_SMIF_t *base, 
                                    cy_stc_smif_config_t const *config,
                                    uint32_t timeout,
                                    cy_stc_smif_context_t *context)
{
    if((NULL == base) || (NULL == config) || (NULL == context))
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(config->dlpAuto == CY_SMIF_DLP_UPDATE_AUTO)
    {
        if((config->rxClk != CY_SMIF_INTERNAL_CLK) && (config->rxClk != CY_SMIF_INV_INTERNAL_CLK))
        {
            return CY_SMIF_BAD_PARAM;
        }

        if(config->delaySel == CY_SMIF_NO_DELAY_SEL)
        {
            return CY_SMIF_BAD_PARAM;
        }
    }

    /* Global variables initialization */
    context->txBufferAddress = NULL;
    context->txBufferCounter = 0;
    context->rxBufferAddress = NULL;
    context->rxBufferCounter = 0;
    context->transferStatus  = CY_SMIF_STARTED;
    context->txCmpltCb       = NULL;
    context->rxCmpltCb       = NULL;
    context->preCmdDataRate  = CY_SMIF_SDR;
    context->preCmdWidth     = CY_SMIF_WIDTH_SINGLE;
    context->txCmdBufCounter = 0;

    /* Configure the initial interrupt mask */
    /* Disable the TR_TX_REQ and TR_RX_REQ interrupts */
    un_SMIF_INTR_MASK_t tempIntrMask;
    tempIntrMask.u32Register = base->unINTR_MASK.u32Register;
    tempIntrMask.stcField.u1TR_TX_REQ = 0u;
    tempIntrMask.stcField.u1TR_RX_REQ = 0u;
    base->unINTR_MASK.u32Register = tempIntrMask.u32Register;

    /* Configure the SMIF interface */
    un_SMIF_CTL_t tempCtl;
    tempCtl.u32Register = base->unCTL.u32Register;
    tempCtl.stcField.u1CLOCK_IF_TX_SEL          = config->txClk;
    tempCtl.stcField.u3CLOCK_IF_RX_SEL          = config->rxClk;
    if(config->delaySel == CY_SMIF_NO_DELAY_SEL)
    {
        tempCtl.stcField.u1DELAY_TAP_ENABLED    = 0u;
    }
    else
    {
        tempCtl.stcField.u1DELAY_TAP_ENABLED    = 1u;
        tempCtl.stcField.u3DELAY_LINE_SEL       = config->delaySel;
    }
    tempCtl.stcField.u1INT_CLOCK_DL_ENABLED     = (uint8_t)config->dlpAuto;
    tempCtl.stcField.u1XIP_MODE                 = config->mode;
    tempCtl.stcField.u2INT_CLOCK_CAPTURE_CYCLE  = config->capDelay;
    tempCtl.stcField.u3DESELECT_DELAY           = config->deselectDelay;
    tempCtl.stcField.u2SELECT_SETUP_DELAY       = config->setupDelay;
    tempCtl.stcField.u2SELECT_HOLD_DELAY        = config->holdDelay;
    tempCtl.stcField.u1BLOCK                    = config->blockEvent;
#if defined (tviic2d6m) || defined (tviic2d4m)
    tempCtl.stcField.u1CLOCK_IF_SEL             = config->clkIfSrc;
#endif
    base->unCTL.u32Register = tempCtl.u32Register;

    /* Read the register to flush the buffer */
    (void) base->unCTL.u32Register;

    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeInit
****************************************************************************//**
*
* This function de-initializes the SMIF block to default values.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \note The SMIF must be disabled before calling the function. Call
*  \ref Cy_SMIF_Disable
*
*******************************************************************************/
void Cy_SMIF_DeInit(volatile stc_SMIF_t *base)
{
    /* Configure the SMIF interface to default values. 
    * The default value is 0. 
    */
    base->unCTL.u32Register = CY_SMIF_CTL_REG_DEFAULT;
    base->unTX_DATA_FIFO_CTL.u32Register      = 0U;
    base->unRX_DATA_MMIO_FIFO_CTL.u32Register = 0U;
    base->unINTR_MASK.u32Register             = 0U;
    for(uint32_t idx = 0UL; idx < CY_SMIF_GetDeviceNumber(base); idx++)
    {
        base->DEVICE[idx].unCTL.u32Register = 0U;
        /* Read the register to flush the buffer */
        (void) base->DEVICE[idx].unCTL.u32Register;
    }
}

/*******************************************************************************
* Function Name: Cy_SMIF_PUSH_TX_Command()
****************************************************************************//**
*
* 
*  If the last bit of input command value is "0" (not last),
*  This function push input tx command value to internal buffer.
*  If the last bit of input command value is "1" (last),
*  This function push all accumulated tx command value to TX_CMD_FIFO.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cmdData
* The input command data
*
* \param ctx
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
*******************************************************************************/
static void Cy_SMIF_PUSH_TX_Command(volatile stc_SMIF_t *base, cy_un_smif_cmd_fifo_t cmdData, cy_stc_smif_context_t *ctx)
{
#if 1
    #define CY_TX_CMD_BUF_MAX (64)
    static cy_un_smif_cmd_fifo_t TxCmdBuf[CY_TX_CMD_BUF_MAX];

    TxCmdBuf[ctx->txCmdBufCounter] = cmdData;
    ctx->txCmdBufCounter++;
    CY_ASSERT_L1(ctx->txCmdBufCounter <= CY_TX_CMD_BUF_MAX); // buffer over flow

    if((cmdData.fieldTx.last == 1)                               ||
       (cmdData.fieldTx.cmd  == CY_SMIF_CMD_FIFO_DESELECT_MODE))
    {
        uint32_t pushWord = 0;
        for(uint32_t remaindWord = ctx->txCmdBufCounter; remaindWord > 0; remaindWord -= pushWord)
        {
            uint32_t progressWord = ctx->txCmdBufCounter - remaindWord;
            uint32_t emptyWord   = CY_SMIF_TX_CMD_FIFO_STATUS_RANGE - base->unTX_CMD_FIFO_STATUS.stcField.u4USED4;

            pushWord    = (remaindWord < emptyWord) ? remaindWord
                                                    : emptyWord;
            uint32_t start = progressWord;
            uint32_t end   = progressWord+pushWord;
            for(uint32_t i = start; i < end; i++)
            {
                base->unTX_CMD_FIFO_WR.u32Register = TxCmdBuf[i].u32;
            }
        }
        ctx->txCmdBufCounter = 0;
    }
#else // for debug
    base->unTX_CMD_FIFO_WR.u32Register = cmdData.u32;
#endif
}

/*******************************************************************************
* Function Name: Cy_SMIF_TransmitCommand()
****************************************************************************//**
*
* This function transmits a command byte followed by a parameter which is
* typically an address field. The transfer is implemented using the TX FIFO.
* This function also asserts the slave select line.
* A command to a memory device generally starts with a command byte
* transmission. This function sets up the slave lines for the rest of the
* command structure. The \ref Cy_SMIF_TransmitCommand is called before \ref
* Cy_SMIF_TransmitData or \ref Cy_SMIF_ReceiveData is called. When enabled, the
* cmpltTxfr parameter in the function will de-assert the slave select line at 
* the end of the function execution.
*
* \note This function blocks until all the command and associated parameters
* have been transmitted over the SMIF block or timeout expire.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cmd
* The command byte to be transmitted.
*
* \param isCommand2Byte
* true: command is 2 byte. false: command is 1 byte.
*
* \param cmdTxfrWidth
* The width of command byte transfer \ref cy_en_smif_txfr_width_t.
*
* \param cmdDataRate
* The data rate of command byte transfer \ref cy_en_smif_data_rate_t.
*
* \param cmdParam
* This is the pointer to an array that has bytes to be transmitted
* after the command byte. Typically, this field has the address bytes
* associated with the memory command.
*
* \param paramSize
* The size of the cmdParam array.
*
* \param paramTxfrWidth
* The width of parameter transfer \ref cy_en_smif_txfr_width_t.
*
* \param paramDataRate
* The data rate of parameter transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param cmpltTxfr
* Specifies if the slave select line must be de-asserted after transferring
* the last byte in the parameter array. Typically, this field is set to 0 when
* this function succeed through \ref Cy_SMIF_TransmitData or \ref
* Cy_SMIF_ReceiveData.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of the command transmit.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_TransmitCommand(volatile stc_SMIF_t *base,
                                uint16_t cmd,
                                bool isCommand2Byte,
                                cy_en_smif_txfr_width_t cmdTxfrWidth,
                                cy_en_smif_data_rate_t  cmdDataRate,
                                uint8_t const cmdParam[],
                                uint32_t paramSize,
                                cy_en_smif_txfr_width_t paramTxfrWidth,
                                cy_en_smif_data_rate_t  paramDataRate,
                                cy_en_smif_slave_select_t  slaveSelect,
                                cy_en_smif_cmd_last_t cmpltTxfr,
                                cy_stc_smif_context_t *context)
{
    /* Check input values */
    // If input parameter size is not zero, but parameter pointer is NULL. then it return with error status
    if((paramSize != 0x0UL) && (cmdParam == NULL))
    {
        return CY_SMIF_BAD_PARAM;
    }


    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    cy_un_smif_cmd_fifo_t cmdData = { .u32 = 0 };

    /* Prepare a cmd fifo data */
    if(isCommand2Byte == true)
    {
        if((cmdTxfrWidth == CY_SMIF_WIDTH_OCTAL) && (cmdDataRate == CY_SMIF_DDR))
        {
            // 2byte for each one command
            cmdData.fieldTx.cmd           = CY_SMIF_CMD_FIFO_TX_MODE;
            cmdData.fieldTx.firstByte     = (uint8_t)(cmd & 0x00FF);
            cmdData.fieldTx.secondByte    = (uint8_t)((cmd >> 8) & 0x00FF);
            cmdData.fieldTx.width         = cmdTxfrWidth;
            cmdData.fieldTx.dataRate      = cmdDataRate;
            cmdData.fieldTx.last          = (0UL == paramSize) ? cmpltTxfr : 0UL;
            cmdData.fieldTx.selectDev     = slaveSelect;
            /* Send the command byte */
            Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
            cmdData.u32 = 0x0ul;
        }
        else
        {
            // 1byte for each one command. need to send two command to send a command of 2byte.
            cmdData.fieldTx.cmd           = CY_SMIF_CMD_FIFO_TX_MODE;
            cmdData.fieldTx.firstByte     = (uint8_t)((cmd >> 8) & 0x00FF);
            cmdData.fieldTx.secondByte    = 0u;
            cmdData.fieldTx.width         = cmdTxfrWidth;
            cmdData.fieldTx.dataRate      = cmdDataRate;
            cmdData.fieldTx.last          = 0;
            cmdData.fieldTx.selectDev     = slaveSelect;
            /* Send the command byte */
            Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
            cmdData.u32 = 0x0ul;

            cmdData.fieldTx.cmd           = CY_SMIF_CMD_FIFO_TX_MODE;
            cmdData.fieldTx.firstByte     = (uint8_t)(cmd & 0x00FF);
            cmdData.fieldTx.secondByte    = 0u;
            cmdData.fieldTx.width         = cmdTxfrWidth;
            cmdData.fieldTx.dataRate      = cmdDataRate;
            cmdData.fieldTx.last          = (0UL == paramSize) ? cmpltTxfr : 0UL;
            cmdData.fieldTx.selectDev     = slaveSelect;
            /* Send the command byte */
            Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
            cmdData.u32 = 0x0ul;
        }
    }
    else
    {
        cmdData.fieldTx.cmd           = CY_SMIF_CMD_FIFO_TX_MODE;
        cmdData.fieldTx.firstByte     = (uint8_t)(cmd & 0x00FF);
        cmdData.fieldTx.secondByte    = 0u;
        cmdData.fieldTx.width         = cmdTxfrWidth;
        cmdData.fieldTx.dataRate      = cmdDataRate;
        cmdData.fieldTx.last          = (0UL == paramSize) ? cmpltTxfr : 0UL;
        cmdData.fieldTx.selectDev     = slaveSelect;
        /* Send the command byte */
        Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
        cmdData.u32 = 0x0ul;
    };


    /* Send the command parameters (usually address) in the blocking mode */
    uint8_t bufIndex           = 0U;
    uint32_t timeoutUnits      = context->timeout;
    cy_en_smif_status_t result = CY_SMIF_SUCCESS;
    cmdData.fieldTx.cmd        = CY_SMIF_CMD_FIFO_TX_MODE;
    cmdData.fieldTx.selectDev  = slaveSelect;

    if((paramTxfrWidth == CY_SMIF_WIDTH_OCTAL) && (paramDataRate == CY_SMIF_DDR))
    {
        // 2 byte transmission for each one command.
        while ((bufIndex < paramSize) && (CY_SMIF_EXCEED_TIMEOUT != result))
        {
            /* Check if there is at least one free entry in TX_CMD_FIFO */
            if  (Cy_SMIF_GetCmdFifoStatus(base) < CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
            {
                cmdData.fieldTx.firstByte = cmdParam[bufIndex+1];
                cmdData.fieldTx.secondByte= cmdParam[bufIndex];
                cmdData.fieldTx.width     = paramTxfrWidth;
                cmdData.fieldTx.dataRate  = paramDataRate;
                cmdData.fieldTx.last      = ((((uint32_t)bufIndex + 2UL) < paramSize) ? 0UL : cmpltTxfr);
                Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
                bufIndex += 2;
            }
            result = Cy_SMIF_TimeoutRun(&timeoutUnits);
        }
    }
    else
    {
        while ((bufIndex < paramSize) && (CY_SMIF_EXCEED_TIMEOUT != result))
        {
            /* Check if there is at least one free entry in TX_CMD_FIFO */
            if  (Cy_SMIF_GetCmdFifoStatus(base) < CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
            {
                cmdData.fieldTx.firstByte = cmdParam[bufIndex];
                cmdData.fieldTx.secondByte= 0x00u;
                cmdData.fieldTx.width     = paramTxfrWidth;
                cmdData.fieldTx.dataRate  = paramDataRate;
                cmdData.fieldTx.last      = ((((uint32_t)bufIndex + 1UL) < paramSize) ? 0UL : cmpltTxfr);
                Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
                bufIndex++;
            }
            result = Cy_SMIF_TimeoutRun(&timeoutUnits);
        }
    }

    return (result);
}

/*******************************************************************************
* Function Name: Cy_SMIF_TransmitData
****************************************************************************//**
*
* This function is used to transmit data using the SMIF interface. This
* function uses the TX Data FIFO to implement the transmit functionality. The
* function sets up an interrupt to trigger the TX Data FIFO and uses that
* interrupt to fill the TX Data FIFO until all the data is transmitted. At the
* end of the transmission, the TxCmpltCb is executed.
*
* \note  This function is to be preceded by \ref Cy_SMIF_TransmitCommand where
* the slave select is selected. The slave is de-asserted at the end of a
* transmit. The function triggers the transfer and the transfer itself utilizes
* the interrupt for FIFO operations in the background. Thus, frequent
* interrupts will be executed after this function is triggered.
* Since this API is non-blocking and sets up the interrupt to act on the data
* FIFO, ensure there will be no another instance of the function called
* before the current instance has completed execution.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param txBuffer
* The pointer to the data to be transferred. If this pointer is a NULL, then the
* function does not enable the interrupt. This use case is typically used when 
* the FIFO is handled outside the interrupt and is managed in either a 
* polling-based code or a DMA. The user would handle the FIFO management in a
* DMA or a polling-based code.
*
* \note If the user provides a NULL pointer in this function and does not handle
* the FIFO transaction, this could either stall or timeout the operation.
* The transfer statuses returned by \ref Cy_SMIF_GetTxfrStatus are no longer 
* valid.
* 
* \param size
* The size of txBuffer. Must be > 0.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataDataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param TxCmpltCb
* The callback executed at the end of a transmission. NULL interpreted as no
* callback.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of a transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_TransmitData(volatile stc_SMIF_t *base,
                            const uint8_t* txBuffer,
                            uint32_t size,
                            cy_en_smif_txfr_width_t transferWidth,
                            cy_en_smif_data_rate_t  dataDataRate,
                            cy_en_smif_slave_select_t  slaveSelect,
                            cy_smif_event_cb_t TxCmpltCb,
                            cy_stc_smif_context_t *context)
{
    uint32_t trUnitNum;

    /* Check input values */
    if(size == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(txBuffer == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    /* If the mode is octal SPI with DDR or Hyperbus, data unit is a 2-byte */
    if((transferWidth == CY_SMIF_WIDTH_OCTAL) && (dataDataRate == CY_SMIF_DDR))
    {
        if(size % 2 != 0)
        {
            return CY_SMIF_BAD_PARAM;
        }
        trUnitNum = size / 2;
    }
    else
    {
        trUnitNum = size;
    }

    /* Prepare a cmd fifo data */
    cy_un_smif_cmd_fifo_t cmdData  = 
    {
        .fieldTxCount.cmd       = CY_SMIF_CMD_FIFO_TX_COUNT_MODE,
        .fieldTxCount.width     = (uint32_t)transferWidth,
        .fieldTxCount.dataNum   = (uint32_t)(trUnitNum - 1U),
        .fieldTxCount.dataRate  = (uint32_t)dataDataRate,
        .fieldTxCount.last      = (uint32_t)LAST_COMMAND_BYTE,
        .fieldTxCount.selectDev = slaveSelect,
    };

    /* Move the parameters to the global variables */
    context->txBufferAddress       = (uint8_t*)txBuffer;
    context->txBufferCounter       = size;
    context->txCmpltCb             = TxCmpltCb;
    context->transferStatus        = CY_SMIF_SEND_BUSY;
    context->preCmdDataRate        = dataDataRate;
    context->preCmdWidth           = transferWidth;

    /* Enter the transmitting mode */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
    /* Enable the TR_TX_REQ interrupt */
    base->unINTR_MASK.stcField.u1TR_TX_REQ = 1u;

    return (CY_SMIF_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SMIF_TransmitDataBlocking
****************************************************************************//**
*
* This function implements the transmit data phase in the memory command. The
* data is transmitted using the Tx Data FIFO and the TX_COUNT command. This
* function blocks until completion. The function does not use the interrupts and
* will use CPU to monitor the FIFO status and move data accordingly. The
* function returns only on completion.
*
* \note  Since this API is blocking, ensure that other transfers finished and it
* will not be called during non-blocking transfer.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param txBuffer
* The pointer to the data to be transferred. If this pointer is a NULL, then the
* function does not fill TX_FIFO. The user would handle the FIFO management in a
* DMA or a polling-based code.
*
* \note If the user provides a NULL pointer in this function and does not handle
* the FIFO transaction, this could either stall or timeout the operation.
* The transfer statuses returned by \ref Cy_SMIF_GetTxfrStatus are no longer
* valid.
*
* \param size
* The size of txBuffer. Must be > 0.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataDataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of a transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_TransmitDataBlocking(volatile stc_SMIF_t *base,
                            const uint8_t *txBuffer,
                            uint32_t size,
                            cy_en_smif_txfr_width_t transferWidth,
                            cy_en_smif_data_rate_t  dataDataRate,
                            cy_en_smif_slave_select_t  slaveSelect,
                            cy_stc_smif_context_t *context)
{
    uint32_t trUnitNum;

    /* Check input values */
    if(size == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(txBuffer == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    if(Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    /* If the mode is octal SPI with DDR or Hyperbus, data unit is a 2-byte */
    if((transferWidth == CY_SMIF_WIDTH_OCTAL) && (dataDataRate == CY_SMIF_DDR))
    {
        if(size % 2 != 0)
        {
            return CY_SMIF_BAD_PARAM;
        }
        trUnitNum = size / 2;
    }
    else
    {
        trUnitNum = size;
    }

    /* Prepare a cmd fifo data */
    cy_un_smif_cmd_fifo_t cmdData  =
    {
        .fieldTxCount.cmd       = CY_SMIF_CMD_FIFO_TX_COUNT_MODE,
        .fieldTxCount.width     = (uint32_t)transferWidth,
        .fieldTxCount.dataNum   = (uint32_t)(trUnitNum - 1U),
        .fieldTxCount.dataRate  = (uint32_t)dataDataRate,
        .fieldTxCount.last      = (uint32_t)LAST_COMMAND_BYTE,
        .fieldTxCount.selectDev = slaveSelect,
    };

    /* initialize parameters for Cy_SMIF_PushTxFifo */
    cy_stc_smif_context_t contextLoc =
    {
        .txBufferAddress     = (uint8_t*)txBuffer,
        .txBufferCounter     = size,
        .txCmpltCb           = NULL,
        .transferStatus      = CY_SMIF_SEND_BUSY,
        .preCmdDataRate      = dataDataRate,
        .preCmdWidth         = transferWidth,
    };

    /* Enter the transmitting mode */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    uint32_t timeoutUnits = context->timeout;
    cy_en_smif_status_t result = CY_SMIF_SUCCESS;
    while ((CY_SMIF_SEND_BUSY == contextLoc.transferStatus) &&
           (CY_SMIF_EXCEED_TIMEOUT != result))
    {
        Cy_SMIF_PushTxFifo(base, &contextLoc);
        result = Cy_SMIF_TimeoutRun(&timeoutUnits);
    }

    return (result);
}


/*******************************************************************************
* Function Name: Cy_SMIF_ReceiveDataCmd
****************************************************************************//**
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param rxBuffer
* The pointer to the variable where the receive data is stored. If this pointer
* is a NULL, then the function does not enable the interrupt. This use case is 
* typically used when the FIFO is handled outside the interrupt and is managed 
* in either a polling-based code or a DMA. The user would handle the FIFO 
* management in a DMA or a polling-based code.
*
* \note If the user provides a NULL pointer in this function and does not handle
* the FIFO transaction, this could either stall or timeout the operation.
* The transfer statuses returned by \ref Cy_SMIF_GetTxfrStatus are no longer 
* valid.
*
* \param size
* The size of data to be received. Must be > 0.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param RxCmpltCb
* The callback executed at the end of a reception. NULL interpreted as no
* callback.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of a reception.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_ReceiveDataCmd(volatile stc_SMIF_t *base,
                            uint8_t *rxBuffer,
                            uint32_t size,
                            cy_en_smif_txfr_width_t transferWidth,
                            cy_en_smif_data_rate_t  dataRate,
                            cy_en_smif_slave_select_t  slaveSelect,
                            cy_smif_event_cb_t RxCmpltCb,
                            cy_stc_smif_context_t *context)
{
    uint32_t rxUnitNum;

    /* Check input values */
    if(size == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(rxBuffer == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* If the mode is octal SPI with DDR or Hyperbus, data unit is a 2-byte */
    if((transferWidth == CY_SMIF_WIDTH_OCTAL) && (dataRate == CY_SMIF_DDR))
    {
        if(size % 2 != 0)
        {
            return CY_SMIF_BAD_PARAM;
        }
        rxUnitNum = size / 2;
    }
    else
    {
        rxUnitNum = size;
    }

    /* Prepare a cmd fifo data */
    cy_un_smif_cmd_fifo_t cmdData  =
    {
        .fieldRxCount.cmd       = CY_SMIF_CMD_FIFO_RX_COUNT_MODE,
        .fieldRxCount.width     = (uint32_t)transferWidth,
        .fieldRxCount.dataRate  = (uint32_t)dataRate,
        .fieldRxCount.dataNum   = (uint32_t)(rxUnitNum - 1U),
        .fieldRxCount.last      = (uint32_t)LAST_COMMAND_BYTE,
        .fieldRxCount.selectDev = slaveSelect,
    };

    /* Enter the receiving mode */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    return (CY_SMIF_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SMIF_ReceiveData
****************************************************************************//**
*
* This function implements the receive data phase in the memory command. The
* data is received into the RX Data FIFO using the RX_COUNT command. This
* function sets up the interrupt to trigger on the RX Data FIFO level, and the 
* data is fetched from the RX Data FIFO to the rxBuffer as it gets filled. This 
* function does not block until completion. The completion will trigger the call
* back function.
*
* \note This function is to be preceded by \ref Cy_SMIF_TransmitCommand. The
* slave select is de-asserted at the end of the receive.
* The function triggers the transfer and the transfer itself utilizes the
* interrupt for FIFO operations in the background. Thus, frequent
* interrupts will be executed after this function is triggered.
* This API is non-blocking and sets up the interrupt to act on the data
* FIFO, ensure there will be no another instance of the function called
* before the current instance has completed execution.
*
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param rxBuffer
* The pointer to the variable where the receive data is stored. If this pointer
* is a NULL, then the function does not enable the interrupt. This use case is 
* typically used when the FIFO is handled outside the interrupt and is managed 
* in either a polling-based code or a DMA. The user would handle the FIFO 
* management in a DMA or a polling-based code.
*
* \note If the user provides a NULL pointer in this function and does not handle
* the FIFO transaction, this could either stall or timeout the operation.
* The transfer statuses returned by \ref Cy_SMIF_GetTxfrStatus are no longer 
* valid.
*
* \param size
* The size of data to be received. Must be > 0.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param RxCmpltCb
* The callback executed at the end of a reception. NULL interpreted as no
* callback.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of a reception.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_ReceiveData(volatile stc_SMIF_t *base,
                            uint8_t *rxBuffer,
                            uint32_t size,
                            cy_en_smif_txfr_width_t transferWidth,
                            cy_en_smif_data_rate_t  dataRate,
                            cy_en_smif_slave_select_t  slaveSelect,
                            cy_smif_event_cb_t RxCmpltCb,
                            cy_stc_smif_context_t *context)
{
    uint32_t rxUnitNum;

    /* Check input values */
    if(size == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(rxBuffer == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    /* If the mode is octal SPI with DDR or Hyperbus, data unit is a 2-byte */
    if((transferWidth == CY_SMIF_WIDTH_OCTAL) && (dataRate == CY_SMIF_DDR))
    {
        if(size % 2 != 0)
        {
            return CY_SMIF_BAD_PARAM;
        }
        rxUnitNum = size / 2;
    }
    else
    {
        rxUnitNum = size;
    }

    /* Prepare a cmd fifo data */
    cy_un_smif_cmd_fifo_t cmdData  =
    {
        .fieldRxCount.cmd       = CY_SMIF_CMD_FIFO_RX_COUNT_MODE,
        .fieldRxCount.width     = (uint32_t)transferWidth,
        .fieldRxCount.dataRate  = (uint32_t)dataRate,
        .fieldRxCount.dataNum   = (uint32_t)(rxUnitNum - 1U),
        .fieldRxCount.last      = (uint32_t)LAST_COMMAND_BYTE,
        .fieldRxCount.selectDev = slaveSelect,
    };

    /* Move the parameters to the global variables */
    context->rxBufferAddress = (uint8_t*)rxBuffer;
    context->rxBufferCounter = size;
    context->rxCmpltCb       = RxCmpltCb;
    context->transferStatus  = CY_SMIF_REC_BUSY;

    /* Enter the receiving mode */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    /* Enable the TR_RX_REQ interrupt */
    base->unINTR_MASK.stcField.u1TR_RX_REQ = 1u;

    return (CY_SMIF_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SMIF_ReceiveDataBlocking
****************************************************************************//**
*
* This function implements the receive data phase in the memory command. The
* data is received into the RX Data FIFO using the RX_COUNT command. This
* function blocks until completion. The function does not use the interrupts and
* will use CPU to monitor the FIFO status and move data accordingly. The
* function returns only on completion.
*
* \note This function is to be preceded by \ref Cy_SMIF_TransmitCommand. The
* slave select is de-asserted at the end of the receive. Ensure there is
* no another transfers.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param rxBuffer
* The pointer to the variable where the receive data is stored. If this pointer
* is a NULL, then the function does not enable the interrupt. This use case is
* typically used when the FIFO is handled outside the interrupt and is managed
* in either a polling-based code or a DMA. The user would handle the FIFO
* management in a DMA or a polling-based code.
*
* \note If the user provides a NULL pointer in this function and does not handle
* the FIFO transaction, this could either stall or timeout the operation.
* The transfer statuses returned by \ref Cy_SMIF_GetTxfrStatus are no longer
* valid.
*
* \param size
* The size of data to be received. Must be > 0.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of a reception.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_ReceiveDataBlocking(volatile stc_SMIF_t *base,
                            uint8_t *rxBuffer,
                            uint32_t size,
                            cy_en_smif_txfr_width_t transferWidth,
                            cy_en_smif_data_rate_t  dataRate,
                            cy_en_smif_slave_select_t  slaveSelect,
                            cy_stc_smif_context_t *context)
{
    uint32_t rxUnitNum;

    /* Check input values */
    if(size == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(rxBuffer == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    /* If the mode is octal SPI with DDR or Hyperbus, data unit is a 2-byte */
    if((transferWidth == CY_SMIF_WIDTH_OCTAL) && (dataRate == CY_SMIF_DDR))
    {
        if(size % 2 != 0)
        {
            return CY_SMIF_BAD_PARAM;
        }
        rxUnitNum = size / 2;
    }
    else
    {
        rxUnitNum = size;
    }

    cy_un_smif_cmd_fifo_t cmdData = 
    {
        .fieldRxCount.cmd       = CY_SMIF_CMD_FIFO_RX_COUNT_MODE,
        .fieldRxCount.width     = (uint32_t)transferWidth,
        .fieldRxCount.dataRate  = (uint32_t)dataRate,
        .fieldRxCount.dataNum   = (uint32_t)(rxUnitNum - 1U),
        .fieldRxCount.last      = (uint32_t)LAST_COMMAND_BYTE,
        .fieldRxCount.selectDev = slaveSelect,
    };

    /* Enter the receiving mode */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    /* initialize parameters for Cy_SMIF_PushTxFifo */
    cy_stc_smif_context_t contextLoc = 
    {
        .rxBufferAddress = (uint8_t*)rxBuffer,
        .rxBufferCounter = size,
        .rxCmpltCb       = NULL,
        .transferStatus  = CY_SMIF_REC_BUSY,
    };

   /* The return variable */
    uint32_t timeoutUnits      = context->timeout;
    cy_en_smif_status_t result = CY_SMIF_SUCCESS;
     while ((CY_SMIF_REC_BUSY == contextLoc.transferStatus) &&
            (CY_SMIF_EXCEED_TIMEOUT != result))
    {
        Cy_SMIF_PopRxFifo(base, &contextLoc);
        result = Cy_SMIF_TimeoutRun(&timeoutUnits);
    }

  return (result);
}


/*******************************************************************************
* Function Name: Cy_SMIF_SendDummyCycles()
****************************************************************************//**
*
* This function sends dummy-clock cycles. The data lines are tri-stated during
* the dummy cycles.
*
* \note This function is to be preceded by \ref Cy_SMIF_TransmitCommand.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param doubleDummy
* Specifies whether an active RWDS refresh indicator
* (captured with the previous rising memory clock output) causes to double
* the number of dummy cycles. This is used for the variable latency in HyperRAM.
*
* \param rwdsInDmyEn
* Specifies whether the RWDS output signal should be driven starting in the
* last dummy cycle until DESELECT. This is needed for write transactions with
* RWDS (e.g. Hyperbus) before a TX_COUNT command.
*
* \param dlpEn
* Specifies whether data learning is enabled.
* In XIP mode this field is set for read transactions when
* CTL.INT_CLOCK_DL_ENABLED is "1".
*
* \param capEn
* specifies whether RWDS / DQS based capturing is enabled in the last dummy cycle,
* i.e. this needs to be set to '1' for read transactions to memory device using
* RWDS / DQS capturing scheme.
*
* \param cycles
* The number of dummy cycles. Must be > 0.
*
* \return A status of dummy cycles sending.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_SendDummyCycles(volatile stc_SMIF_t *base,
                                             cy_en_smif_txfr_width_t transferWidth,
                                             cy_en_smif_data_rate_t  dataRate,
                                             bool doubleDm,
                                             bool rwdsInDm,
                                             bool dlpEn,
                                             bool capEn,
                                             uint32_t cycles,
                                             cy_stc_smif_context_t *context)
{
    /* Check input values */
    if(cycles == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    cy_un_smif_cmd_fifo_t cmdData =
    {
        .fieldDummy.cmd      = CY_SMIF_CMD_FIFO_DUMMY_COUNT_MODE,
        .fieldDummy.cycleNum = ((uint32_t)(cycles-1U)),
        .fieldDummy.width    = (uint32_t)transferWidth,
        .fieldDummy.dataRate = (uint32_t)dataRate,
        .fieldDummy.last     = (uint32_t)NOT_LAST_COMMAND,
        .fieldDummy.doubleDm = doubleDm,
        .fieldDummy.rwdsInDm = rwdsInDm,
        .fieldDummy.dlpEn    = dlpEn,
        .fieldDummy.capEn    = capEn,
    };

    /* Send the dummy bytes */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    return (CY_SMIF_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SMIF_Deselect()
****************************************************************************//**
*
* This function  de-activates the select_out[3:0] IO output signals.
*
* \return
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_Deselect(volatile stc_SMIF_t *base, cy_stc_smif_context_t *context)
{
    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    cy_un_smif_cmd_fifo_t cmdData =
    {
        .fieldDeselect.cmd      = CY_SMIF_CMD_FIFO_DESELECT_MODE,
    };

    /* Send the dummy bytes */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    return (CY_SMIF_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SMIF_GetTxfrStatus
****************************************************************************//**
*
* This function provides the status of the transfer. This function is used to
* poll for the status of the TransmitData or receiveData function. When this
* function is called to determine the status of ongoing
* \ref Cy_SMIF_ReceiveData() or \ref Cy_SMIF_TransmitData(), the returned status
* is only valid if the functions passed a non-NULL buffer to transmit or
* receive respectively. If the pointer passed to \ref Cy_SMIF_ReceiveData()
* or \ref Cy_SMIF_TransmitData() is a NULL, then the code/DMA outside this
* driver will take care of the transfer and the Cy_GetTxfrStatus() will return 
* an erroneous result.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return Returns the transfer status. \ref cy_en_smif_txfr_status_t
*
*******************************************************************************/
cy_en_smif_txfr_status_t Cy_SMIF_GetTxfrStatus(volatile stc_SMIF_t *base, 
                                cy_stc_smif_context_t const *context)
{
    return (context->transferStatus);
}


/*******************************************************************************
* Function Name: Cy_SMIF_Enable
****************************************************************************//**
*
* Enables the operation of the SMIF block.
*
* \note This function only enables the SMIF IP. The interrupts associated with
* the SMIF will need to be separately enabled using the interrupt driver.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
*******************************************************************************/
void Cy_SMIF_Enable(volatile stc_SMIF_t *base, cy_stc_smif_context_t *context)
{
    base->unCTL.stcField.u1ENABLED = 1u;

    /* Read the register to flush the buffer */
    (void) base->unCTL.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SMIF_Encrypt()
****************************************************************************//**
*
* Uses the Encryption engine to create an encrypted result when the input, key
* and data arrays are provided. The AES-128 encryption of the address with the
* key, fetching the result and XOR with the data array are all done in the
* function. The operational scheme is the following:
*                   data = XOR(AES128(address, key), data)
* Decryption is done using the input data-array identically to the encryption.
* In the XIP mode, encryption and decryption are done without calling this
* function. The operational scheme in the XIP mode is the same. The address
* parameter in the XIP mode equals the actual address in the TVII memory map.
* The SMIF encryption engine is designed for code storage.
* For data storage, the encryption key can be changed.
* For sensitive data, the Crypto block is used.
*
* \note The API does not have access to the encryption key. The key must be
* placed in the register before calling this API. The crypto routine
* that can access the key storage area is recommended. This crypto routine is
* typically a protection context 0 function.
*
* \note This is a blocking API. The API waits for encryption completion. Will
* exit if a timeout is set (not equal to 0) and expired.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param address
* The address that gets encrypted is a masked 16-byte block address. The 32-bit
* address with the last 4 bits masked is placed as the last 4 bytes in the
* 128-bit input. The rest of the higher bit for the 128 bits are padded zeros.
* PA[127:0]:
* PA[3:0] = 0
* PA[7:4] = ADDR[7:4].
* PA[15:8] = ADDR[15:8].
* PA[23:16] = ADDR[23:16].
* PA[31:24] = ADDR[31:24].
* The other twelve of the sixteen plain text address bytes of PA[127:0] are "0":
* PA[127:32] = "0".
*
* \param dataInOut
* This is the location where the input data-array is passed while the function
* is called. This array gets populated with the result after encryption is
* completed.
*
* \param sizeInByte
* Provides a size of the array.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of the command transmit.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_Encrypt(volatile stc_SMIF_t *base,
                                        uint32_t address,
                                        uint8_t dataInOut[],
                                        uint32_t sizeInByte,
                                        cy_stc_smif_context_t const *context)
{
    if((sizeInByte == 0UL) ||
       (sizeInByte % CY_SMIF_AES128_BYTES) != 0UL) // The user needs to do padding in application.
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(dataInOut == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(base->unCRYPTO_CMD.stcField.u1START != CY_SMIF_CRYPTO_COMPLETED)
    {
        return CY_SMIF_BAD_STATUS;
    }

    /* Fill the output array */
    cy_en_smif_status_t status = CY_SMIF_SUCCESS;
    uint32_t blockNum          = (sizeInByte / CY_SMIF_AES128_BYTES);
    for(uint32_t i_AES_Block = 0U; i_AES_Block < blockNum; i_AES_Block++)
    {
        uint32_t timeoutUnits      = context->timeout;
        uint32_t addrOffset = i_AES_Block * CY_SMIF_AES128_BYTES;

        /* Fill the input field */
        uint32_t validPosOfINPUT0 = base->unCRYPTO_INPUT0.u32Register & (~CY_SMIF_CRYPTO_ADDR_MASK);
        base->unCRYPTO_INPUT0.u32Register = ((address + addrOffset) & CY_SMIF_CRYPTO_ADDR_MASK) + validPosOfINPUT0;

        /* Start the encryption */
        base->unCRYPTO_CMD.stcField.u1START = CY_SMIF_CRYPTO_START;

        while((base->unCRYPTO_CMD.stcField.u1START != CY_SMIF_CRYPTO_COMPLETED) && (CY_SMIF_EXCEED_TIMEOUT != status))
        {
            /* Wait until the encryption is completed and check the 
            * timeout 
            */
            status = Cy_SMIF_TimeoutRun(&timeoutUnits);
        }

        if (status == CY_SMIF_EXCEED_TIMEOUT)
        {
            break;
        }

        *(uint32_t*)&dataInOut[addrOffset]        ^= base->unCRYPTO_OUTPUT0.u32Register;
        *(uint32_t*)&dataInOut[addrOffset + 4ul]  ^= base->unCRYPTO_OUTPUT1.u32Register;
        *(uint32_t*)&dataInOut[addrOffset + 8ul]  ^= base->unCRYPTO_OUTPUT2.u32Register;
        *(uint32_t*)&dataInOut[addrOffset + 12ul] ^= base->unCRYPTO_OUTPUT3.u32Register;
    }
    return (status);
}

/*******************************************************************************
* Function Name: Cy_SMIF_CacheEnable
****************************************************************************//**
*
* This function is used to enable the fast cache, the slow cache or both.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cacheType
* Holds the type of the cache to be modified. \ref cy_en_smif_cache_en_t
*
* \return A status of function completion.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CacheEnable(volatile stc_SMIF_t *base, 
                                        cy_en_smif_cache_en_t cacheType)
{
    switch (cacheType)
    {
    case CY_SMIF_CACHE_SLOW:
        base->unSLOW_CA_CTL.stcField.u1ENABLED = 1;
        break;
    case CY_SMIF_CACHE_FAST:
        base->unFAST_CA_CTL.stcField.u1ENABLED = 1;
        break;
    case CY_SMIF_CACHE_BOTH:
        base->unSLOW_CA_CTL.stcField.u1ENABLED = 1;
        base->unFAST_CA_CTL.stcField.u1ENABLED = 1;
        break;
    default:
        /* A user error*/
        return CY_SMIF_BAD_PARAM;
    }   
    return CY_SMIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_SMIF_CacheDisable
****************************************************************************//**
*
* This function is used to disable the fast cache, the slow cache or both
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cacheType
* Holds the type of the cache to be modified. \ref cy_en_smif_cache_en_t
*
* \return A status of function completion.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CacheDisable(volatile stc_SMIF_t *base, 
                                            cy_en_smif_cache_en_t cacheType)
{
    switch (cacheType)
    {
    case CY_SMIF_CACHE_SLOW:
        base->unSLOW_CA_CTL.stcField.u1ENABLED = 0;
        break;
    case CY_SMIF_CACHE_FAST:
        base->unFAST_CA_CTL.stcField.u1ENABLED = 0;
        break;
    case CY_SMIF_CACHE_BOTH:
        base->unSLOW_CA_CTL.stcField.u1ENABLED = 0;
        base->unFAST_CA_CTL.stcField.u1ENABLED = 0;
        break;
    default:
        /* A user error*/
        return CY_SMIF_BAD_PARAM;
    }   
    return CY_SMIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_SMIF_CachePrefetchingEnable
****************************************************************************//**
*
* This function is used to enable pre-fetching for the fast cache, the slow
* cache or both.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cacheType
* Holds the type of the cache to be modified. \ref cy_en_smif_cache_en_t
*
* \return A status of function completion.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CachePrefetchingEnable(volatile stc_SMIF_t *base,
                                                    cy_en_smif_cache_en_t cacheType)
{
    switch (cacheType)
    {
    case CY_SMIF_CACHE_SLOW:
        base->unSLOW_CA_CTL.stcField.u1PREF_EN = 1;
        break;
    case CY_SMIF_CACHE_FAST:
        base->unFAST_CA_CTL.stcField.u1PREF_EN = 1;
        break;
    case CY_SMIF_CACHE_BOTH:
        base->unSLOW_CA_CTL.stcField.u1PREF_EN = 1;
        base->unFAST_CA_CTL.stcField.u1PREF_EN = 1;
        break;
    default:
        /* A user error*/
        return CY_SMIF_BAD_PARAM;
    }
    return (CY_SMIF_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SMIF_CachePrefetchingDisable
****************************************************************************//**
*
* This function is used to disable pre-fetching for the fast cache, the slow
* cache or both
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cacheType
* Holds the type of the cache to be modified. \ref cy_en_smif_cache_en_t
*
* \return A status of function completion.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CachePrefetchingDisable(volatile stc_SMIF_t *base,  
                                                    cy_en_smif_cache_en_t cacheType)
{
    switch (cacheType)
    {
    case CY_SMIF_CACHE_SLOW:
        base->unSLOW_CA_CTL.stcField.u1PREF_EN = 0;
        break;
    case CY_SMIF_CACHE_FAST:
        base->unFAST_CA_CTL.stcField.u1PREF_EN = 0;
        break;
    case CY_SMIF_CACHE_BOTH:
        base->unSLOW_CA_CTL.stcField.u1PREF_EN = 0;
        base->unFAST_CA_CTL.stcField.u1PREF_EN = 0;
        break;
    default:
        /* A user error*/
        return CY_SMIF_BAD_PARAM;
    }
    return (CY_SMIF_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SMIF_CacheInvalidate
****************************************************************************//**
*
* This function is used to invalidate/clear the fast cache, the slow cache or
* both
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cacheType
* Holds the type of the cache to be modified. \ref cy_en_smif_cache_en_t
*
* \return A status of function completion.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CacheInvalidate(volatile stc_SMIF_t *base, 
                                            cy_en_smif_cache_en_t cacheType)
{
    switch (cacheType)
    {
    case CY_SMIF_CACHE_SLOW:
        base->unSLOW_CA_CMD.stcField.u1INV = 1;
        (void)(base->unSLOW_CA_CMD.u32Register);
        break;
    case CY_SMIF_CACHE_FAST:
        base->unFAST_CA_CMD.stcField.u1INV = 1;
        (void)(base->unFAST_CA_CMD.u32Register);
        break;
    case CY_SMIF_CACHE_BOTH:
        base->unSLOW_CA_CMD.stcField.u1INV = 1;
        base->unFAST_CA_CMD.stcField.u1INV = 1;
        (void)(base->unFAST_CA_CMD.u32Register);
        break;
    default:
        /* A user error*/
        return CY_SMIF_BAD_PARAM;
    }
    return (CY_SMIF_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetMode
****************************************************************************//**
*
* Sets the mode of operation for the SMIF. The mode of operation can be the XIP
* mode where the slave devices are mapped as memories and are directly accessed
* from the TVII register map. In the MMIO mode, the SMIF block acts as a simple
* SPI engine.
*
* \note Interrupt and triggers and not working in XIP mode, see TRM for details
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param mode
* The mode of the SMIF operation.
*
* \return If busy it will return CY_SMIF_BAD_STATUS. (see \ref cy_en_smif_status_t).
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_SetMode(volatile stc_SMIF_t *base, cy_en_smif_mode_t mode)
{
    /*  Wait if busy */
    if(Cy_SMIF_IsBusy(base))
    {
        return CY_SMIF_BAD_STATUS;
    }

    Cy_SMIF_CacheInvalidate(base, CY_SMIF_CACHE_BOTH);

    /*  Set the register SMIF.CTL.XIP_MODE */
    base->unCTL.stcField.u1XIP_MODE = (uint8_t)mode;

    /* Read the register to flush the buffer */
    (void) base->unCTL.u32Register;

    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetMode
****************************************************************************//**
*
* This function calculates CRC for input data. Please call this function in XIP
* mode.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param inData
* The data that this function will calculate it's CRC.
*
* \param sizeInByte
* The size of input data.
*
* \param result
* A pointer to 8 bit CRC calculated.
*
* \note Not tested yet.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CRC_Calibration(volatile stc_SMIF_t *base, uint8_t inData[], uint32_t sizeInByte, uint8_t* result)
{
    if(base->unCTL.stcField.u1XIP_MODE == CY_SMIF_MEMORY)
    {
        return CY_SMIF_BAD_STATUS;
    }

    if(inData == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(sizeInByte == 0ul)
    {
        return CY_SMIF_BAD_PARAM;
    }

    for(uint32_t i_byte = 0ul; i_byte < sizeInByte; i_byte += 8ul)
    {
        base->unCRC_INPUT0.u32Register = 0ul;
        base->unCRC_INPUT1.u32Register = 0ul;

        uint32_t restByte = (8ul < (sizeInByte - i_byte)) ? 8ul : (sizeInByte - i_byte); // min(8ul, (sizeInByte - i_byte));
        for(uint32_t i_subByte = 0ul; i_subByte < restByte; i_subByte++)
        {
            if(i_subByte < 4ul)
            {
                base->unCRC_INPUT0.au8Byte[i_subByte] = inData[i_byte + i_subByte];
            }
            else
            {
                base->unCRC_INPUT1.au8Byte[i_subByte-4ul] = inData[i_byte + i_subByte];
            }
        }

        if(i_byte == 0ul)
        {
            base->unCRC_CMD.stcField.u1START = 1u;
            while(base->unCRC_CMD.stcField.u1START == 1u); // Note: no timeout for now
        }
        else
        {
            base->unCRC_CMD.stcField.u1CONTINUE = 1u;
            while(base->unCRC_CMD.stcField.u1CONTINUE == 1u); // Note: no timeout for now
        }
    }

    *result = (uint8_t)(base->unCRC_OUTPUT.stcField.u8CRC_OUTPUT);
    return CY_SMIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_SMIF_SetDelayLineForFreq
****************************************************************************//**
*
* Selects the right delay line for the specified frequency, so that the taps of
* that delay line should give enough possibilities to fine tune the delay in a 
* subsequent calibration step.
*
* \note Expects that SMIF is not busy!
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param freqMhz
* The frequency of the SMIF clock (external)
*
* \param isDDR
* If the protocol has double data rate (e.g. in case of HyperBus)
*
* \param calculatedTap
* Caller can optionally retrieve the tap number index that was calculated by this 
* function (value: 0 .. (SMIFn_DELAY_TAPS_NR - 1)). Pass NULL if value is not needed
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_SetDelayLineForFreq(volatile stc_SMIF_t *base, uint8_t freqMhz, bool isDDR, uint8_t *calculatedTap)
{
    // TODO: Device dependent delays (and number of delay lines)
    static const float32_t minDelayPerTapInNs[SMIF0_DELAY_LINES_NR] = {0.0559, 0.1197, 0.1935, 0.3973};
    cy_en_smif_status_t status = CY_SMIF_SUCCESS;
    uint8_t tapNumCalculated;
    uint8_t delayLine;
    uint8_t maxTaps = CY_SMIF_GetDelayTapsNumber(base);

    // Calculate the offset to the nominal center of the data eye
    float32_t clockPeriodNs = 1000.0 / freqMhz;
    float32_t dataPeriodNs = clockPeriodNs / ((isDDR == true) ? 2.0 : 1.0);
    float32_t dataPeriodCenterNs = dataPeriodNs / 2.0;

    // Check all scan lines whether the number of taps required to hit the nominal center of the data eye are below the actual number of taps per delay line
    for(delayLine = 0; delayLine < SMIF0_DELAY_LINES_NR; delayLine++)
    {
        tapNumCalculated = (uint8_t) ((dataPeriodCenterNs / minDelayPerTapInNs[delayLine]) + 0.5);
        if(tapNumCalculated <= maxTaps)
        {
            break;
        }
    }

    // No matching delay line found, use the one with the largest delays and return error...
    if(delayLine == 4)
    {
        delayLine = 3;
        status = CY_SMIF_BAD_PARAM;
    }

    Cy_SMIF_Set_DelayLineSel(base, (cy_en_smif_delay_line_t)delayLine);

    if(calculatedTap != NULL)
    {
        // If no matching delay line has been found, return the maximum number of taps
        *calculatedTap = ((tapNumCalculated < maxTaps) ? tapNumCalculated : maxTaps) - 1;
    }

    return status;
}


/*******************************************************************************
* Function Cy_SMIF_GetSmifBasePtrFromAnySmifPtr
****************************************************************************//**
*
* This function returns the SMIF base pointer corresponding to any pointer within
* the SMIF instance address space. 
*
* \param any_smif_address
* Can be any address within a SMIF instance address space. Typically it either is
* already a SMIF base pointer or a SMIF_DEVICE base pointer.
*
* \return SMIF base pointer
*
*******************************************************************************/
volatile stc_SMIF_t * Cy_SMIF_GetSmifBasePtrFromAnySmifPtr(volatile void *any_smif_address)
{
    uint32_t mask_for_smif            = ~(SMIF_SECTION_SIZE - 1);
    uint32_t smif_base                = ((uint32_t) any_smif_address) & mask_for_smif;

    return (volatile stc_SMIF_t *) smif_base;
}

/*******************************************************************************
* Function Cy_SMIF_GetSlaveSelectFromDeviceBasePtr
****************************************************************************//**
*
* This function returns the cy_en_smif_slave_select_t corresponding to the
* base pointer to a SMIF_DEVICE instance which is passed as parameter
*
* \param device
* The base address of the SMIF_DEVICE registers.
*
* \return (see \cy_en_smif_slave_select_t)
*
*******************************************************************************/
cy_en_smif_slave_select_t Cy_SMIF_GetSlaveSelectFromDeviceBasePtr(volatile stc_SMIF_DEVICE_t *device)
{
    uint32_t device_offset_from_smif    = ((uint32_t)device & (SMIF_SECTION_SIZE - 1));
    uint32_t device_offset_from_device0 = device_offset_from_smif - (((uint32_t)SMIF0_DEVICE0 - (uint32_t) SMIF0));
    uint8_t  device_instance            = device_offset_from_device0 / SMIF_DEVICE_SECTION_SIZE;

    cy_en_smif_slave_select_t slave_select = Cy_SMIF_GetSlaveSelectFromDeviceNumber(device_instance);

    return slave_select;
}


/*******************************************************************************
* Function Cy_SMIF_GetSlaveSelectFromDeviceNumber
****************************************************************************//**
*
* This function returns the cy_en_smif_slave_select_t corresponding to the
* device number which is passed as parameter
*
* \param device_number
* Device index (0, 1, 2, ...)
*
* \return (see \cy_en_smif_slave_select_t)
*
*******************************************************************************/
cy_en_smif_slave_select_t Cy_SMIF_GetSlaveSelectFromDeviceNumber(uint8_t device_number)
{
    cy_en_smif_slave_select_t slave_select = (cy_en_smif_slave_select_t) 0xdeaddead;

    switch(device_number)
    {
        case 0:
            slave_select = CY_SMIF_SLAVE_SELECT_0;
            break;
      #if (SMIF0_DEVICE_NR >= 2) || (defined(SMIF1_DEVICE_NR) && (SMIF1_DEVICE_NR >= 2))
        case 1:
            slave_select = CY_SMIF_SLAVE_SELECT_1;
            break;
      #endif
      #if (SMIF0_DEVICE_NR >= 3) || (defined(SMIF1_DEVICE_NR) && (SMIF1_DEVICE_NR >= 3))
        case 2:
            slave_select = CY_SMIF_SLAVE_SELECT_2;
            break;
      #endif
      #if (SMIF0_DEVICE_NR >= 4) || (defined(SMIF1_DEVICE_NR) && (SMIF1_DEVICE_NR >= 4))
        case 3:
            slave_select = CY_SMIF_SLAVE_SELECT_3;
            break;
      #endif
        default:
            break;
    }

    return slave_select;
}


/*******************************************************************************
* Function Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndSlaveSelect
****************************************************************************//**
*
* This function returns the device base pointer corresponding to the
* base pointer to a SMIF instance and slave select which are passed as parameter
*
* \param base
* The base address of the SMIF registers.
*
* \param slave
* Specifies the number of the slave device.
* \ref cy_en_smif_slave_select_t
*
* \return SMIF_DEVICE base pointer
*
*******************************************************************************/
volatile stc_SMIF_DEVICE_t * Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndSlaveSelect(volatile stc_SMIF_t *base, cy_en_smif_slave_select_t slave)
{
    // Offset to first device / slave select #0
    uint32_t offset = (((uint32_t)SMIF0_DEVICE0 - (uint32_t) SMIF0));

    // Increment offset based on actual slave select
    switch(slave)
    {
        case CY_SMIF_SLAVE_SELECT_0:
            offset += (0 * SMIF_DEVICE_SECTION_SIZE);
            break;
      #if (SMIF0_DEVICE_NR >= 2) || (defined(SMIF1_DEVICE_NR) && (SMIF1_DEVICE_NR >= 2))
        case CY_SMIF_SLAVE_SELECT_1:
            offset += (1 * SMIF_DEVICE_SECTION_SIZE);
            break;
      #endif
      #if (SMIF0_DEVICE_NR >= 3) || (defined(SMIF1_DEVICE_NR) && (SMIF1_DEVICE_NR >= 3))
        case CY_SMIF_SLAVE_SELECT_2:
            offset += (2 * SMIF_DEVICE_SECTION_SIZE);
            break;
      #endif
      #if (SMIF0_DEVICE_NR >= 4) || (defined(SMIF1_DEVICE_NR) && (SMIF1_DEVICE_NR >= 4))
        case CY_SMIF_SLAVE_SELECT_3:
            offset += (3 * SMIF_DEVICE_SECTION_SIZE);
            break;
      #endif
        default:
            break;
    }

    return (volatile stc_SMIF_DEVICE_t *) ((uint32_t)base + offset);
}


/*******************************************************************************
* Function Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndDeviceNumber
****************************************************************************//**
*
* This function returns the device base pointer corresponding to the
* base pointer to a SMIF instance and device number which are passed as parameter
*
* \param base
* The base address of the SMIF registers.
*
* \param device_number
* Device index (0, 1, 2, ...)
*
* \return SMIF_DEVICE base pointer
*
*******************************************************************************/
volatile stc_SMIF_DEVICE_t * Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndDeviceNumber(volatile stc_SMIF_t *base, uint8_t device_number)
{
    // Offset to first device / slave select #0
    uint32_t offset = (((uint32_t)SMIF0_DEVICE0 - (uint32_t) SMIF0));

    offset += (device_number * SMIF_DEVICE_SECTION_SIZE);

    return (volatile stc_SMIF_DEVICE_t *) ((uint32_t)base + offset);
}



/*******************************************************************************
* Function Name: Cy_SMIF_DeepSleepCallback
****************************************************************************//**
*
* This function handles the transition of the SMIF into and out of Deep
* Sleep mode. It prevents the device from entering DeepSleep if SMIF is actively
* communicating, or there is any data in the TX or RX FIFOs, or SMIF is in
* memory mode.
*
* This function should be called while execution of \ref Cy_SysPm_DeepSleep
* therefore must be registered as a callback before the call. To register it
* call \ref Cy_SysPm_RegisterCallback and specify \ref CY_SYSPM_DEEPSLEEP
* as the callback type.
*
* \note
* This API is template and user should add code for external memory enter/exit
* low power mode.
*
* \param callbackParams
* The pointer to the structure with SMIF SysPm callback parameters (pointer to
* SMIF registers, context and call mode \ref cy_stc_syspm_callback_params_t).
*
* \return
* \ref cy_en_syspm_status_t
*
* Example setup of SysPM deep sleep and hibernate mode
* \snippet smif/smif_sut_01.cydsn/main_cm4.c SMIF SysPM Callback
*
*******************************************************************************/
cy_en_syspm_status_t Cy_SMIF_DeepSleepCallback(cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t retStatus = CY_SYSPM_SUCCESS;
    
    CY_ASSERT_L1(NULL != callbackParams);

    stc_SMIF_t *locBase = (stc_SMIF_t *) callbackParams->base;
    cy_stc_smif_context_t *locContext = (cy_stc_smif_context_t *) callbackParams->context;

    switch(callbackParams->mode)
    {
        case CY_SYSPM_CHECK_READY:
        {
            /* Check if API is not busy executing transfer operation */
            /* If SPI bus is not busy, all data elements are transferred on
                * the bus from the TX FIFO and shifter and the RX FIFIOs is
                * empty - the SPI is ready enter Deep Sleep.
            */
            bool checkFail = (CY_SMIF_REC_BUSY == (cy_en_smif_txfr_status_t)
                                    Cy_SMIF_GetTxfrStatus(locBase, locContext));
            checkFail = (Cy_SMIF_IsBusy(locBase)) || checkFail;
            checkFail = (Cy_SMIF_GetMode(locBase) == CY_SMIF_MEMORY) || checkFail;

            if (checkFail)
            {
                retStatus = CY_SYSPM_FAIL;
            }
            else
            {
                Cy_SMIF_Disable(locBase);
                retStatus = CY_SYSPM_SUCCESS;
            }
            /* Add check memory that memory not in progress */
        }
        break;

        case CY_SYSPM_CHECK_FAIL:
        {
            /* Other driver is not ready for Deep Sleep. Restore Active mode
            * configuration.
            */
            Cy_SMIF_Enable(locBase, locContext);

        }
        break;

        case CY_SYSPM_BEFORE_ENTER:
        {
            /* This code executes inside critical section and enabling active
            * interrupt source makes interrupt pending in the NVIC. However
            * interrupt processing is delayed until code exists critical
            * section. The pending interrupt force WFI instruction does
            * nothing and device remains in the active mode.
            */

            /* Put external memory in low power mode */
        }
        break;

        case CY_SYSPM_AFTER_EXIT:
        {
            /* Put external memory in active mode */
            Cy_SMIF_Enable(locBase, locContext);
        }
        break;

        default:
            retStatus = CY_SYSPM_FAIL;
            break;
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SMIF_HibernateCallback
****************************************************************************//**
*
* This function handles the transition of the SMIF into Hibernate mode.
* It prevents the device from entering Hibernate if the SMIF
* is actively communicating, or there is any data in the TX or RX FIFO, or SMIF
* is in memory mode.
*
* This function should be called during execution of \ref Cy_SysPm_Hibernate
* therefore it must be registered as a callback before the call. To register it
* call \ref Cy_SysPm_RegisterCallback and specify \ref CY_SYSPM_HIBERNATE
* as the callback type.
*
* \note
* This API is template and user should add code for external memory enter/exit
* low power mode.
*
* \param callbackParams
* The pointer to the structure with SMIF SysPm callback parameters (pointer to
* SMIF registers, context and call mode \ref cy_stc_syspm_callback_params_t).
*
* \return
* \ref cy_en_syspm_status_t
*
* Example setup of SysPM deep sleep and hibernate mode
* \snippet smif/smif_sut_01.cydsn/main_cm4.c SMIF SysPM Callback
*
*******************************************************************************/
cy_en_syspm_status_t Cy_SMIF_HibernateCallback(cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t retStatus = CY_SYSPM_SUCCESS;

    CY_ASSERT_L1(NULL != callbackParams);
    
    stc_SMIF_t *locBase = (stc_SMIF_t *) callbackParams->base;
    cy_stc_smif_context_t *locContext = (cy_stc_smif_context_t *) callbackParams->context;

    switch(callbackParams->mode)
    {
        case CY_SYSPM_CHECK_READY:
        {
            /* Check if API is not busy executing transfer operation 
            * If SPI bus is not busy, all data elements are transferred on
            * the bus from the TX FIFO and shifter and the RX FIFIOs is
            * empty - the SPI is ready enter Deep Sleep.
            */
            bool checkFail = (CY_SMIF_REC_BUSY == (cy_en_smif_txfr_status_t)
                                Cy_SMIF_GetTxfrStatus(locBase, locContext));
            checkFail = (Cy_SMIF_IsBusy(locBase)) || checkFail;
            checkFail = (Cy_SMIF_GetMode(locBase) == CY_SMIF_MEMORY) || checkFail;

            if (checkFail)
            {
                retStatus = CY_SYSPM_FAIL;

            }
            else
            {
                retStatus = CY_SYSPM_SUCCESS;
                Cy_SMIF_Disable(locBase);
            }
            /* Add check memory that memory not in progress */
        }
        break;

        case CY_SYSPM_CHECK_FAIL:
        {
            /* Other driver is not ready for Deep Sleep. Restore Active mode
            * configuration.
            */
            Cy_SMIF_Enable(locBase, locContext);

        }
        break;

        case CY_SYSPM_BEFORE_ENTER:
        {
            /* Put external memory in low power mode */
        }
        break;

        case CY_SYSPM_AFTER_EXIT:
        {
            Cy_SMIF_Enable(locBase, locContext);
            /* Put external memory in active mode */

        }
        break;

        default:
            retStatus = CY_SYSPM_FAIL;
        break;
    }

    return (retStatus);
}

#if defined(__cplusplus)
}
#endif

#endif /* CY_IP_MXSMIF */

/* [] END OF FILE */
