#include "3D_Control.h"

volatile ThreeD_Printer* iPrinter = NULL; 

volatile uint16_t TCNT1A_X = 0;
volatile uint16_t TCNT1B_Y = 0;
volatile uint16_t TCNT1C_Z = 0;
volatile uint16_t TCNT1D_E = 0;



volatile uint16_t OCR1AA_X = 0;
volatile uint16_t OCR1AB_Y = 0;
volatile uint16_t OCR1AC_Z = 0;
volatile uint16_t OCR1AD_E = 0;

volatile uint16_t StepsSleepTimeout   = 0;
volatile uint16_t HomePositionTimeout = 0;
volatile uint16_t CheckTempTimeout    = 0;
volatile uint8_t UATRTimeOut = 0;
volatile uint16_t PID_CALIB_TIME = 0;
volatile uint8_t ProgramPWM = 0;

void setup_printer(){
    UART_init(BaudRate); 
    UART_set_call_back_RX(add_in_buffer);   
    clear_RX();
    iPrinter = malloc(sizeof(ThreeD_Printer));
    if(iPrinter == NULL){
          error(MemoryAllocError);
    }
    memset((void*)iPrinter,0,sizeof(ThreeD_Printer));
    iPrinter->buffer = malloc(sizeof(RingBuffer));
    if(iPrinter->buffer ==NULL){
          error(MemoryAllocError);
    }
    memset((void*)iPrinter->buffer,0,sizeof(RingBuffer));
    iPrinter->NozzlePID = new_PIDR(3.0,0.4,2.0,&NOZZLE_REGISTER);
    iPrinter->BedPID = new_PIDR(3.0,0.3,2.0,&BED_REGISTER);
    iPrinter->flow = 1.0;
    //Ports setup
    AXES_DDR = 255;
    AXES_PORT = 0; //disable all ports
    PWM_PORT = 0;
    PWM_DDR  |= (1 << NOZZLE_HEAT_PIN) | (1 << BED_HEAT_PIN) | (1 << FAN1_CONTROL_PIN) | (1 << FAN2_CONTROL_PIN);
  
   
    EndStopsAndAnableStepsDDR &= ~((1 << EndstopX) | (1 << EndstopY) | (1 << EndstopZ));               
    EndStopsAndAnableStepsDDR |= (1 << StepStatePin);
    LedPort |= (1 << LedPin);
    //Max time interval for timer and max speed
    iPrinter->Steps.speedAtY = (1 / (((float)AXES_TIMER_PRESCALER * STEP_TIMER_UNIT) / F_CPU)) / Y_STEPS_MM;     
    iPrinter->Steps.speedAtX = (1 / (((float)AXES_TIMER_PRESCALER * STEP_TIMER_UNIT) / F_CPU)) / X_STEPS_MM;
    iPrinter->Steps.speedAtE = (1 / (((float)AXES_TIMER_PRESCALER * STEP_TIMER_UNIT) / F_CPU)) / E_STEPS_MM;
    iPrinter->Steps.speedAtZ = (1 / (((float)AXES_TIMER_PRESCALER * STEP_TIMER_UNIT) / F_CPU)) / Z_STEPS_MM;
    iPrinter->speed = StandartSpeed;
    ADC_Init();
    PWM_timer_init();//timer 0
    init_axes_timer(); //timer 1
    init_watch_dog_printer_timer(); //timer2
    
    stop_axes_timer(); //reset all data in registers
    sei();
    enable_steps();
    
    iPrinter->fan2 = 255; //radiator fan is enable (M106 && M107 control him)
    //For user
    blickLight(0,255,0); //
    UART_printf("Printer setup sucses!");
    //
    set_light(255,255,255);
}

void printer_serve(){
     while(1){
          if(RingBuffer_capacity(iPrinter->buffer) < 8){  
               execute_next_command();
               RingBuffer_delete_and_next(iPrinter->buffer);
               UART_send_command(EndcommandByte,ACK);
               UATRTimeOut = 0;
          }  
     }
}

