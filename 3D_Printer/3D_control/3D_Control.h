#ifndef ThreeD_Control_H
#define ThreeD_Control_H
#include "Configuration.h"
#include "Ali_Pri/command.h"
#include "Printer/Printer.h"
#include "Serial/UART.h"
#include "Service/Service.h"
#include "Termistor/Termistor.h"
#include "PIDR/PIDR.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "Light_WS2812/light_ws2812.h"


#define AXES_TIMER_PRESCALER 256
#define STEP_TIMER_UNIT 2

#define X_Timer_Register OCR1AA_X
#define Y_Timer_Register OCR1AB_Y
#define E_Timer_Register OCR1AD_E
#define Z_Timer_Register OCR1AC_Z

extern volatile uint16_t StepsSleepTimeout;
extern volatile uint16_t PID_CALIB_TIME;

char RX_Buffer[RX_Buffer_SIZE];
extern volatile ThreeD_Printer* iPrinter; 

void setup_printer();
void printer_serve();

//timers 
void start_axes_timer();
void stop_axes_timer();

static inline void PWM_timer_init()
{
     // Настройка Fast PWM 10-bit, неинвертирующий режим, делитель 64
     // Настройка режима: Fast PWM, 10-bit (WGM13:0 = 0b0011)
     // TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);
     TCCR1A = (1 << COM1A1) | (1 << COM1B1)  | (1 << WGM10);
     TCCR1B = (1 << WGM12) | (1 << CS11); // Предделитель 64 (CS11 и CS10)

     OCR1A = 0;
     OCR1B = 0;
} // Timer0
static inline void init_watch_dog_printer_timer(){
     TCCR0  |= (1 << WGM01) | (1 << CS02) | (1 << CS00); //10254
     OCR0 = 255;
     TIMSK |= (1 << OCIE0);
}//Timer2

static inline void init_axes_timer(){
     TCCR2  |= (1 << WGM21) |(1 << CS22) | (1 << CS21); //CTC 256
     OCR2  = STEP_TIMER_UNIT;
     TIMSK |= (1 << OCIE2);
}

unsigned int get_delay_timer(float speed, int  speedAt);
uint8_t stepTimersNull();
//Move
static inline int float_to_step(float distance_MM, int stepByMM){
    return round(distance_MM * stepByMM);
}
void move_to(float X,float Y, float Z, float E,int speedMMS);
void move(float X, float Y, float Z, float E, int speedMMS);

void Await();
inline int convert_ADC_to_temp(int ADCValue){

}
//Buffer and commands
void execute_next_command();
void add_in_buffer(char byte);

//temp
extern inline void set_temp_bed(uint8_t temp);
extern inline void set_temp_nozzle(int temp);
uint16_t read_adc(uint8_t channel);

void handle_X();
void handle_Y();
void handle_E();
void handle_Z();

extern inline void clear_RX();

void error(char* errorMsg);
void stop_print();
void panic();

void out_of_range(float X,float Y, float Z,float E);
float get_extruder_move(float nowMove);

inline float sque(float value){
     return value * value;
}
//GCode++
static inline void command_G0(const char *command)
{
     float X = 0, Y = 0, Z = 0, F = 0;
     while (*command != '\0')
     {
          if (*command == 'X' || *command == 'x')
          {
               X = parse_GCode_from_string(command + 1);
          }
          else if (*command == 'Y' || *command == 'y')
          {
               Y = parse_GCode_from_string(command + 1);
          }
          else if (*command == 'Z' || *command == 'z')
          {
               Z = parse_GCode_from_string(command + 1);
          }
          else if (*command == 'F' || *command == 'f')
          {
               F = parse_GCode_from_string(command + 1) / 60;
          }
          command++;
     }
     if (F <= 0){
          F = MaxSpeedMMS;
     }

     if(iPrinter->Flags & (1 << FlagIsAbsalute)){
          X = X-iPrinter->CurrentPosition.X;
          Y = X-iPrinter->CurrentPosition.Y;
          Z = X-iPrinter->CurrentPosition.Z;
    }
   
    move(X, Y, Z, 0, F);
}
static inline void command_G1 (const char* command){
     float X = 0, Y = 0, Z = 0, E = 0, F = 0;
     while (*command != '\0')
     {
          if (*command == 'X' || *command == 'x')
          {
               X = parse_GCode_from_string(command + 1);
          }else if(*command == 'Y' || *command == 'y'){
              Y = parse_GCode_from_string(command+1);
         }else if(*command == 'Z' || *command == 'z'){
              Z = parse_GCode_from_string(command+1);
         }else if(*command == 'E' || *command == 'e'){
              E = get_extruder_move(parse_GCode_from_string(command+1));
         }else if(*command == 'F' || *command == 'f'){
              F = parse_GCode_from_string(command+1) / 60;
         }
         command++;
    }
    if (F <= 0){
         F = iPrinter->speed;
    }else{
     iPrinter->speed = F;
    }
    if(iPrinter->Flags & (1 << FlagIsAbsalute)){
          X = X-iPrinter->CurrentPosition.X;
          Y = X-iPrinter->CurrentPosition.Y;
          Z = X-iPrinter->CurrentPosition.Z;
    }
    E = E*iPrinter->flow;
    move(X, Y, Z, E, F);
}

