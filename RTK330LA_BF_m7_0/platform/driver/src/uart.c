#include "uart.h"
#include "cy_project.h"
#include "cy_device_headers.h"
#include <stdio.h>
#include <stdarg.h>

/********************************/
/**** UART and DW Variables *****/
/********************************/
#define UART1_RX_UNIT_BYTE            (1ul)
#define UART1_RX_FIFO_LEVEL           (0ul)
#define UART1_RX_BUFFER_SIZE          (256ul)
#define UART1_RX_BUFFER_BLOCK_NUM     (UART1_RX_BUFFER_SIZE / (UART1_RX_FIFO_LEVEL + 1ul))
#define UART1_TX_FIFO_LEVEL           (1ul)
#define UART1_TX_BUFFER_SIZE          (1024ul * 4)

#define UART1_OVERSAMPLING          (8)
static cy_stc_scb_uart_context_t   g_stc_uart_context_uart1;
static cy_stc_scb_uart_config_t    g_stc_uart_config_uart1 =
{
    .uartMode                   = CY_SCB_UART_STANDARD,
    .oversample                 = UART1_OVERSAMPLING,
    .dataWidth                  = 8ul,
    .enableMsbFirst             = false,
    .stopBits                   = CY_SCB_UART_STOP_BITS_1,
    .parity                     = CY_SCB_UART_PARITY_NONE,
    .enableInputFilter          = false,
    .dropOnParityError          = false,
    .dropOnFrameError           = false,
    .enableMutliProcessorMode   = false,
    .receiverAddress            = 0ul,
    .receiverAddressMask        = 0ul,
    .acceptAddrInFifo           = false,
    .irdaInvertRx               = false,
    .irdaEnableLowPowerReceiver = false,
    .smartCardRetryOnNack       = false,
    .enableCts                  = false,
    .ctsPolarity                = CY_SCB_UART_ACTIVE_LOW,
    .rtsRxFifoLevel             = 0ul,
    .rtsPolarity                = CY_SCB_UART_ACTIVE_LOW,
    .breakWidth                 = 0ul,
    .rxFifoTriggerLevel         = UART1_RX_FIFO_LEVEL,
    .rxFifoIntEnableMask        = 0ul,
    .txFifoTriggerLevel         = UART1_TX_FIFO_LEVEL,
    .txFifoIntEnableMask        = 0ul
};

#define DW1_CH_TO_SCB_UART1_TX    (18)
#define DW1_CH_TO_SCB_UART1_RX    (19)
#define UART1_TX_TO_DMA_TRIG      (TRIG_IN_1TO1_2_SCB_TX_TO_PDMA11)
#define UART1_RX_TO_DMA_TRIG      (TRIG_IN_1TO1_2_SCB_RX_TO_PDMA11)

/****************/
/* DW Utilities */
/****************/
static  cy_stc_pdma_descr_t         dw_uart1_tx_descr;
static const   cy_stc_pdma_chnl_config_t   dw_uart1_tx_config =
{
    .PDMA_Descriptor = &dw_uart1_tx_descr,
    .preemptable     = 0ul,
    .priority        = 0ul,
    .enable          = 0ul,        /* enabled after initialization */
};
#define USER_DW_TX_SIZE (128)
static uint8_t user_tx_data[USER_DW_TX_SIZE];
static uint8_t is_uart1_tx_Complete = 1;
static  cy_stc_pdma_descr_config_t  dw1ch_uart1_tx_config =
{
    .deact          =   0ul,
    .intrType       =   CY_PDMA_INTR_DESCR_CMPLT,
    .trigoutType    =   CY_PDMA_TRIGOUT_DESCR_CMPLT,
    .chStateAtCmplt =   CY_PDMA_CH_DISABLED,
    .triginType     =   CY_PDMA_TRIGIN_DESCR,
    .dataSize       =   CY_PDMA_BYTE,
    .srcTxfrSize    =   0ul, // same as "dataSize"
    .destTxfrSize   =   1ul, // 32bit width
    .descrType      =   CY_PDMA_2D_TRANSFER,
    .srcAddr        =   user_tx_data,
    .destAddr       =   (void *)&CY_UART1_TYPE->unTX_FIFO_WR.u32Register,
    .srcXincr       =   1ul,
    .destXincr      =   0ul,
    .xCount         =   UART1_TX_FIFO_LEVEL,
    .srcYincr       =   UART1_TX_FIFO_LEVEL,
    .destYincr      =   0ul,
    .yCount         =   0ul, // will be updated in run time
    .descrNext      =   &dw_uart1_tx_descr
};

