/***************************************************************************//**
* \file m7_ipc.c
*
* \brief
* daich
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_project.h"
#include "cy_device_headers.h"
#include "m7_ipc.h"

#if GT_PACKET == 1

bool released = true;

cy_en_ipc_pipe_status_t status;

cy_stc_ipc_pipe_config_t pipeConfig = 
{
    .epIndexForThisCpu = EP_INDEX_RTK_CPU,
    .epConfigData      = CY_IPC_PIPE_ENDPOINTS_DEFAULT_CONFIG,
};

/* Call when the other end point release the handle corresponding this end point */
void ReleaseCallback(void)
{
    /* Notified core already got the data. */
    /* Update send data for next transmission. */
    released = true;
}



void ipc_send_init(void)
{
    Cy_SysTick_DelayInUs(500000ul);     //wait for ins ipc init
    /* Initialize PIPE */
    Cy_IPC_Pipe_Init(&pipeConfig);

    NVIC_ClearPendingIRQ(pipeConfig.epConfigData[EP_INDEX_RTK_CPU].ipcCpuIntIdx);
    NVIC_EnableIRQ(pipeConfig.epConfigData[EP_INDEX_RTK_CPU].ipcCpuIntIdx);
}


int send_ipc_pipe_data(custom_data_t* data)
{
    while(released == false)
    {
        ;
        // Cy_SysLib_Delay(100);
    }
    released = false;
    status = Cy_IPC_Pipe_SendMessage
    (
        EP_INDEX_INS_CPU,
        data,
        ReleaseCallback
    );
    
    /* When the data successfully into the pipe */
    if (status == CY_IPC_PIPE_SUCCESS)
    {
        return true;
    }
    
    return false;
}
#endif