void add_in_buffer(char byte){
     int len = strlen(RX_Buffer);
     if((len == RX_Buffer_SIZE - 1)&&
     (byte != EndcommandByte)){
          error(RXBufferOverfloError);
     }
     if(byte == EndcommandByte){
          RX_Buffer[len] = '\0';
          RingBuffer_add_element(iPrinter->buffer,(char*)RX_Buffer);
          clear_RX();
     }else{
         RX_Buffer[len] = byte;
         RX_Buffer[len+1] = '\0';
     }
}


void set_light(uint8_t R,uint8_t G,uint8_t B){
     struct cRGB led[0];
     led[0].r = R;
     led[0].g = G;
     led[0].b = B;
    for(int i = 0; i < Pixels; i++){
          ws2812_sendarray((uint8_t *)led,3);
     }
     
}
void execute_next_command(){
     char *command = RingBuffer_get_now_command(iPrinter->buffer);
     char prefix[5];
     prefix[0] = command[0] == ' ' ? '\0' : command[0];
     prefix[1] = command[1] == ' ' ? '\0' : command[1];
     prefix[2] = command[2] == ' ' ? '\0' : command[2];
     prefix[3] = command[3] == ' ' ? '\0' : command[3];
     prefix[4] = '\0';
     
     switch (command[0])
     {
     case 'G':
          execute_GCode(prefix,command);
          break;
     case 'M':
          execute_MCode(prefix,command);
          break;
     default:
          if (!strcmp(prefix, GetAllInformation))
          {
               send_all_information();
          }
          else if (!strcmp(prefix, GetBaseInformation))
          {
               send_base_inforamtion();
          }
          else if(!strcmp(prefix,GetADCValue)){
               UART_send_command(EndcommandByte,"ADC nozzle:%d",ADC_read(TermisterNozzle));
               UART_send_command(EndcommandByte,"ADC Bed:%d",ADC_read(TermisterBed));
               UART_send_command(EndcommandByte,"Temp nozzle:%d",convert_ADC_to_bed_temp(ADC_read(TermisterNozzle)));
               UART_send_command(EndcommandByte,"Temp Bed:%d",convert_ADC_to_bed_temp(ADC_read(TermisterBed)));
          }
          else
          {
               UART_send_command(EndcommandByte, I_DidntDefCommand,command);
          }
          break;
     }
}

void out_of_range(float X,float Y, float Z,float E){
    if((iPrinter->CurrentPosition.X + X > SIZE_X_MM ||
        iPrinter->CurrentPosition.Y + Y > SIZE_Y_MM ||
        iPrinter->CurrentPosition.Z + Z > SIZE_Z_MM) 
        || 
        (iPrinter->CurrentPosition.X + X < 0 ||
         iPrinter->CurrentPosition.Y + Y < 0||
         iPrinter->CurrentPosition.Z + Z < 0))
        {
          error(OutOfRange_Error);
     }
}

ISR (TIMER0_COMP_vect){
     PID_CALIB_TIME++;
     StepsSleepTimeout++;
     HomePositionTimeout++;
     CheckTempTimeout++;
     UATRTimeOut++;
     ProgramPWM+=20;
     float ticksInSecond = (1 / (((float)OCR0 * (float)1024) / F_CPU));
     if(!(iPrinter->Flags & (1 << FlagIMove)) && 
          (StepsSleepTimeout / ticksInSecond) >= STEPS_TIMEOUT_S){
          disable_steps();
          StepsSleepTimeout = 0;
     }
     if(iPrinter->Flags & (1 << FlagGoHome) &&
     HomePositionTimeout / ticksInSecond >= HOME_POSITION_TIMEOUT_S){
         error(HomePositionTimeOutError); 
     }
     if((CheckTempTimeout / ticksInSecond) >= 1.0){
          UpdateTemps();
          CheckTempTimeout = 0;
          PIDR_calculate_new_value(iPrinter->NozzlePID,iPrinter->tempNozzle,((float)CheckTempTimeout/ticksInSecond));
          PIDR_calculate_new_value(iPrinter->BedPID,iPrinter->tempBed,((float)CheckTempTimeout/ticksInSecond));
          
     }
     if(UATRTimeOut/ticksInSecond >= UART_Timeout_S){
          iPrinter->Flags |= (1 << FlagUARTTimeOut);
          UATRTimeOut=0;
     }
     if(PID_CALIB_TIME/ticksInSecond >= PID_CALIB_TIME_S){
          iPrinter->Flags |= (1 << FlagColibrationPID);
     }

     if(ProgramPWM < iPrinter->fan1){
          PWM_PORT |= (1 << FAN1_CONTROL_PIN); 
     }else{
          PWM_PORT &= ~(1 << FAN1_CONTROL_PIN);
     }
     if(ProgramPWM < iPrinter->fan2){
          PWM_PORT |=(1 << FAN2_CONTROL_PIN);
     }else{
          PWM_PORT &= ~(1 << FAN2_CONTROL_PIN);
     }
    
}