#define UART6_RX_UNIT_BYTE            (1ul)
#define UART6_RX_FIFO_LEVEL           (0ul)
#define UART6_RX_BUFFER_SIZE          (256ul)
#define UART6_RX_BUFFER_BLOCK_NUM     (UART6_RX_BUFFER_SIZE / (UART6_RX_FIFO_LEVEL + 1ul))
#define UART6_TX_FIFO_LEVEL           (16ul)
#define UART6_TX_BUFFER_SIZE          (1024ul * 4)

#define UART6_OVERSAMPLING          (8)
static cy_stc_scb_uart_context_t   g_stc_uart_context_uart6;
static cy_stc_scb_uart_config_t    g_stc_uart_config_uart6 =
{
    .uartMode                   = CY_SCB_UART_STANDARD,
    .oversample                 = UART6_OVERSAMPLING,
    .dataWidth                  = 8ul,
    .enableMsbFirst             = false,
    .stopBits                   = CY_SCB_UART_STOP_BITS_1,
    .parity                     = CY_SCB_UART_PARITY_NONE,
    .enableInputFilter          = false,
    .dropOnParityError          = false,
    .dropOnFrameError           = false,
    .enableMutliProcessorMode   = false,
    .receiverAddress            = 0ul,
    .receiverAddressMask        = 0ul,
    .acceptAddrInFifo           = false,
    .irdaInvertRx               = false,
    .irdaEnableLowPowerReceiver = false,
    .smartCardRetryOnNack       = false,
    .enableCts                  = false,
    .ctsPolarity                = CY_SCB_UART_ACTIVE_LOW,
    .rtsRxFifoLevel             = 0ul,
    .rtsPolarity                = CY_SCB_UART_ACTIVE_LOW,
    .breakWidth                 = 0ul,
    .rxFifoTriggerLevel         = 0ul,
    .rxFifoIntEnableMask        = 0ul,
    .txFifoTriggerLevel         = 0ul,
    .txFifoIntEnableMask        = 0ul
};

#define UART7_RX_UNIT_BYTE            (1ul)
#define UART7_RX_FIFO_LEVEL           (0ul)
#define UART7_RX_BUFFER_SIZE          (256ul)
#define UART7_RX_BUFFER_BLOCK_NUM     (UART7_RX_BUFFER_SIZE / (UART7_RX_FIFO_LEVEL + 1ul))
#define UART7_TX_FIFO_LEVEL           (1ul)
#define UART7_TX_BUFFER_SIZE          (1024ul * 4)

#define UART7_OVERSAMPLING          (8)
static cy_stc_scb_uart_context_t   g_stc_uart_context_uart7;
static cy_stc_scb_uart_config_t    g_stc_uart_config_uart7 =
{
    .uartMode                   = CY_SCB_UART_STANDARD,
    .oversample                 = UART7_OVERSAMPLING,
    .dataWidth                  = 8ul,
    .enableMsbFirst             = false,
    .stopBits                   = CY_SCB_UART_STOP_BITS_1,
    .parity                     = CY_SCB_UART_PARITY_NONE,
    .enableInputFilter          = false,
    .dropOnParityError          = false,
    .dropOnFrameError           = false,
    .enableMutliProcessorMode   = false,
    .receiverAddress            = 0ul,
    .receiverAddressMask        = 0ul,
    .acceptAddrInFifo           = false,
    .irdaInvertRx               = false,
    .irdaEnableLowPowerReceiver = false,
    .smartCardRetryOnNack       = false,
    .enableCts                  = false,
    .ctsPolarity                = CY_SCB_UART_ACTIVE_LOW,
    .rtsRxFifoLevel             = 0ul,
    .rtsPolarity                = CY_SCB_UART_ACTIVE_LOW,
    .breakWidth                 = 0ul,
    .rxFifoTriggerLevel         = UART7_RX_FIFO_LEVEL,
    .rxFifoIntEnableMask        = 0ul,
    .txFifoTriggerLevel         = UART7_TX_FIFO_LEVEL,
    .txFifoIntEnableMask        = 0ul
};

#define DW1_CH_TO_SCB_UART7_TX    (30)
#define DW1_CH_TO_SCB_UART7_RX    (31)
#define UART7_TX_TO_DMA_TRIG      (TRIG_IN_1TO1_2_SCB_TX_TO_PDMA17)
#define UART7_RX_TO_DMA_TRIG      (TRIG_IN_1TO1_2_SCB_RX_TO_PDMA17)

