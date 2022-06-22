#ifndef _M7_IPC_H_
#define _M7_IPC_H_

#include <stdint.h>
#include "ucb_packet.h"

#if GT_PACKET == 1
#define EP_INDEX_INS_CPU         0
#define EP_INDEX_RTK_CPU         1


#pragma pack(1)
typedef struct
{
    uint32_t clientId;  /* PIPE I/F internal use area. Must be first element of the structure. */
    ucb_GT_struct gnss_time;
} custom_data_t;
#pragma pack()

void ipc_rev_init(void);
#endif

#endif 
