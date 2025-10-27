#include "RingBuffer.h"



void RingBuffer_delete_and_next(volatile RingBuffer* buffer){
    buffer->bitVector &= ~(1<<(buffer->traX));
    memset((void*)buffer->commands[buffer->traX],0,commandsBufferSize);
    if(buffer->traX == 7){
        buffer->traX =0;
    }else{
        buffer->traX++;
    }
} 

char* RingBuffer_get_now_command(volatile RingBuffer* buffer){
    return buffer->commands[buffer->traX];
}

uint8_t RingBuffer_capacity(volatile RingBuffer* buffer){
    uint8_t res = 0;
    for(int i = 0; i < 8; i++){
        if(((1 << i) & buffer->bitVector) == 0){
            res++;
        }
    }
    return res;
}

uint8_t RingBuffer_MaxSize(){
    return 8;
}


void RingBuffer_add_element(volatile RingBuffer* buffer,const char* element){
    strcpy(buffer->commands[buffer->resX],element);
    buffer->bitVector |= (1 << buffer->resX);
    if(buffer->resX == 7){
        buffer->resX =0;
    }else{
        buffer->resX++;
    }
}