/****************/
/* DW Utilities */
/****************/
static  cy_stc_pdma_descr_t         dw_uart7_tx_descr;
static const   cy_stc_pdma_chnl_config_t   dw_uart7_tx_config =
{
    .PDMA_Descriptor = &dw_uart7_tx_descr,
    .preemptable     = 0ul,
    .priority        = 0ul,
    .enable          = 0ul,        /* enabled after initialization */
};
#define DEBUG_DW_TX_SIZE (128)
static uint8_t debug_tx_data[DEBUG_DW_TX_SIZE];
static uint8_t is_uart7_tx_Complete = 1;
static  cy_stc_pdma_descr_config_t  dw1ch_uart7_tx_config =
{
    .deact          =   0ul,
    .intrType       =   CY_PDMA_INTR_DESCR_CMPLT,
    .trigoutType    =   CY_PDMA_TRIGOUT_DESCR_CMPLT,
    .chStateAtCmplt =   CY_PDMA_CH_DISABLED,
    .triginType     =   CY_PDMA_TRIGIN_DESCR,
    .dataSize       =   CY_PDMA_BYTE,
    .srcTxfrSize    =   0ul, // same as "dataSize"
    .destTxfrSize   =   1ul, // 32bit width
    .descrType      =   CY_PDMA_2D_TRANSFER,
    .srcAddr        =   debug_tx_data,
    .destAddr       =   (void *)&CY_UART7_TYPE->unTX_FIFO_WR.u32Register,
    .srcXincr       =   1ul,
    .destXincr      =   0ul,
    .xCount         =   UART7_TX_FIFO_LEVEL,
    .srcYincr       =   UART7_TX_FIFO_LEVEL,
    .destYincr      =   0ul,
    .yCount         =   0ul, // will be updated in run time
    .descrNext      =   &dw_uart7_tx_descr
};


static uint8_t g_uart1_in_data[128];
static uint8_t g_uart6_in_data[128];
static uint8_t g_uart7_in_data[128];

#define UART1_RX_FIFO_SIZE  (1024 * 4)
#define UART1_TX_FIFO_SIZE  (1024 * 4)

#define UART6_RX_FIFO_SIZE  (1024 * 4)
#define UART6_TX_FIFO_SIZE  (1024 * 4)

#define UART7_RX_FIFO_SIZE  (1024 * 4)
#define UART7_TX_FIFO_SIZE  (1024 * 4)

fifo_type uart1_rx_fifo;
fifo_type uart1_tx_fifo;
fifo_type uart6_rx_fifo;
fifo_type uart6_tx_fifo;
fifo_type uart7_rx_fifo;
fifo_type uart7_tx_fifo;

uint8_t uart1_rx_buff[UART1_RX_FIFO_SIZE];
uint8_t uart1_tx_buff[UART1_TX_FIFO_SIZE];
uint8_t uart6_rx_buff[UART6_RX_FIFO_SIZE];
uint8_t uart6_tx_buff[UART6_TX_FIFO_SIZE];
uint8_t uart7_rx_buff[UART7_RX_FIFO_SIZE];
uint8_t uart7_tx_buff[UART7_TX_FIFO_SIZE];


void Scb_UART1_IntrISR(void);
void Scb_UART1_Event(uint32_t locEvents);
void DW1_uart1_tx_IntHandler(void);

