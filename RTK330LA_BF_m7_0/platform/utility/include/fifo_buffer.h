#ifndef _FIFO_BUFFER_H_
#define _FIFO_BUFFER_H_

#include <stdint.h>

typedef struct
{
	uint8_t* buffer;
	uint16_t in;
	uint16_t out;
	uint16_t size;
} fifo_type;

void fifo_init(fifo_type* fifo, uint8_t* buffer, uint16_t size);
uint16_t fifo_get(fifo_type* fifo, uint8_t* buffer, uint16_t len);
uint16_t fifo_status(fifo_type* fifo);
void fifo_push(fifo_type* fifo, uint8_t* buffer, uint16_t size);


#endif /* _FIFO_BUFFER_H_ */
