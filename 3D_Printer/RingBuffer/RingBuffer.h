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




typedef struct 
{
    unsigned char data[commandsBufferSize];
    size_t  readPos;
    size_t  writePos;    
}Buffio;


    static uint8_t Buffio_isEmpty(Buffio* buffio)  { return buffio->readPos == buffio->writePos; }
    static uint8_t Buffio_isFull(Buffio* buffio)  { return ((buffio->writePos + 1) % commandsBufferSize) == buffio->readPos; }

    static size_t Buffio_available(Buffio* buffio)  {
        if(buffio->writePos >= buffio->readPos)
            return buffio->writePos - buffio->readPos;
        else
            return commandsBufferSize - (buffio->readPos - buffio->writePos);
    }

    static size_t Buffio_freeSpace(Buffio* buffio)
     { return commandsBufferSize - Buffio_available(buffio) - 1; }

    static int Buffio_WriteNext(Buffio* buffio,const uint8_t* buffer, int len) {
        if(len > (int)Buffio_freeSpace(buffio))
            return -1;

        for(int i = 0; i < len; i++) {
            buffio->data[buffio->writePos] = buffer[i];
            buffio->writePos       = (buffio->writePos + 1) % commandsBufferSize;
        }
        return len;
    }

   static uint8_t Buffio_WriteByte(Buffio* buffio,const uint8_t byte) {
        if(!Buffio_freeSpace(buffio)){
            return -1;
        }
            buffio->data[buffio->writePos] = byte;
            buffio->writePos = (buffio->writePos + 1) % commandsBufferSize;
        return 1;    
    }

    // int Buffio_WriteNext(Buffio* buffio,const char* buffer, int len) {
    //     return WriteNext(reinterpret_cast<const uint8_t*>(buffer), len);
    // }

    static char* Buffio_allBuffer(Buffio* buffio){
        return buffio->data;
    }

    static int Buffio_ReadBytes(Buffio* buffio,uint8_t* buffer, int len) {
        int counter = 0;
        while(len) {
            buffer[counter] = buffio->data[buffio->readPos];
            buffio->readPos         = (buffio->readPos + 1) % commandsBufferSize;
            counter++;
        }
        return len;
    }

    // int Buffio_ReadBytes(Buffio* buffio,char* buffer, int len) {
    //     return ReadBytes(reinterpret_cast<uint8_t*>(buffer), len);
    // }

    // int Buffio_ReadLine(Buffio* buffio,uint8_t* buffer, int maxLen, uint8_t delim) {
    //     int count = 0;
    //     while(!isEmpty() && maxLen > 0) {
    //         uint8_t c = buffio->data[buffio->readPos];
    //         buffio->readPos   = (buffio->readPos + 1) % commandsBufferSize;
    //         if(c == delim)
    //             break;
    //         buffer[count++] = c;
    //         maxLen--;
    //     }
    //     return count;
    // }

    // int Buffio_ReadLine(Buffio* buffio,char* buffer, int maxLen, uint8_t delim) {
    //     return ReadLine(reinterpret_cast<uint8_t*>(buffer), maxLen, delim);
    // }

    static int Buffio_ReadLine(Buffio* buffio,char* buffer, int maxLen, const char* delim) {
        int    count    = 0;
        size_t delimLen = strlen(delim);
        while(maxLen) {
            if(Buffio_isEmpty(buffio)) {
                continue;
            }
            buffer[count] = buffio->data[buffio->readPos];
            buffio->readPos       = (buffio->readPos + 1) % commandsBufferSize;
            count++;
            maxLen--;

            if(count >= (int)delimLen &&
               memcmp(&buffer[count - delimLen], delim, delimLen) == 0) {
                buffer[count - delimLen] = '\0';
                return count - delimLen;
            }
        }
        return count;
    }


#endif