ISR (TIMER1_COMPA_vect){
     //МБ чтот тут будет, но врятли.
}

ISR (TIMER2_COMP_vect){
     TCNT1A_X++;
     TCNT1B_Y++;
     TCNT1C_Z++;
     TCNT1D_E++;

     if((TCNT1B_Y >= OCR1AB_Y) && OCR1AB_Y){
          TCNT1B_Y = 0;
          handle_Y();
     } 

     if((TCNT1A_X >= OCR1AA_X) && OCR1AA_X){
          TCNT1A_X=0;
          handle_X();
     }
   
     if((TCNT1C_Z >= OCR1AC_Z) && OCR1AC_Z){
          TCNT1C_Z=0;
           handle_Z();          
     }
     if((TCNT1D_E >= OCR1AD_E) && OCR1AD_E){
          TCNT1D_E = 0;
           handle_E();
     } 
}

void UpdateTemps(){
     // iPrinter->tempBed    = convert_ADC_to_bed_temp(ADC_read(TermisterBed));
     // iPrinter->tempNozzle = convert_ADC_to_nozzle_temp(ADC_read(TermisterNozzle));
     iPrinter->tempBed = (int)((float)iPrinter->tempBed * 0.8 + (float)(convert_ADC_to_bed_temp(ADC_read(TermisterBed)) * 0.2));
     iPrinter->tempNozzle = (int)((float)iPrinter->tempNozzle * 0.8 + (float)(convert_ADC_to_nozzle_temp(ADC_read(TermisterNozzle)) * 0.2));

}

void stop_axes_timer(){
     TIMSK = TIMSK  & ~(1 << OCIE1A);
     TCNT1A_X = 0;
     TCNT1B_Y = 0;
     TCNT1C_Z = 0;
     TCNT1D_E = 0;
     
     X_Timer_Register = 0;
     Y_Timer_Register = 0;
     Z_Timer_Register = 0;
     E_Timer_Register = 0;
}

void start_axes_timer(){
   TIMSK |= (1 << OCIE1A);
}


void handle_Y(){ 
      if(iPrinter->Steps.nowYsteps ==0){
          return;
     }
     iPrinter->Steps.nowYsteps--;
     // iPrinter->Flags &= ~(1 << FlagYstep);
     AXES_PORT ^= (1 << Y_STEP_PORT);
}

void handle_E(){  
     if(iPrinter->Steps.nowEsteps ==0){
          return;
     }
     iPrinter->Steps.nowEsteps--;
     // iPrinter->Flags &= ~(1 << FlagEstep);
     AXES_PORT ^= (1 << E_STEP_PORT);
}

void handle_Z(){  
     if(iPrinter->Steps.nowZsteps ==0){
          return;
     }
     iPrinter->Steps.nowZsteps--;
     // iPrinter->Flags &= ~(1 << FlagZstep);
     AXES_PORT ^= (1 << Z_STEP_PORT);
}

void handle_X(){
     if(iPrinter->Steps.nowXsteps ==0){
          return;
     }
     iPrinter->Steps.nowXsteps--;
     // iPrinter->Flags &= ~(1 << FlagXstep);
     AXES_PORT ^= (1 << X_STEP_PORT);
}

