#ifndef PRINTER_H
#define PRINTER_H
#define SETTINGS_MAGIC 0xBEEA

#include <3D_control/PIDR/PIDR.h>
#include <RingBuffer/RingBuffer.h>
#include <avr/eeprom.h>

typedef struct Position {
    float X;
    float Y;
    float Z;
    float E;
} Position;

typedef struct Settings {
    uint16_t magic;
    // char UniqueKey[9];
    int steps_to_mm_X;
    int steps_to_mm_Y;
    int steps_to_mm_Z;
    int steps_to_mm_E;
    float z_offset;
} Settings;

typedef struct {
    uint32_t magic;
    char CustomName[25];
} BaseSettings;

static Settings EEMEM settings_eeprom;
static BaseSettings EEMEM BaseSettings_eeprom;

typedef struct Steps {
    volatile uint32_t CurrentXsteps;
    volatile uint32_t CurrentYsteps;
    volatile uint32_t CurrentZsteps;
    volatile uint32_t CurrentEsteps;

    uint8_t motionX;
    uint8_t motionY;
    uint8_t motionZ;
    uint8_t motionE;
    
    int speedAtX;
    int speedAtY;
    int speedAtZ;
    int speedAtE;
} Steps;

#define FlagUARTTimeOut 0
#define FlagIsAbsalute 1
#define FlagExtruderIsAbsalute 2
#define FlagUpdateTemps 3
#define FlagGoHome 4
#define FlagIMove 5
#define FlagDebug 6
#define FlagEstep 7

typedef struct {
    Buffio buffio;

    Settings settings;

    struct Speed {
        float X;
    };
    volatile uint8_t Flags;
    PIDR NozzlePID;
    PIDR BedPID;
    int tempNozzle;
    int tempBed;

    float flowrate;
    uint8_t feedrate;
    float speed;

    uint8_t fan1;
    uint8_t fan2;

    volatile Position CurrentPosition;
    volatile Steps Steps;
} ThreeD_Printer;

#endif