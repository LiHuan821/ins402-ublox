#ifndef _CAN_H_
#define _CAN_H_

#include "cy_project.h"
#include "cy_device_headers.h"

/* CAN in Use */
/* !!!! Please modify according to your HW !!! */
#define CY_CANFD_TYPE     CY_CANFD1_2_TYPE
#define CY_CANFD_RX_PORT  GPIO_PRT3
#define CY_CANFD_RX_PIN   7
#define CY_CANFD_TX_PORT  GPIO_PRT3
#define CY_CANFD_TX_PIN   6
#define CY_CANFD_RX_MUX   P3_7_CANFD1_TTCAN_RX2
#define CY_CANFD_TX_MUX   P3_6_CANFD1_TTCAN_TX2
#define CY_CANFD_PCLK     PCLK_CANFD1_CLOCK_CAN2
#define CY_CANFD_IRQN     canfd_1_interrupts0_2_IRQn
#define NON_ISO_OPERATION (0)


#define CY_CANFD_AB_PORT  GPIO_PRT17
#define CY_CANFD_AB_PIN   5
#define CY_CANFD_AB_MUX   P17_5_GPIO
#define CY_CANFD_EN_PORT  GPIO_PRT17
#define CY_CANFD_EN_PIN   3
#define CY_CANFD_EN_MUX   P17_3_GPIO


void can_init(void);

#endif
/* [] END OF FILE */
