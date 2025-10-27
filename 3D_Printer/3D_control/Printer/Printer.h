#ifndef PRINTER_H
#define PRINTER_H
#include <RingBuffer/RingBuffer.h>
#include <3D_control/PIDR/PIDR.h>

typedef struct Position
{
    float X;
    float Y;
    float Z; 
    float E;
} Position;

typedef struct Steps
{
    volatile int nowXsteps;
    volatile int nowYsteps;
    volatile int nowZsteps;
    volatile int nowEsteps;

    int speedAtX;
    int speedAtY;
    int speedAtZ;
    int speedAtE; 
}Steps;

#define FlagUARTTimeOut 0
#define FlagIsAbsalute 1
#define FlagExtruderIsAbsalute 2
#define FlagUpdateTemps 3
#define FlagGoHome 4
#define FlagIMove 5
#define FlagColibrationPID 6
#define FlagEstep 7

typedef struct
{
    volatile RingBuffer* buffer;
    volatile uint8_t Flags;
    float speed;
    PIDR* NozzlePID;
    PIDR* BedPID;
    int tempNozzle;
    int tempBed;
    // int NeedTempNozzle;
    // uint8_t NeedTempBed;

    float flow;
    uint8_t fan1;
    uint8_t fan2;
    
    volatile Position CurrentPosition;
    volatile Steps Steps;
} ThreeD_Printer;

#endif