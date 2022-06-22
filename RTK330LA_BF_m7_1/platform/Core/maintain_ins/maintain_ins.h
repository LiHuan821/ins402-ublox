#ifndef _MAINTAIN_INS_H_
#define _MAINTAIN_INS_H_

#include <stdint.h>

#define INS_SAVE_BUF_SIZE   156


#pragma pack(4)

typedef struct {
    double latitude;
    double longitude;
    double height;
    uint32_t crc;
} maintain_ins_status_t;

#pragma pack()

extern uint8_t maintain_ins_valid;
extern maintain_ins_status_t maintain_ins_status;

void maintain_ins_clear_inflash(void);
void maintain_ins_read_inflash(void);
void maintain_ins_write_inflash(void);
void maintain_ins_save_buf_inflash(void);
void maintain_ins_read_ins_save_buf(void);
void ins_save_buf_flash_erase(void);
#endif