unsigned int get_delay_timer(float speed, int speedAt){
     if (speed < 0){
          speed = -speed;
     }
     // UART_printf("Speed:%f !!!!!  speedAt:%d !!!!! sesult:%d",speed,speedAt, round((speedAt / speed)));
     return round((speedAt / speed));
}

void set_dir_port_state(uint8_t XDir, uint8_t YDir, uint8_t ZDir, uint8_t EDir)
{
     // Reset dirs
     AXES_PORT = AXES_PORT & ((1 << X_STEP_PORT) |
                              (1 << Y_STEP_PORT) |
                              (1 << Z_STEP_PORT) |
                              (1 << E_STEP_PORT));
#if INVERT_X == 1
     if (XDir != 0)
     AXES_PORT |= (1 << X_DIR_PORT);
#else
     if ((XDir == 0))
     AXES_PORT |= (1 << X_DIR_PORT);
#endif

#if INVERT_Y == 1
     if (YDir != 0)
     AXES_PORT |= (1 << Y_DIR_PORT);
#else
     if (YDir == 0)
     AXES_PORT |= (1 << Y_DIR_PORT);
#endif

#if INVERT_Z == 1
     if (ZDir != 0)
     AXES_PORT |= (1 << Z_DIR_PORT);
#else
     if (ZDir == 0)
     AXES_PORT |= (1 << Z_DIR_PORT);
#endif

#if INVERT_E == 1
     if (EDir != 0)
     AXES_PORT |= (1 << E_DIR_PORT);
#else
     if (EDir == 0)
     AXES_PORT |= (1 << E_DIR_PORT);
#endif
}

void bring_steps_to_format()
{
     if (iPrinter->Steps.nowXsteps < 0)
          iPrinter->Steps.nowXsteps = -iPrinter->Steps.nowXsteps;
     if (iPrinter->Steps.nowYsteps < 0)
          iPrinter->Steps.nowYsteps = -iPrinter->Steps.nowYsteps;
     if (iPrinter->Steps.nowEsteps < 0)
          iPrinter->Steps.nowEsteps = -iPrinter->Steps.nowEsteps;
     if (iPrinter->Steps.nowZsteps < 0)
          iPrinter->Steps.nowZsteps = -iPrinter->Steps.nowZsteps;
}

void home_position()
{ // todo
     // reset steps
     iPrinter->Steps.nowXsteps = 0;
     iPrinter->Steps.nowYsteps = 0;
     iPrinter->Steps.nowZsteps = 0;
     iPrinter->Steps.nowEsteps = 0;
     //TIMSK |= (1 << OCIE0) | (1 << OCIE1A); // turn on timers
     enable_steps();
     start_axes_timer();
     // X
     X_Timer_Register = get_delay_timer(StandartSpeed, iPrinter->Steps.speedAtX);
     set_dir_port_state(0, 0, 0, 1);
     HomePositionTimeout = 0;
     iPrinter->Flags |= (1 << FlagIMove) | (1 << FlagGoHome);
#if INVERT_X_EDNSTOPS == 1
     while ((EndStopsAndAnableStepsPIN & (1 << EndstopX)) == 0)
     {
          iPrinter->Steps.nowXsteps = 1;
          Await();
     }
#else
     while ((EndStopsAndAnableStepsPIN & (1 << EndstopX)) != 0)
     {
          iPrinter->Steps.nowXsteps = 1;
          Await();
     }
#endif
     // Y
     HomePositionTimeout = 0;
     Y_Timer_Register = get_delay_timer(StandartSpeed, iPrinter->Steps.speedAtY);
#if INVERT_Y_EDNSTOPS == 1
     while ((EndStopsAndAnableStepsPIN & (1 << EndstopY)) == 0)
     {
          iPrinter->Steps.nowYsteps = 1;
          Await();
     }
#else
     while ((EndStopsAndAnableStepsPIN & (1 << EndstopY)) != 0)
     {
          iPrinter->Steps.nowYsteps = 1;
          Await();
     }
#endif
     // Z
     HomePositionTimeout = 0;
     Z_Timer_Register = get_delay_timer(StandartSpeed, iPrinter->Steps.speedAtZ);
#if INVERT_Z_EDNSTOPS == 1
     while ((EndStopsAndAnableStepsPIN & (1 << EndstopZ)) == 0)
     {
          iPrinter->Steps.nowZsteps = 1;
          Await();
     }
#else
     while ((EndStopsAndAnableStepsPIN & (1 << EndstopZ)) != 0)
     {
          iPrinter->Steps.nowZsteps = 1;
          Await();
     }
#endif
     iPrinter->Flags &= ~((1 << FlagGoHome) | (1 << FlagIMove));
     stop_axes_timer();

     iPrinter->CurrentPosition.X = 0.0;
     iPrinter->CurrentPosition.Z = 0.0;
     iPrinter->CurrentPosition.Y = 0.0;
}

