#include "uart.h"
#include "cy_project.h"
#include "cy_device_headers.h"
#include <stdio.h>
#include <stdarg.h>



// UART7
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


static uint8_t g_uart7_in_data[128];
#define UART_USER_RX_FIFO_SIZE  (1024 * 4)
#define UART_USER_TX_FIFO_SIZE  (1024 * 4)


fifo_type uart_user_rx_fifo;
fifo_type uart_user_tx_fifo;

uint8_t uart_user_rx_buff[UART_USER_RX_FIFO_SIZE];
uint8_t uart7_tx_buff[UART_USER_TX_FIFO_SIZE];


void Scb_UART7_IntrISR(void);
void Scb_UART7_Event(uint32_t locEvents);
void DW1_uart7_tx_IntHandler(void);

void bsp_uart7_init(uint32_t boadrate, uint32_t sourceFreq)
{
    fifo_init(&uart_user_tx_fifo, uart7_tx_buff, UART_USER_TX_FIFO_SIZE);
    fifo_init(&uart_user_rx_fifo, uart_user_rx_buff, UART_USER_RX_FIFO_SIZE);

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
        uint64_t targetFreq     = UART7_OVERSAMPLING * boadrate;
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

    
    Cy_PDMA_Enable(DW1);

    /**************************************/
    /* Trigger Multiplexer Initialization */
    /**************************************/
    /* UART TX To DW */
    Cy_TrigMux_Connect1To1(UART7_TX_TO_DMA_TRIG, 0ul, TRIGGER_TYPE_LEVEL, 0ul);
    /* UART RX To DW */

    /* Interrupt Initialization */
    cy_stc_sysint_irq_t stc_sysint_irq_cfg;
    stc_sysint_irq_cfg.sysIntSrc = UART7_DW1_TX_IRQN;
    stc_sysint_irq_cfg.intIdx    = CPUIntIdx2_IRQn;
    stc_sysint_irq_cfg.isEnabled = true;
    Cy_SysInt_InitIRQ(&stc_sysint_irq_cfg);
    Cy_SysInt_SetSystemIrqVector(stc_sysint_irq_cfg.sysIntSrc, DW1_uart7_tx_IntHandler);
    NVIC_EnableIRQ(stc_sysint_irq_cfg.intIdx);

    /* SW Trigger */

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
        fifo_push(&uart_user_rx_fifo, &g_uart7_in_data[0], UART7_RX_UNIT_BYTE);
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
        uint32_t data_len = fifo_get(&uart_user_tx_fifo, debug_tx_data, DEBUG_DW_TX_SIZE);
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
        fifo_push(&uart_user_tx_fifo, src, len);
        if (is_uart7_tx_Complete == 1) {
            uint32_t data_len = fifo_get(&uart_user_tx_fifo, debug_tx_data, DEBUG_DW_TX_SIZE);
            if (data_len) {
                is_uart7_tx_Complete = 0;
                Cy_PDMA_Descr_SetSrcAddr(&dw_uart7_tx_descr, debug_tx_data);
                dw_uart7_tx_descr.unPDMA_DESCR_Y_CTL.stcField.u8Y_COUNT = data_len-1;
                Cy_PDMA_Chnl_Enable(DW1, DW1_CH_TO_SCB_UART7_TX);
            }
        }
    }
}

