//#define DEBUG
#define DEBUG_STR
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <avr/io.h>

//do not tuch
#define CastomTermistor 0
#define TERMISTER_NTC100k 1
//

#define HAS_LIGHT 0
#define RGB_LIGHT 0

//Base
#define PrinterName "ATM32"
#define S_Version "1.1.0"
#define PrinterType "FDM"
//UART
#define BaudRate 9600
#define commandsBufferSize 70
#define RX_Buffer_SIZE commandsBufferSize
#define UART_Timeout_S 3

//CastomTermistor,TERMISTER_NTC100k... todo
#define NozzleThetmistor TERMISTER_NTC100k
#define BedThermistor TERMISTER_NTC100k

//STEPS TO MM
#define X_STEPS_MM 160 //old is 160
#define Y_STEPS_MM 160 // old is 160
#define Z_STEPS_MM 400
#define E_STEPS_MM 500
#define STEPS_TIMEOUT_S 60
#define HOME_POSITION_TIMEOUT_S 10
//
#define MaxSpeedMMS 200
#define StandartSpeed 50
//heat
#define PWM_DDR DDRD
#define PWM_PORT PORTD 
//PB0  PB1 - UART RX & TX
#define NOZZLE_HEAT_PIN PD4 //Timer 1 (OC1B)
#define BED_HEAT_PIN PD5 //Timer1 (OC1A)
#define NOZZLE_REGISTER OCR1B
#define BED_REGISTER OCR1A

#define FAN_PORT PORTD
#define FAN1_CONTROL_PIN PD6
#define FAN2_CONTROL_PIN PD7

#define SIZE_X_MM 200.0f
#define SIZE_Y_MM 200.0f
#define SIZE_Z_MM 200.0f
//PINS

#define EndStopsAndAnableStepsDDR DDRB
#define EndStopsAndAnableStepsPORT PORTB
#define EndStopsAndAnableStepsPIN PINB
#define EndstopX PB0
#define EndstopY PB1
#define EndstopZ PB2
#define StepStatePin PB3
#define LedPort DDRB
#define LedPin PB4
#define Pixels 63

#define TermisterNozzle PA0
#define TermisterBed PA1
    
//STEP Driver
#define AXES_DDR   DDRC
#define AXES_PORT  PORTC
#define X_DIR_PORT PORTC0 
#define Y_DIR_PORT PORTC2
#define Z_DIR_PORT PORTC4
#define E_DIR_PORT PORTC6

#define X_STEP_PORT PORTC1
#define Y_STEP_PORT PORTC3
#define Z_STEP_PORT PORTC5
#define E_STEP_PORT PORTC7


#define INVERT_X 0
#define INVERT_Y 0
#define INVERT_Z 0
#define INVERT_E 0


#define INVERT_X_EDNSTOPS 0
#define INVERT_Y_EDNSTOPS 0
#define INVERT_Z_EDNSTOPS 0


//PID
#define PID_CALIB_TIME_S 180

//OPTIONAL
#define BoozerUsed 
#define ScreenUsed 




#endif