void move (float X, float Y, float Z, float E, int speedMMS){
     iPrinter->Steps.nowXsteps = float_to_step(X,X_STEPS_MM);
     iPrinter->Steps.nowYsteps = float_to_step(Y,Y_STEPS_MM);
     iPrinter->Steps.nowZsteps = float_to_step(Z,Z_STEPS_MM);
     iPrinter->Steps.nowEsteps = float_to_step(E,E_STEPS_MM);

     //The resulting vector is calculated using the Pythagorean theorem from the previous vectors.
     float resVecXYZE  = sqrtf(((sque(X) + sque(Y)) + sque(Z)) + sque(E));
     if(resVecXYZE < 0.0001f){
          UART_send_command(EndcommandByte,NullStepsValueError);
     }
     if(speedMMS > MaxSpeedMMS){
          speedMMS = MaxSpeedMMS;
     }
     Y_Timer_Register = get_delay_timer((Y * speedMMS)/resVecXYZE, iPrinter->Steps.speedAtY);
     X_Timer_Register = get_delay_timer((X * speedMMS)/resVecXYZE, iPrinter->Steps.speedAtX);
     Z_Timer_Register = get_delay_timer((Z * speedMMS)/resVecXYZE, iPrinter->Steps.speedAtZ);
     E_Timer_Register = get_delay_timer((E * speedMMS)/resVecXYZE, iPrinter->Steps.speedAtE);
     if(stepTimersNull()){
          UART_send_command(EndcommandByte,NullStepsValueError);
          stop_axes_timer();
          return;
     }
     enable_steps(); 
     set_dir_port_state(X>0,Y>0,Z>0,E>0);

     bring_steps_to_format();
     start_axes_timer();
     iPrinter->Flags |= (1 << FlagIMove);
     while (iPrinter->Steps.nowXsteps != 0 
     ||     iPrinter->Steps.nowYsteps != 0
     ||     iPrinter->Steps.nowEsteps != 0
     ||     iPrinter->Steps.nowZsteps != 0)
     {
          Await();
     }                                     
     stop_axes_timer();
     iPrinter->Flags &= ~(1 << FlagIMove);

     iPrinter->CurrentPosition.X += X;
     iPrinter->CurrentPosition.Y += Y;
     iPrinter->CurrentPosition.Z += Z;
     iPrinter->CurrentPosition.E += E;
}

uint8_t stepTimersNull(){
     if (iPrinter->Steps.nowXsteps && X_Timer_Register == 0){
          return 1;
     }
     if (iPrinter->Steps.nowYsteps && Y_Timer_Register == 0){
          return 1;
     }
     if (iPrinter->Steps.nowZsteps && Z_Timer_Register == 0){
          return 1; 
     }
     if (iPrinter->Steps.nowEsteps && E_Timer_Register == 0){
          return 1;
     }
     return 0;
}

/////////////////////////////Gcode and commands ++

void send_base_inforamtion(){
     char data[] = PrinterName;
     UART_send_command(EndcommandByte,M_Name,PrinterName);
     UART_send_command(EndcommandByte,M_Version,S_Version);
     UART_send_command(EndcommandByte,M_Type,PrinterType);

     //SIZE
     UART_send_command(EndcommandByte,M_Width,SIZE_X_MM);
     UART_send_command(EndcommandByte,M_Height,SIZE_Y_MM);
     UART_send_command(EndcommandByte,M_Length,SIZE_Z_MM);

     UART_send_command(EndcommandByte,M_BuffercommandSize,8);
     UART_send_command(EndcommandByte,M_MaxBufferSize,8);
}

