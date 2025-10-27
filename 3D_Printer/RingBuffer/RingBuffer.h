#ifndef RINGBUFFER_H 
#define RINGBUFFER_H
#include "Configuration.h"
#include "string.h"

typedef struct RingBuffer
{
    volatile char commands[8][commandsBufferSize];
    volatile uint8_t resX;
    volatile uint8_t traX;
    volatile uint8_t bitVector;
} RingBuffer;

//void RingBuffer_init_buffer(RingBuffer* buffer);
void RingBuffer_delete_and_next(volatile RingBuffer* buffer);
char* RingBuffer_get_now_command(volatile RingBuffer* buffer);
uint8_t RingBuffer_capacity(volatile RingBuffer* buffer);
void RingBuffer_add_element(volatile RingBuffer* buffer,const char* element);

#endif