static inline void command_G4(const char* command){
    // char** split = strSplit(command,' ');
     
    // for(int i = 0; split[i]!= NULL; i++){
    //      if(strcasestr(split[i],"F")){
    //          int delay = atoi(split[i]+1);
    //           for(int i = 0; i < delay; i++){
    //                if (!(iPrinter->Flags & (1 << FlagISleep))){
    //                     iPrinter->Flags|= (1 << FlagISleep);
    //                }
    //                Await();
    //           }
    //      }
    // }
    // deleteSplit(split);
}

static inline void CalibrationPIDs(){
     UART_printf("PID calibration is start;");
     for (int P = 1; P != 10; P++){
         for(int I = 1; I != 10; I++){
             for(int D = 1;D!= 10; D++){
               iPrinter->NozzlePID->proportionallyCoef = P;
               iPrinter->NozzlePID->integralCoef = I;
               iPrinter->NozzlePID->differinialCoef = D;
               iPrinter->BedPID->proportionallyCoef = P;
               iPrinter->BedPID->integralCoef = I;
               iPrinter->BedPID->differinialCoef = D;
               iPrinter->Flags &= ~(1 << FlagColibrationPID);
               PID_CALIB_TIME =0;
               iPrinter->NozzlePID->needValue = 200;
               iPrinter->BedPID->needValue = 50;
               int maxN,maxB,midleN,MidleB = 0;
               uint32_t CounterN,CounterB = 0;

               while(!((iPrinter->Flags) & (1 << FlagColibrationPID))){
                    CounterN++;
                    CounterB++;
                    if(iPrinter->NozzlePID->needValue - iPrinter->tempNozzle < maxN){
                         maxN = (iPrinter->NozzlePID->needValue - iPrinter->tempNozzle < 0) ? 
                         iPrinter->NozzlePID->needValue - iPrinter->tempNozzle : 
                         iPrinter->NozzlePID->needValue - iPrinter->tempNozzle *-1;
                    }    
                    midleN = (uint32_t)midleN *((float)iPrinter->tempNozzle/(float)CounterN) ;
                    MidleB = (uint32_t)MidleB *((float)iPrinter->tempBed/(float)CounterB) ;
                    if(iPrinter->BedPID->needValue - iPrinter->tempBed < maxN){
                         maxN = (iPrinter->BedPID->needValue - iPrinter->tempBed < 0) ? 
                         iPrinter->BedPID->needValue - iPrinter->tempBed : 
                         iPrinter->BedPID->needValue - iPrinter->tempBed *-1;
                    }    
                    Await();
               }
               PID_CALIB_TIME =0;
               PIDR_set_need_value(iPrinter->NozzlePID,0);
               PIDR_set_need_value(iPrinter->BedPID,0);
               UART_printf("calib with P:%d,I:%d,D:%d complete;",P,I,D);
               UART_printf("results:maxNozzle:%d, maxBed:%d;",maxN,maxB);
               UART_printf("MidleNozzle:%d, MidleBed:%d;",midleN,MidleB);
               //Cooling

                 iPrinter->Flags &= ~(1 << FlagColibrationPID);
                 while(!(iPrinter->Flags) & (1 << FlagColibrationPID)){
                    
                 }
             }
         }
     }
 }
 