void bsp_uart1_init(uint32_t boadrate, uint32_t sourceFreq)
{
    fifo_init(&uart1_rx_fifo, uart1_rx_buff, UART1_RX_FIFO_SIZE);
    fifo_init(&uart1_tx_fifo, uart1_tx_buff, UART1_TX_FIFO_SIZE);

    /* Port Configuration for UART */
    cy_stc_gpio_pin_config_t    stc_port_pin_cfg_uart = {0ul};
    stc_port_pin_cfg_uart.driveMode = CY_GPIO_DM_HIGHZ;
    stc_port_pin_cfg_uart.hsiom     = CY_UART1_RX_MUX;
    Cy_GPIO_Pin_Init(CY_UART1_RX_PORT, CY_UART1_RX_PIN, &stc_port_pin_cfg_uart);

    stc_port_pin_cfg_uart.driveMode = CY_GPIO_DM_STRONG_IN_OFF;
    stc_port_pin_cfg_uart.hsiom     = CY_UART1_TX_MUX;
    Cy_GPIO_Pin_Init(CY_UART1_TX_PORT, CY_UART1_TX_PIN, &stc_port_pin_cfg_uart);

    /* SCB-UART Initialization */
    Cy_SCB_UART_DeInit(CY_UART1_TYPE);
    Cy_SCB_UART_Init(CY_UART1_TYPE, &g_stc_uart_config_uart1, &g_stc_uart_context_uart1);
    Cy_SCB_UART_RegisterCallback(CY_UART1_TYPE, Scb_UART1_Event, &g_stc_uart_context_uart1);
    Cy_SCB_UART_Enable(CY_UART1_TYPE);

    /* Clock Configuration for UART */
    // Assign a programmable divider
    Cy_SysClk_PeriphAssignDivider(CY_UART1_PCLK, CY_SYSCLK_DIV_24_5_BIT, 1);
    // Set divider value
    {
        uint64_t targetFreq     = UART1_OVERSAMPLING * boadrate;
        uint64_t sourceFreq_fp5 = ((uint64_t)sourceFreq << 5ull);
        uint32_t divSetting_fp5 = (uint32_t)(sourceFreq_fp5 / targetFreq);
        Cy_SysClk_PeriphSetFracDivider(Cy_SysClk_GetClockGroup(CY_UART1_PCLK),
                                       CY_SYSCLK_DIV_24_5_BIT,
                                       1,
                                       ((divSetting_fp5 & 0x1FFFFFE0ul) >> 5ul),
                                       (divSetting_fp5 & 0x0000001Ful));
    }
    // Enable peripheral divider
    Cy_SysClk_PeriphEnableDivider(Cy_SysClk_GetClockGroup(CY_UART1_PCLK), CY_SYSCLK_DIV_24_5_BIT, 1);

    /* Interrupt Configuration for UART */
    cy_stc_sysint_irq_t stc_sysint_irq_cfg_uart;
    stc_sysint_irq_cfg_uart.sysIntSrc = UART1_IRQN;
    stc_sysint_irq_cfg_uart.intIdx    = CPUIntIdx6_IRQn;
    stc_sysint_irq_cfg_uart.isEnabled = true;
    Cy_SysInt_InitIRQ(&stc_sysint_irq_cfg_uart);
    Cy_SysInt_SetSystemIrqVector(stc_sysint_irq_cfg_uart.sysIntSrc, Scb_UART1_IntrISR);
    NVIC_EnableIRQ(stc_sysint_irq_cfg_uart.intIdx);

    Cy_SCB_UART_Receive(CY_UART1_TYPE, &g_uart1_in_data[0], UART1_RX_UNIT_BYTE, &g_stc_uart_context_uart1);

    /**************************************************/
    /* DW dedicating to UART TX and RX Initialization */
    /**************************************************/
    Cy_PDMA_Disable(DW1);

    /* DW channel dedicating to UART TX Initialization */
    Cy_PDMA_Chnl_DeInit(DW1, DW1_CH_TO_SCB_UART1_TX);
    Cy_PDMA_Descr_Init(&dw_uart1_tx_descr, &dw1ch_uart1_tx_config);
    Cy_PDMA_Chnl_Init(DW1, DW1_CH_TO_SCB_UART1_TX, &dw_uart1_tx_config);
    Cy_PDMA_Chnl_SetInterruptMask(DW1, DW1_CH_TO_SCB_UART1_TX);

    // /* DW channel dedicating to UART RX Initialization */
    // Cy_PDMA_Chnl_DeInit(DW1, DW1_CH_TO_SCB_UART1_RX);
    // Cy_PDMA_Descr_Init(&dwUartRxDescr, &dw1ChUartRxConfig);
    // Cy_PDMA_Chnl_Init(DW1, DW1_CH_TO_SCB_UART1_RX, &dwUartRxConfig);
    // Cy_PDMA_Chnl_SetInterruptMask(DW1, DW1_CH_TO_SCB_UART1_RX);
    
    Cy_PDMA_Enable(DW1);

    /**************************************/
    /* Trigger Multiplexer Initialization */
    /**************************************/
    /* UART TX To DW */
    Cy_TrigMux_Connect1To1(UART1_TX_TO_DMA_TRIG, 0ul, TRIGGER_TYPE_LEVEL, 0ul);
    /* UART RX To DW */
    // Cy_TrigMux_Connect1To1(UART1_RX_TO_DMA_TRIG, 0ul, TRIGGER_TYPE_LEVEL, 0ul);

    /* Interrupt Initialization */
    cy_stc_sysint_irq_t stc_sysint_irq_cfg;
    stc_sysint_irq_cfg.sysIntSrc = UART1_DW1_TX_IRQN;
    stc_sysint_irq_cfg.intIdx    = CPUIntIdx2_IRQn;
    stc_sysint_irq_cfg.isEnabled = true;
    Cy_SysInt_InitIRQ(&stc_sysint_irq_cfg);
    Cy_SysInt_SetSystemIrqVector(stc_sysint_irq_cfg.sysIntSrc, DW1_uart1_tx_IntHandler);
    NVIC_EnableIRQ(stc_sysint_irq_cfg.intIdx);

    /* SW Trigger */
    // Cy_TrigMux_SwTrigger(DW_SW_TRIG, TRIGGER_TYPE_EDGE, 1ul);

    /* Wait 1ms  */
    Cy_SysTick_DelayInUs(10000ul); // Need to check why this is required.
}

void Scb_UART1_IntrISR(void)
{
    // UART interrupt handler
    Cy_SCB_UART_Interrupt(CY_UART1_TYPE, &g_stc_uart_context_uart1);
}

