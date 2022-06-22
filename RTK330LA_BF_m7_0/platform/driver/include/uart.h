#ifndef _UART_H_
#define _UART_H_

#include "utils.h"

/**********************************************************************/
/* Parameters Depending on TVII MCU Series                            */
/**********************************************************************/

// USER PORT
#define CY_UART1_RX_PORT                 GPIO_PRT20
#define CY_UART1_RX_PIN                  3
#define CY_UART1_RX_MUX                  P20_3_SCB1_UART_RX  
#define CY_UART1_TX_PORT                 GPIO_PRT20
#define CY_UART1_TX_PIN                  4
#define CY_UART1_TX_MUX                  P20_4_SCB1_UART_TX

#define CY_UART1_TYPE                    SCB1
#define CY_UART1_PCLK                    PCLK_SCB1_CLOCK

#define UART1_IRQN                       scb_1_interrupt_IRQn  
#define UART1_DW1_TX_IRQN                cpuss_interrupts_dw1_18_IRQn


// GNSS PORT
#define CY_UART6_RX_PORT                 GPIO_PRT21
#define CY_UART6_RX_PIN                  7
#define CY_UART6_RX_MUX                  P21_7_SCB6_UART_RX
#define CY_UART6_TX_PORT                 GPIO_PRT22
#define CY_UART6_TX_PIN                  1
#define CY_UART6_TX_MUX                  P22_1_SCB6_UART_TX

#define CY_UART6_TYPE                    SCB6
#define CY_UART6_PCLK                    PCLK_SCB6_CLOCK

#define UART6_IRQN                       scb_6_interrupt_IRQn 

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

extern fifo_type uart1_rx_fifo;
extern fifo_type uart6_rx_fifo;
extern fifo_type uart7_rx_fifo;

void bsp_uart1_init(uint32_t boadrate, uint32_t sourceFreq);
void bsp_uart6_init(uint32_t boadrate, uint32_t sourceFreq);
void bsp_uart7_init(uint32_t boadrate, uint32_t sourceFreq);

void uart1_write_dma(uint8_t* src, uint32_t len);
void uart6_write_bytes(char* buff, int len);
void uart7_write_dma(uint8_t* src, uint32_t len);

void UART1_Printf(void *fmt, ...);
void UART7_Printf(void *fmt, ...);

#endif
