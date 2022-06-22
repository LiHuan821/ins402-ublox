#ifndef _M7_IPC_H_
#define _M7_IPC_H_

#include <stdint.h>
#if GT_PACKET == 1

#define EP_INDEX_RTK_CPU         1
#define EP_INDEX_INS_CPU         0

#pragma pack(1)
typedef struct {
    uint32_t time_now;
    uint16_t week;
} out_GT_struct;

typedef struct
{
    uint32_t clientId;  /* PIPE I/F internal use area. Must be first element of the structure. */
    out_GT_struct gnss_time;
} custom_data_t;
#pragma pack()

void ipc_send_init(void);

int send_ipc_pipe_data(custom_data_t* data);

#endif

#endif 