void Scb_UART1_Event(uint32_t locEvents)
{
    
    switch (locEvents)
    {
    case CY_SCB_UART_TRANSMIT_IN_FIFO_EVENT:
        break;

    case CY_SCB_UART_TRANSMIT_DONE_EVENT:
        break;

    case CY_SCB_UART_RECEIVE_DONE_EVENT:
        /* Transmit received data */
        Cy_SCB_UART_Receive(CY_UART1_TYPE, &g_uart1_in_data[0], UART1_RX_UNIT_BYTE, &g_stc_uart_context_uart1);
        fifo_push(&uart1_rx_fifo, &g_uart1_in_data[0], UART1_RX_UNIT_BYTE);
        break;

    case CY_SCB_UART_RB_FULL_EVENT:
        break;

    case CY_SCB_UART_RECEIVE_ERR_EVENT:
        break;

    case CY_SCB_UART_TRANSMIT_ERR_EVENT:
        break;

    default:
        break;
    }
}

void DW1_uart1_tx_IntHandler(void)
{
    uint32_t masked;

    masked = Cy_PDMA_Chnl_GetInterruptStatusMasked(DW1, DW1_CH_TO_SCB_UART1_TX);
    if ((masked & CY_PDMA_INTRCAUSE_COMPLETION) != 0ul)
    {
        /* Clear Complete DMA interrupt flag */
        Cy_PDMA_Chnl_ClearInterrupt(DW1, DW1_CH_TO_SCB_UART1_TX);
        
        /* Mark the transmission as complete */
        uint32_t data_len = fifo_get(&uart1_tx_fifo, user_tx_data, USER_DW_TX_SIZE);
        if (data_len) {
            is_uart1_tx_Complete = 0;
            Cy_PDMA_Descr_SetSrcAddr(&dw_uart1_tx_descr, user_tx_data);
            dw_uart1_tx_descr.unPDMA_DESCR_Y_CTL.stcField.u8Y_COUNT = data_len-1;
            Cy_PDMA_Chnl_Enable(DW1, DW1_CH_TO_SCB_UART1_TX);
        } else {
            is_uart1_tx_Complete = 1;
        }
    }
}

void uart1_write_dma(uint8_t* src, uint32_t len)
{
    if (len) {
        fifo_push(&uart1_tx_fifo, src, len);
        if (is_uart1_tx_Complete == 1) {
            uint32_t data_len = fifo_get(&uart1_tx_fifo, user_tx_data, USER_DW_TX_SIZE);
            if (data_len) {
                is_uart1_tx_Complete = 0;
                Cy_PDMA_Descr_SetSrcAddr(&dw_uart1_tx_descr, user_tx_data);
                dw_uart1_tx_descr.unPDMA_DESCR_Y_CTL.stcField.u8Y_COUNT = data_len-1;
                Cy_PDMA_Chnl_Enable(DW1, DW1_CH_TO_SCB_UART1_TX);
            }
        }
    }
}

void Scb_UART6_IntrISR(void);
void Scb_UART6_Event(uint32_t locEvents);

