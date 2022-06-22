#ifndef _UART_H_
#define _UART_H_

#include "fifo.h"

/**********************************************************************/
/* Parameters Depending on TVII MCU Series                            */
/**********************************************************************/
// DEBUG PORT
#define CY_UART7_RX_PORT                 GPIO_PRT23
#define CY_UART7_RX_PIN                  0
#define CY_UART7_RX_MUX                  P23_0_SCB7_UART_RX  
#define CY_UART7_TX_PORT                 GPIO_PRT23
#define CY_UART7_TX_PIN                  1
#define CY_UART7_TX_MUX                  P23_1_SCB7_UART_TX

#define CY_UART7_TYPE                    SCB7
#define CY_UART7_PCLK                    PCLK_SCB7_CLOCK

#define UART7_IRQN                       scb_7_interrupt_IRQn  
#define UART7_DW1_TX_IRQN                cpuss_interrupts_dw1_30_IRQn


extern fifo_type uart_user_rx_fifo;
void uart7_write_dma(uint8_t* src, uint32_t len);

#endif