void send_all_information(){
     //PrinterData
     // UART_send_command(EndcommandByte,M_Name,PrinterName);
     // UART_send_command(EndcommandByte,M_Version,S_Version);
     // UART_send_command(EndcommandByte,M_Type, PrinterType);
     
     //XYZ
     UART_send_command(EndcommandByte,M_PositionX,iPrinter->CurrentPosition.X);
     UART_send_command(EndcommandByte,M_PositionY,iPrinter->CurrentPosition.Y);
     UART_send_command(EndcommandByte,M_PositionZ,iPrinter->CurrentPosition.Z);
     //Temp
     UART_send_command(EndcommandByte,M_TemperatureBed,iPrinter->tempBed);
     UART_send_command(EndcommandByte,M_TemperatureNozzle,iPrinter->tempNozzle);

}
// void send_temperatures(){
//      UART_send_command(EndcommandByte,MTemperatureBed,iPrinter->tempBed);
//      UART_send_command(EndcommandByte,MTemperatureNozzle,iPrinter->tempNozzle);
// }

inline void execute_GCode(const char* prefix, const char* command){
     if (!strcmp(prefix,G0)){
          command_G0(command);
     }else if(!strcmp(prefix,G1)){
          command_G1(command);   
     }else if(!strcmp(prefix,G4)){
          command_G4(command);
     }else if(!strcmp(prefix,G10)){
          Command_G10(command); 
     }else if(!strcmp(prefix,G11)){
          Command_G11(command);
     }else if(!strcmp(prefix,G28)){
          home_position();
     }else if(!strcmp(prefix,G90)){
          Command_G90();
     }else if(!strcmp(prefix,G91)){
          Command_G91();
     }else if(!strcmp(prefix,G92)){
          Command_G92(command);
     }else{
          UART_send_command(EndcommandByte,It_UnsuportedCommand,command);
     }
}

inline void execute_MCode(const char* prefix, const char* command){
     if (!strcmp(prefix,HeatBed)){
          heat_bed_command(command,0);
     }else if (!strcmp(prefix,HeatBedAndwait)){
          heat_bed_command(command,1);
     }else if(!strcmp(prefix,GetTemps)){
          UART_send_command(EndcommandByte,M_TemperatureBed,iPrinter->tempBed);
          UART_send_command(EndcommandByte,M_TemperatureNozzle,iPrinter->tempNozzle);
     }else if (!strcmp(prefix, HeatNozzle)){
          heat_nozzle_command(command,0);
     }else if (!strcmp(prefix, HeatNozzleAndWait)) {
          heat_nozzle_command(command,1);
     } else if(!strcmp(prefix,StopHeating)){
          set_temp_bed(0);
          set_temp_nozzle(0);
     }else if(!strcmp(prefix,STOP)){
          stop_axes_timer();
     }else if(!strcmp(prefix,GetPosition)){
          UART_send_command(EndcommandByte,M_PositionX,iPrinter->CurrentPosition.X);
          UART_send_command(EndcommandByte,M_PositionY,iPrinter->CurrentPosition.Y);
          UART_send_command(EndcommandByte,M_PositionZ,iPrinter->CurrentPosition.Z);
     }else if(!strcmp(prefix,GetVersion)){
          UART_send_command(EndcommandByte,M_Version,S_Version);
     }else if(!strcmp(prefix,M82)){
          Command_M82();
     }else if(!strcmp(prefix,M83)){
          Command_M83();
     }else if(!strcmp(prefix,EnableStepscommand)){
          enable_steps();
     }else if(!strcmp(prefix,DisableStepscommand)){
          disable_steps();
     }else if(!strcmp(prefix,TurnOnFan)){
          set_fan_value(command);
     }else if(!strcmp(prefix,TurnOfFan)){
          diasble_fan(command);
     }else if(!strcmp(prefix,M303)){
          CalibrationPIDs();
     }else{
          UART_send_command(EndcommandByte,It_UnsuportedCommand,command);
     }

}