void bsp_uart6_init(uint32_t boadrate, uint32_t sourceFreq)
{
    fifo_init(&uart6_rx_fifo, uart6_rx_buff, UART6_RX_FIFO_SIZE);
    fifo_init(&uart6_tx_fifo, uart6_tx_buff, UART6_TX_FIFO_SIZE);


    /* Port Configuration for UART */
    cy_stc_gpio_pin_config_t stc_port_pin_cfg_uart = {0ul};
    stc_port_pin_cfg_uart.driveMode = CY_GPIO_DM_HIGHZ;
    stc_port_pin_cfg_uart.hsiom = CY_UART6_RX_MUX;
    Cy_GPIO_Pin_Init(CY_UART6_RX_PORT, CY_UART6_RX_PIN, &stc_port_pin_cfg_uart);

    stc_port_pin_cfg_uart.driveMode = CY_GPIO_DM_STRONG_IN_OFF;
    stc_port_pin_cfg_uart.hsiom = CY_UART6_TX_MUX;
    Cy_GPIO_Pin_Init(CY_UART6_TX_PORT, CY_UART6_TX_PIN, &stc_port_pin_cfg_uart);

    /* SCB-UART Initialization */
    Cy_SCB_UART_DeInit(CY_UART6_TYPE);
    Cy_SCB_UART_Init(CY_UART6_TYPE, &g_stc_uart_config_uart6, &g_stc_uart_context_uart6);
    Cy_SCB_UART_RegisterCallback(CY_UART6_TYPE, Scb_UART6_Event, &g_stc_uart_context_uart6);
    Cy_SCB_UART_Enable(CY_UART6_TYPE);

    /* Clock Configuration for UART */
    // Assign a programmable divider
    Cy_SysClk_PeriphAssignDivider(CY_UART6_PCLK, CY_SYSCLK_DIV_24_5_BIT, 2);
    // Set divider value
    {
        uint64_t targetFreq     = UART6_OVERSAMPLING * boadrate;
        uint64_t sourceFreq_fp5 = ((uint64_t)sourceFreq << 5ull);
        uint32_t divSetting_fp5 = (uint32_t)(sourceFreq_fp5 / targetFreq);
        Cy_SysClk_PeriphSetFracDivider(Cy_SysClk_GetClockGroup(CY_UART6_PCLK),
                                       CY_SYSCLK_DIV_24_5_BIT,
                                       2,
                                       ((divSetting_fp5 & 0x1FFFFFE0ul) >> 5ul),
                                       (divSetting_fp5 & 0x0000001Ful));
    }
    // Enable peripheral divider
    Cy_SysClk_PeriphEnableDivider(Cy_SysClk_GetClockGroup(CY_UART6_PCLK), CY_SYSCLK_DIV_24_5_BIT, 2);

    /* Interrupt Configuration for UART */
    cy_stc_sysint_irq_t stc_sysint_irq_cfg_uart;
    stc_sysint_irq_cfg_uart.sysIntSrc = UART6_IRQN;
    stc_sysint_irq_cfg_uart.intIdx = CPUIntIdx0_IRQn;
    stc_sysint_irq_cfg_uart.isEnabled = true;
    Cy_SysInt_InitIRQ(&stc_sysint_irq_cfg_uart);
    Cy_SysInt_SetSystemIrqVector(stc_sysint_irq_cfg_uart.sysIntSrc, Scb_UART6_IntrISR);
    NVIC_EnableIRQ(stc_sysint_irq_cfg_uart.intIdx);

    Cy_SCB_UART_Receive(CY_UART6_TYPE, &g_uart6_in_data[0], UART6_RX_UNIT_BYTE, &g_stc_uart_context_uart6);
}

void Scb_UART6_IntrISR(void)
{
    /* UART interrupt handler */
    Cy_SCB_UART_Interrupt(CY_UART6_TYPE, &g_stc_uart_context_uart6);
}

void Scb_UART6_Event(uint32_t locEvents)
{
    switch (locEvents)
    {
    case CY_SCB_UART_TRANSMIT_IN_FIFO_EVENT:
        break;

    case CY_SCB_UART_TRANSMIT_DONE_EVENT:
        break;

    case CY_SCB_UART_RECEIVE_DONE_EVENT:
        Cy_SCB_UART_Receive(CY_UART6_TYPE, &g_uart6_in_data[0], UART6_RX_UNIT_BYTE, &g_stc_uart_context_uart6);
        fifo_push(&uart6_rx_fifo,&g_uart6_in_data[0],UART6_RX_UNIT_BYTE);
        break;

    case CY_SCB_UART_RB_FULL_EVENT:
        break;

    case CY_SCB_UART_RECEIVE_ERR_EVENT:
        break;

    case CY_SCB_UART_TRANSMIT_ERR_EVENT:
        break;

    default:
        break;
    }
}

void uart6_write_bytes(char* buff,int len)
{
    while(g_stc_uart_context_uart6.txStatus & CY_SCB_UART_TRANSMIT_ACTIVE)
    {
        ;
    }

    Cy_SCB_UART_Transmit(CY_UART6_TYPE, buff, len, &g_stc_uart_context_uart6);
}

void Scb_UART7_IntrISR(void);
void Scb_UART7_Event(uint32_t locEvents);
void DW1_uart7_tx_IntHandler(void);

