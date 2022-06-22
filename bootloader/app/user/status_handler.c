#include "status_handler.h"
#include <string.h>

status_t g_status;

void init_status_struct(void)
{
    memset(&g_status, 0, sizeof(status_t));
    g_status.status_bit.power = 1;
    g_status.status_bit.MCU_status = 1;
}