float get_extruder_move(float nowMove){
     if(iPrinter->Flags & (1 << FlagExtruderIsAbsalute)){
          return nowMove - iPrinter->CurrentPosition.E; 
     }else{
          return nowMove;
     }
}

inline void heat_bed_command(const char* command,uint8_t wait){
     float tempBed = 0;
     while(*command != '\0'){
          if(*command == 'S' || *command == 's'){
               tempBed = parse_GCode_from_string(command + 1);
               break;
          }
          command++;
     }
     PIDR_set_need_value(iPrinter->BedPID, tempBed);
     while((wait) && (iPrinter->tempBed < tempBed)){
          Await();
     }
     
}

inline void heat_nozzle_command(const char* command,uint16_t wait){ 
     float NozzleTemp = 0;
     while(*command != '\0'){
          if(*command == 'S' || *command == 's'){
               NozzleTemp = parse_GCode_from_string(command + 1);
               break;
          }
          command++;
     }
     PIDR_set_need_value(iPrinter->NozzlePID,NozzleTemp);
     while(wait && iPrinter->tempNozzle < NozzleTemp){
          Await();
     } 

      // int NozzleTemp = 0;
     // char** coms = strSplit(command,' ');
     // for (int i = 0; coms[i] != NULL; i++){
     //     if(strcasestr(coms[i], "S")){
     //      NozzleTemp = atoi(coms[i]+1);
     //     }
     // }
     // set_temp_nozzle(NozzleTemp);
     // deleteSplit(coms);
     // while(wait && iPrinter->NeedTempNozzle != iPrinter->tempNozzle){
     //      Await();
     // }
}

void Await(){
     if(iPrinter->Flags & (1 << FlagUARTTimeOut) != 0){
         //UART_send_command(EndcommandByte,CommandImHere);
         send_all_information(); 
         iPrinter->Flags&=~(1 << FlagUARTTimeOut);
          UATRTimeOut = 0;
     }
     // if(iPrinter->Flags & (1 << FlagUpdateTemps)){
     //      UpdateTemps();
     //      iPrinter->Flags &= ~(1 <<FlagUpdateTemps);
     // }
     
     // if(iPrinter->Flags &(1 << FlagXstep)){
     //      handle_X();
     // }
     // if(iPrinter->Flags &(1 << FlagYstep)){
     //      handle_Y();
     // }
     // if(iPrinter->Flags &(1 << FlagZstep)){
     //      handle_Z();
     // }
     // if(iPrinter->Flags &(1 << FlagEstep)){
     //      handle_E();
     // }
}

void move_to(float X,float Y, float Z, float E,int speedMMS){
     float XVec = X - iPrinter->CurrentPosition.X; 
     float YVec = Y - iPrinter->CurrentPosition.Y; 
     float ZVec = Z - iPrinter->CurrentPosition.Z; 
     move(XVec,YVec, ZVec,E,speedMMS);
}

inline void set_temp_bed(uint8_t temp){
     iPrinter->BedPID->needValue = temp;
}

inline void set_temp_nozzle(int temp){
     iPrinter->NozzlePID->needValue = temp;
}

/////////////////////////////Gcode and commands --

void error(char* errorMsg){
    
     UART_send_command(EndcommandByte,errorMsg);
     stop_print();
     blickLight(255,0,0);
     while(1){
          
     }
}

inline void blickLight(uint8_t R,uint8_t G,uint8_t B){
     set_light(R,G,B);
     _delay_ms(300);
     set_light(0,0,0);
     _delay_ms(300);
     set_light(R,G,B);
     _delay_ms(300);
     set_light(0,0,0);
     _delay_ms(300);
     set_light(R,G,B);
     _delay_ms(300);
     set_light(0,0,0);
     _delay_ms(300);
}

inline void clear_RX(){
     memset((void*)RX_Buffer,0,RX_Buffer_SIZE);
     RX_Buffer[0] = '\0';
}

void stop_print(){
     stop_axes_timer();
     GICR &= ~(1 << INT1);
     disable_steps();
}