void bsp_uart7_init(uint32_t boadrate, uint32_t sourceFreq)
{
    fifo_init(&uart7_rx_fifo, uart7_rx_buff, UART7_RX_FIFO_SIZE);
    fifo_init(&uart7_tx_fifo, uart7_tx_buff, UART7_TX_FIFO_SIZE);


    /* Port Configuration for UART */
    cy_stc_gpio_pin_config_t    stc_port_pin_cfg_uart = {0ul};
    stc_port_pin_cfg_uart.driveMode = CY_GPIO_DM_HIGHZ;
    stc_port_pin_cfg_uart.hsiom     = CY_UART7_RX_MUX;
    Cy_GPIO_Pin_Init(CY_UART7_RX_PORT, CY_UART7_RX_PIN, &stc_port_pin_cfg_uart);

    stc_port_pin_cfg_uart.driveMode = CY_GPIO_DM_STRONG_IN_OFF;
    stc_port_pin_cfg_uart.hsiom     = CY_UART7_TX_MUX;
    Cy_GPIO_Pin_Init(CY_UART7_TX_PORT, CY_UART7_TX_PIN, &stc_port_pin_cfg_uart);

    /* SCB-UART Initialization */
    Cy_SCB_UART_DeInit(CY_UART7_TYPE);
    Cy_SCB_UART_Init(CY_UART7_TYPE, &g_stc_uart_config_uart7, &g_stc_uart_context_uart7);
    Cy_SCB_UART_RegisterCallback(CY_UART7_TYPE, Scb_UART7_Event, &g_stc_uart_context_uart7);
    Cy_SCB_UART_Enable(CY_UART7_TYPE);

    /* Clock Configuration for UART */
    // Assign a programmable divider
    Cy_SysClk_PeriphAssignDivider(CY_UART7_PCLK, CY_SYSCLK_DIV_24_5_BIT, 3);
    // Set divider value
    {
        uint64_t targetFreq     = UART6_OVERSAMPLING * boadrate;
        uint64_t sourceFreq_fp5 = ((uint64_t)sourceFreq << 5ull);
        uint32_t divSetting_fp5 = (uint32_t)(sourceFreq_fp5 / targetFreq);
        Cy_SysClk_PeriphSetFracDivider(Cy_SysClk_GetClockGroup(CY_UART7_PCLK),
                                       CY_SYSCLK_DIV_24_5_BIT,
                                       3,
                                       ((divSetting_fp5 & 0x1FFFFFE0ul) >> 5ul),
                                       (divSetting_fp5 & 0x0000001Ful));
    }
    // Enable peripheral divider
    Cy_SysClk_PeriphEnableDivider(Cy_SysClk_GetClockGroup(CY_UART7_PCLK), CY_SYSCLK_DIV_24_5_BIT, 3);

    /* Interrupt Configuration for UART */
    cy_stc_sysint_irq_t stc_sysint_irq_cfg_uart;
    stc_sysint_irq_cfg_uart.sysIntSrc = UART7_IRQN;
    stc_sysint_irq_cfg_uart.intIdx    = CPUIntIdx6_IRQn;
    stc_sysint_irq_cfg_uart.isEnabled = true;
    Cy_SysInt_InitIRQ(&stc_sysint_irq_cfg_uart);
    Cy_SysInt_SetSystemIrqVector(stc_sysint_irq_cfg_uart.sysIntSrc, Scb_UART7_IntrISR);
    NVIC_EnableIRQ(stc_sysint_irq_cfg_uart.intIdx);

    // start to recieve
    Cy_SCB_UART_Receive(CY_UART7_TYPE, &g_uart7_in_data[0], UART7_RX_UNIT_BYTE, &g_stc_uart_context_uart7);

    

    /**************************************************/
    /* DW dedicating to UART TX and RX Initialization */
    /**************************************************/
    Cy_PDMA_Disable(DW1);

    /* DW channel dedicating to UART TX Initialization */
    Cy_PDMA_Chnl_DeInit(DW1, DW1_CH_TO_SCB_UART7_TX);
    Cy_PDMA_Descr_Init(&dw_uart7_tx_descr, &dw1ch_uart7_tx_config);
    Cy_PDMA_Chnl_Init(DW1, DW1_CH_TO_SCB_UART7_TX, &dw_uart7_tx_config);
    Cy_PDMA_Chnl_SetInterruptMask(DW1, DW1_CH_TO_SCB_UART7_TX);

    // /* DW channel dedicating to UART RX Initialization */
    // Cy_PDMA_Chnl_DeInit(DW1, DW1_CH_TO_SCB_UART7_RX);
    // Cy_PDMA_Descr_Init(&dwUartRxDescr, &dw1ChUartRxConfig);
    // Cy_PDMA_Chnl_Init(DW1, DW1_CH_TO_SCB_UART7_RX, &dwUartRxConfig);
    // Cy_PDMA_Chnl_SetInterruptMask(DW1, DW1_CH_TO_SCB_UART7_RX);
    
    Cy_PDMA_Enable(DW1);

    /**************************************/
    /* Trigger Multiplexer Initialization */
    /**************************************/
    /* UART TX To DW */
    Cy_TrigMux_Connect1To1(UART7_TX_TO_DMA_TRIG, 0ul, TRIGGER_TYPE_LEVEL, 0ul);
    /* UART RX To DW */
    // Cy_TrigMux_Connect1To1(UART7_RX_TO_DMA_TRIG, 0ul, TRIGGER_TYPE_LEVEL, 0ul);

    /* Interrupt Initialization */
    cy_stc_sysint_irq_t stc_sysint_irq_cfg;
    stc_sysint_irq_cfg.sysIntSrc = UART7_DW1_TX_IRQN;
    stc_sysint_irq_cfg.intIdx    = CPUIntIdx2_IRQn;
    stc_sysint_irq_cfg.isEnabled = true;
    Cy_SysInt_InitIRQ(&stc_sysint_irq_cfg);
    Cy_SysInt_SetSystemIrqVector(stc_sysint_irq_cfg.sysIntSrc, DW1_uart7_tx_IntHandler);
    NVIC_EnableIRQ(stc_sysint_irq_cfg.intIdx);

    /* SW Trigger */
    // Cy_TrigMux_SwTrigger(DW_SW_TRIG, TRIGGER_TYPE_EDGE, 1ul);

    /* Wait 1ms  */
    Cy_SysTick_DelayInUs(10000ul); // Need to check why this is required.
}