static inline void Command_G10(char* command){
     float E,F = 0;
     while(*command != '\0'){
          if(*command == 'E'){
               E = parse_GCode_from_string(command + 1);
          }else if(*command == 'F'){
               F = parse_GCode_from_string(command +1);
          }
          command++;
     }
     if(E == 0 && F==0){return;}

     if(F == 0){
          F = StandartSpeed;
     }
     move(0,0,0,E,F);
} //retract
static inline void Command_G11 (char* command){
     float E,F = 0;
     while(*command != '\0'){
          if(*command == 'E'){
               E = parse_GCode_from_string(command + 1);
          }else if(*command == 'F'){
               F = parse_GCode_from_string(command +1);
          }
          command++;
     }
     if(E == 0 && F==0){return;}

     if(F == 0){
          F = StandartSpeed;
     }
     move(0,0,0,-E,F);
}
void home_position(); //G28

static inline void Command_G90 (){
     iPrinter->Flags |= (1 << FlagIsAbsalute);
}//set absolute coord
static inline void Command_G91 (){
     iPrinter->Flags &= ~(1 << FlagIsAbsalute);
}
static inline void Command_G92(char *command)
{
     float X = -1, Y = -1, Z = -1, E = -1;
     while (*command != '\0')
     {
          if (*command == 'X' || *command == 'x')   {
               X = parse_GCode_from_string(command + 1);
          }
          else if (*command == 'Y' || *command == 'y') {
               Y = parse_GCode_from_string(command + 1);
          }
          else if (*command == 'Z' || *command == 'z') {
               Z = parse_GCode_from_string(command + 1);
          }
          else if (*command == 'E' || *command == 'e') {
               E = get_extruder_move(parse_GCode_from_string(command + 1));
          }
          command++;
     }
     iPrinter->CurrentPosition.X = (X == -1) ? iPrinter->CurrentPosition.X : X;
     iPrinter->CurrentPosition.Y = (Y == -1) ? iPrinter->CurrentPosition.Y : Y;
     iPrinter->CurrentPosition.Z = (Z == -1) ? iPrinter->CurrentPosition.Z : Z;
     iPrinter->CurrentPosition.E = (E == -1) ? iPrinter->CurrentPosition.E : E;
}
//Gcode--

//MCode++
static inline void execute_MCode(const char* prefix, const char* command);
static inline void heat_bed_command(const char* command,uint8_t wait); //M140 | M190
static inline void heat_nozzle_command(const char* command,uint16_t wait);//M104 | M109
static inline void Command_M82(){ //set absolute extruder mode
    iPrinter->Flags |= (1 << FlagExtruderIsAbsalute);
}
static inline void Command_M83(){
    iPrinter->Flags &= ~(1 << FlagExtruderIsAbsalute);
}
static inline void enable_steps(){ //M17
     EndStopsAndAnableStepsPORT &= ~(1 << StepStatePin);
     StepsSleepTimeout = 0;
}
static inline void disable_steps(){ //M18
     EndStopsAndAnableStepsPORT |= (1 << StepStatePin);
}

static inline void set_fan_value(const char* command){
     uint8_t NumberFan = 1;
     uint8_t value = 0;
     while (*command != '\0'){
          if(*command == 'P' || *command == 'p'){
               NumberFan =  (int)parse_GCode_from_string(command + 1);
          }
          if(*command == 'S' || *command == 's'){
               value = (int)parse_GCode_from_string(command + 1);
          }
          command++;
     }
     UART_printf("Fan value:%d",value);
     if(NumberFan == 1){
          iPrinter->fan1 = value;
     }else if(NumberFan = 2){
          iPrinter->fan2 = value;
     }
}

static inline void diasble_fan(const char* command){
     uint8_t NumberFan = 1;
     uint8_t value = 0;
     while (*command != '\0'){
          if(*command == 'P' || *command == 'p'){
               NumberFan =  (int)parse_GCode_from_string(command + 1);
          }
          command++;
     }
     if(NumberFan == 1){
          iPrinter->fan1 = 0;
     }else if(NumberFan = 2){
          iPrinter->fan2 = 0;
     }
}
//MCode--

#endif