#include <stdint.h>
#include <string.h>
#include "cy_project.h"
#include "app_version.h"
#include "taskRTK.h"
#include "m7_ipc.h"
#include "timer.h"
#include "fifo_buffer.h"
#include "share_memory.h"
#ifdef RTK330LA
#include "led.h"
#endif

int main(void)
{
    SystemInit();

    SCB_DisableDCache(); // Disables D cache because DMA also reads descriptor in the SRAM.
    __enable_irq(); /* Enable global interrupts. */

    // push_rtk_compile_message();
    /* Wait 1ms  */
    Cy_SysTick_DelayInUs(1000000ul); // Need to check why this is required.

    fifo_init(fifo_user_uart_base, fifo_user_uart_base_buf, RTCM_FIFO_DATA_SIZE);
    fifo_init(fifo_gps_uart_rover, fifo_gps_uart_rover_buf, RTCM_FIFO_DATA_SIZE);

    system_timer_init();

#ifdef RTK330LA
    bsp_led_init();
#endif
    
#if GT_PACKET == 1
    ipc_send_init();
#endif

    gnss_rtk_task();

    for (;;) {
    }
}