void Scb_UART7_IntrISR(void)
{
    /* UART interrupt handler */
    Cy_SCB_UART_Interrupt(CY_UART7_TYPE, &g_stc_uart_context_uart7);

}

void Scb_UART7_Event(uint32_t locEvents)
{
    switch (locEvents)
    {
    case CY_SCB_UART_TRANSMIT_IN_FIFO_EVENT:
        break;

    case CY_SCB_UART_TRANSMIT_DONE_EVENT:
        break;

    case CY_SCB_UART_RECEIVE_DONE_EVENT:
        /* Transmit received data */
        Cy_SCB_UART_Receive(CY_UART7_TYPE, &g_uart7_in_data[0], UART7_RX_UNIT_BYTE, &g_stc_uart_context_uart7);
        fifo_push(&uart7_rx_fifo,&g_uart7_in_data[0],UART7_RX_UNIT_BYTE);     
        break;

    case CY_SCB_UART_RB_FULL_EVENT:
        break;

    case CY_SCB_UART_RECEIVE_ERR_EVENT:
        break;

    case CY_SCB_UART_TRANSMIT_ERR_EVENT:
        break;

    default:
        break;
    }
}

void DW1_uart7_tx_IntHandler(void)
{
    uint32_t masked;

    masked = Cy_PDMA_Chnl_GetInterruptStatusMasked(DW1, DW1_CH_TO_SCB_UART7_TX);
    if ((masked & CY_PDMA_INTRCAUSE_COMPLETION) != 0ul)
    {
        /* Clear Complete DMA interrupt flag */
        Cy_PDMA_Chnl_ClearInterrupt(DW1, DW1_CH_TO_SCB_UART7_TX);
        
        /* Mark the transmission as complete */
        uint32_t data_len = fifo_get(&uart7_tx_fifo, debug_tx_data, DEBUG_DW_TX_SIZE);
        if (data_len) {
            is_uart7_tx_Complete = 0;
            Cy_PDMA_Descr_SetSrcAddr(&dw_uart7_tx_descr, debug_tx_data);
            dw_uart7_tx_descr.unPDMA_DESCR_Y_CTL.stcField.u8Y_COUNT = data_len-1;
            Cy_PDMA_Chnl_Enable(DW1, DW1_CH_TO_SCB_UART7_TX);
        } else {
            is_uart7_tx_Complete = 1;
        }
    }
}

void uart7_write_dma(uint8_t* src, uint32_t len)
{
    if (len) {
        fifo_push(&uart7_tx_fifo, src, len);
        if (is_uart7_tx_Complete == 1) {
            uint32_t data_len = fifo_get(&uart7_tx_fifo, debug_tx_data, DEBUG_DW_TX_SIZE);
            if (data_len) {
                is_uart7_tx_Complete = 0;
                Cy_PDMA_Descr_SetSrcAddr(&dw_uart7_tx_descr, debug_tx_data);
                dw_uart7_tx_descr.unPDMA_DESCR_Y_CTL.stcField.u8Y_COUNT = data_len-1;
                Cy_PDMA_Chnl_Enable(DW1, DW1_CH_TO_SCB_UART7_TX);
            }
        }
    }
}

void UART7_Printf(void *fmt, ...)
{
    uint8_t uart_out_data[1024];
    va_list arg;

    /* UART Print */
    va_start(arg, fmt);
    vsprintf((char*)&uart_out_data[0], (char*)fmt, arg);
    while (Cy_SCB_UART_IsTxComplete(CY_UART7_TYPE) != true) {};
    Cy_SCB_UART_PutArray(CY_UART7_TYPE, uart_out_data, strlen((char *)uart_out_data));
    va_end(arg);
}

void UART1_Printf(void *fmt, ...)
{
    uint8_t uart_out_data[512];
    va_list arg;

    /* UART Print */
    va_start(arg, fmt);
    vsprintf((char*)&uart_out_data[0], (char*)fmt, arg);
    while (Cy_SCB_UART_IsTxComplete(CY_UART1_TYPE) != true) {};
    Cy_SCB_UART_PutArray(CY_UART1_TYPE, uart_out_data, strlen((char *)uart_out_data));
    va_end(arg);
}
