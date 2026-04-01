#include "3D_Control.h"

static ThreeD_Printer printer = {};
volatile ThreeD_Printer* iPrinter = &printer;

// ThreeD_Printer volatile *iPrinter = NULL;

volatile uint16_t TCNT1A_X = 0;
volatile uint16_t TCNT1B_Y = 0;
volatile uint16_t TCNT1C_Z = 0;
volatile uint16_t TCNT1D_E = 0;

volatile uint16_t OCR1AA_X = 0;
volatile uint16_t OCR1AB_Y = 0;
volatile uint16_t OCR1AC_Z = 0;
volatile uint16_t OCR1AD_E = 0;

volatile uint16_t StepsSleepTimeout = 0;
volatile uint16_t HomePositionTimeout = 0;
volatile uint16_t CheckTempTimeout = 0;
volatile uint8_t UATRTimeOut = 0;
volatile uint16_t PID_CALIB_TIME = 0;
volatile uint8_t ProgramPWM = 0;

const float ticksInSecond = (1 / (((float)255 * (float)1024) / F_CPU));


void setup_printer()
{
   
     UART_init(BaudRate);
     UART_set_call_back_RX(add_in_buffer);

     iPrinter->NozzlePID = new_PIDR(3.0, 0.4, 2.0, &NOZZLE_REGISTER);
     iPrinter->BedPID = new_PIDR(3.0, 0.3, 2.0, &BED_REGISTER);
     iPrinter->flowrate = 100;
     iPrinter->feedrate = 100;


     eeprom_read_block(&iPrinter->settings,&settings_eeprom,sizeof(Settings));

     //EEPROM
     if(iPrinter->settings.steps_to_mm_X == 0){
          iPrinter->settings.steps_to_mm_X = X_STEPS_MM;
     }else  if(iPrinter->settings.steps_to_mm_Y == 0){
          iPrinter->settings.steps_to_mm_Y = Y_STEPS_MM;
     }else  if(iPrinter->settings.steps_to_mm_Z == 0){
          iPrinter->settings.steps_to_mm_Z = Z_STEPS_MM;
     } else  if(iPrinter->settings.steps_to_mm_E == 0){
          iPrinter->settings.steps_to_mm_E = E_STEPS_MM;
     }

     BaseSettings Set = {}; 
     eeprom_read_block(&Set,&BaseSettings_eeprom,sizeof(BaseSettings));
     if (Set.magic != SETTINGS_MAGIC) {
          Set.magic = SETTINGS_MAGIC;
          strcpy(Set.CustomName, PrinterName);
          eeprom_write_block(&Set, &BaseSettings_eeprom, sizeof(BaseSettings));
     }

     // Ports setup
     AXES_DDR = 255;
     AXES_PORT = 0; // disable all ports
     PWM_PORT = 0;
     PWM_DDR |= (1 << NOZZLE_HEAT_PIN) | (1 << BED_HEAT_PIN) | (1 << FAN1_CONTROL_PIN) | (1 << FAN2_CONTROL_PIN);

     EndStopsAndAnableStepsDDR &= ~((1 << EndstopX) | (1 << EndstopY) | (1 << EndstopZ));
     EndStopsAndAnableStepsDDR |= (1 << StepStatePin);
     LedPort |= (1 << LedPin);
     // Max time interval for timer and max speed
     iPrinter->Steps.speedAtY = (1 / (((float)AXES_TIMER_PRESCALER * STEP_TIMER_UNIT) / F_CPU)) / Y_STEPS_MM;
     iPrinter->Steps.speedAtX = (1 / (((float)AXES_TIMER_PRESCALER * STEP_TIMER_UNIT) / F_CPU)) / X_STEPS_MM;
     iPrinter->Steps.speedAtE = (1 / (((float)AXES_TIMER_PRESCALER * STEP_TIMER_UNIT) / F_CPU)) / E_STEPS_MM;
     iPrinter->Steps.speedAtZ = (1 / (((float)AXES_TIMER_PRESCALER * STEP_TIMER_UNIT) / F_CPU)) / Z_STEPS_MM;
     iPrinter->speed = StandartSpeed;
     
     
     
     ADC_Init();
     PWM_timer_init();               // timer 0
     init_axes_timer();              // timer 1
     init_watch_dog_printer_timer(); // timer2

     stop_axes_timer(); // reset all data in registers
     sei();
     enable_steps();

     iPrinter->fan2 = 255; // radiator fan is enable (M106 && M107 control him)
     // For user
     blickLight(0, 255, 0); //
     //
     set_light(255, 255, 255);
     // UART_send_message("test");
     UART_send_message(EndOfData);
     // log_success("The printer has loaded!\n"
     // "Name:%s\n"
     // "Chip:%s\n"
     // "Width:%d\n"
     // "Length:%d\n"
     // "Height:%d\n"
     // "Max command len:%d",
     // Set.CustomName,ChipName,SIZE_X_MM,SIZE_Y_MM,SIZE_Z_MM,MaxCommandLen);

     // log_information("Free ram:%d",free_memory());
}

void printer_serve()
{
     log_information("Serve");
     while (1)
     {
          if (!Buffio_isEmpty(&(iPrinter->buffio))){
               log_success("Is empty");
               char CurrentCommand[MaxCommandLen] = {};
               if (Buffio_ReadLine(&(iPrinter->buffio), CurrentCommand,
                    sizeof(CurrentCommand), EndOfData)== -1){ 
                           if((iPrinter->Flags & (1 <<FlagDebug)) != 0){
                              log_error("Max command len!");
                }     
                    panic("Max command len error!");
               }
               execute_command(CurrentCommand);
               UART_send_command(EndOfData, ACK);
               UATRTimeOut = 0;
          }else{
               // asm("nop");//Отче наш я же си на небеси. Да светится имя твое да будет царствие твое до будет воля твОя 
               Await();
          }
     }
}

void add_in_buffer(char byte)
{
     if (Buffio_WriteByte(&(iPrinter->buffio), byte) != 1)
     {
          panic("buffer overflow!");
     }
}

void set_light(uint8_t R, uint8_t G, uint8_t B)
{
     struct cRGB led[0];
     led[0].r = R;
     led[0].g = G;
     led[0].b = B;
     for (int i = 0; i < Pixels; i++)
     {
          ws2812_sendarray((uint8_t *)led, 3);
     }
}
void execute_command(char* command)
{
     if((iPrinter->Flags & (1 <<FlagDebug)) != 0){
          log_information("Start executing command:%s",command);     
          log_information("Command len:%d",strlen(command));
     }

     if (strlen(command) == 0){
          if (iPrinter->Flags & (1 << FlagDebug)){
               log_warning("Void command!%s");
          }
          return;
     }
     switch (command[0])
   
     {
     case 'G':
          execute_GCode(command);
          break;
     case 'M':
          execute_MCode(command);
          break;
     
     default:
          if (strcasestr(command, SYNC))
          {
               UART_send_command(EndOfData,MyBufferLen, commandsBufferSize);
               memset(&iPrinter->buffio, 0, sizeof(iPrinter->buffio));
          }
          else if (strcasestr(command, Identification))
          {
               BaseSettings Set = {}; 
               eeprom_read_block(&Set,&BaseSettings_eeprom,sizeof(BaseSettings));
     
               UART_send_command(EndOfData,M_Name,Set.CustomName);
               UART_send_command(EndOfData,M_Type, PrinterType);
               UART_send_command(EndOfData,DEVICE_CHIP_NAME,ChipName);
               // XYZ
               UART_send_command(EndOfData, M_Width, SIZE_X_MM);
               UART_send_command(EndOfData, M_Length, SIZE_Y_MM);
               UART_send_command(EndOfData, M_Height, SIZE_Z_MM);

               UART_send_command(EndOfData,MyKey,iPrinter->settings.UniqueKey);
          }else if(strcasestr(command,"DebugMode")){
               int mode;
               if(sscanf(command,DebugMode,&mode) == 1){
                   if(mode){
                    iPrinter->Flags |= 1 << FlagDebug;
                   }else{
                    iPrinter->Flags &= ~(1 << FlagDebug);
                   }
               }
          }else if(strstr(command,SetKey)){
               char buf[9] ={};
               if(sscanf(command,SetKey,buf) != 0){
                    strcpy(iPrinter->settings.UniqueKey,buf);
                    save_setting();
               }
          }else{
               log_warning("command not defined:%s",command);
          }
          break;
     }
}

void out_of_range(float X, float Y, float Z, float E)
{
     if ((iPrinter->CurrentPosition.X + X > SIZE_X_MM ||
          iPrinter->CurrentPosition.Y + Y > SIZE_Y_MM ||
          iPrinter->CurrentPosition.Z + Z > SIZE_Z_MM) ||
         (iPrinter->CurrentPosition.X + X < 0 ||
          iPrinter->CurrentPosition.Y + Y < 0 ||
          iPrinter->CurrentPosition.Z + Z < 0))
     {
          panic("Out of range!");
     }
}

void UpdateTemps()
{
     // iPrinter->tempBed    = convert_ADC_to_bed_temp(ADC_read(TermisterBed));
     // iPrinter->tempNozzle = convert_ADC_to_nozzle_temp(ADC_read(TermisterNozzle));
     iPrinter->tempBed = (int)((float)iPrinter->tempBed * 0.8 + (float)(convert_ADC_to_bed_temp(ADC_read(TermisterBed)) * 0.2));
     iPrinter->tempNozzle = (int)((float)iPrinter->tempNozzle * 0.8 + (float)(convert_ADC_to_nozzle_temp(ADC_read(TermisterNozzle)) * 0.2));
}


ISR(TIMER0_COMP_vect)
{
     PID_CALIB_TIME++;
     StepsSleepTimeout++;
     HomePositionTimeout++;
     CheckTempTimeout++;
     UATRTimeOut++;
     ProgramPWM += 20;
     if (!(iPrinter->Flags & (1 << FlagIMove)) &&
         (StepsSleepTimeout / ticksInSecond) >= STEPS_TIMEOUT_S)
     {
          disable_steps();
          StepsSleepTimeout = 0;
     }
     if (iPrinter->Flags & (1 << FlagGoHome) &&
         HomePositionTimeout / ticksInSecond >= HOME_POSITION_TIMEOUT_S)
     {
          panic("Home position time out!");
     }
     if ((CheckTempTimeout / ticksInSecond) >= 1.0)
     {
          UpdateTemps();
          CheckTempTimeout = 0;
          PIDR_calculate_new_value(iPrinter->NozzlePID, iPrinter->tempNozzle, ((float)CheckTempTimeout / ticksInSecond));
          PIDR_calculate_new_value(iPrinter->BedPID, iPrinter->tempBed, ((float)CheckTempTimeout / ticksInSecond));
     }
     if (UATRTimeOut / ticksInSecond >= UART_Timeout_S)
     {
          iPrinter->Flags |= (1 << FlagUARTTimeOut);
          UATRTimeOut = 0;
     }
     if (ProgramPWM < iPrinter->fan1)
     {
          PWM_PORT |= (1 << FAN1_CONTROL_PIN);
     }
     else
     {
          PWM_PORT &= ~(1 << FAN1_CONTROL_PIN);
     }
     if (ProgramPWM < iPrinter->fan2)
     {
          PWM_PORT |= (1 << FAN2_CONTROL_PIN);
     }
     else
     {
          PWM_PORT &= ~(1 << FAN2_CONTROL_PIN);
     }
}

ISR(TIMER1_COMPA_vect)
{
     // МБ чтот тут будет, но врятли.
}

ISR(TIMER2_COMP_vect)
{
     TCNT1A_X++;
     TCNT1B_Y++;
     TCNT1C_Z++;
     TCNT1D_E++;

     if ((TCNT1B_Y >= OCR1AB_Y) && OCR1AB_Y)
     {
          TCNT1B_Y = 0;
          handle_Y();
     }

     if ((TCNT1A_X >= OCR1AA_X) && OCR1AA_X)
     {
          TCNT1A_X = 0;
          handle_X();
     }

     if ((TCNT1C_Z >= OCR1AC_Z) && OCR1AC_Z)
     {
          TCNT1C_Z = 0;
          handle_Z();
     }
     if ((TCNT1D_E >= OCR1AD_E) && OCR1AD_E)
     {
          TCNT1D_E = 0;
          handle_E();
     }
}

void stop_axes_timer()
{
     TIMSK = TIMSK & ~(1 << OCIE1A);
     TCNT1A_X = 0;
     TCNT1B_Y = 0;
     TCNT1C_Z = 0;
     TCNT1D_E = 0;

     X_Timer_Register = 0;
     Y_Timer_Register = 0;
     Z_Timer_Register = 0;
     E_Timer_Register = 0;
}

void start_axes_timer()
{
     TIMSK |= (1 << OCIE1A);
}

void handle_Y()
{
     if (iPrinter->Steps.nowYsteps == 0)
     {
          return;
     }
     iPrinter->Steps.nowYsteps--;
     // iPrinter->Flags &= ~(1 << FlagYstep);
     AXES_PORT ^= (1 << Y_STEP_PORT);
}

void handle_E()
{
     if (iPrinter->Steps.nowEsteps == 0)
     {
          return;
     }
     iPrinter->Steps.nowEsteps--;
     // iPrinter->Flags &= ~(1 << FlagEstep);
     AXES_PORT ^= (1 << E_STEP_PORT);
}

void handle_Z()
{
     if (iPrinter->Steps.nowZsteps == 0)
     {
          return;
     }
     iPrinter->Steps.nowZsteps--;
     // iPrinter->Flags &= ~(1 << FlagZstep);
     AXES_PORT ^= (1 << Z_STEP_PORT);
}

void handle_X()
{
     if (iPrinter->Steps.nowXsteps == 0)
     {
          return;
     }
     iPrinter->Steps.nowXsteps--;
     // iPrinter->Flags &= ~(1 << FlagXstep);
     AXES_PORT ^= (1 << X_STEP_PORT);
}

unsigned int get_delay_timer(float speed, int speedAt)
{
     if (speed < 0)
     {
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
     // iPrinter->Steps.nowEsteps = iPrinter->Steps.nowXsteps < 0 ? -iPrinter->Steps.nowXsteps
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
     // TIMSK |= (1 << OCIE0) | (1 << OCIE1A); // turn on timers
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

void move(float X, float Y, float Z, float E, float speedMMS)
{

     iPrinter->Steps.nowXsteps = float_to_step(X, X_STEPS_MM);
     iPrinter->Steps.nowYsteps = float_to_step(Y, Y_STEPS_MM);
     iPrinter->Steps.nowZsteps = float_to_step(Z, Z_STEPS_MM);
     iPrinter->Steps.nowEsteps = float_to_step(E, E_STEPS_MM);
     set_dir_port_state(X > 0.0001f, Y > 0.0001f, Z > 0.0001f, E > 0.0001f);
     bring_steps_to_format();
     // The resulting vector is calculated using the Pythagorean theorem from the previous vectors.
     float resVecXYZE = sqrtf(((sque(X) + sque(Y)) + sque(Z)) + sque(E));
     if (resVecXYZE < 0.0001f)
     {
          log_warning("Null steps value!");
          return;
     }
     if (speedMMS > MaxSpeedMMS)
     {
          speedMMS = MaxSpeedMMS;
     }
     Y_Timer_Register = get_delay_timer((Y * speedMMS) / resVecXYZE, iPrinter->Steps.speedAtY);
     X_Timer_Register = get_delay_timer((X * speedMMS) / resVecXYZE, iPrinter->Steps.speedAtX);
     Z_Timer_Register = get_delay_timer((Z * speedMMS) / resVecXYZE, iPrinter->Steps.speedAtZ);
     E_Timer_Register = get_delay_timer((E * speedMMS) / resVecXYZE, iPrinter->Steps.speedAtE);
    
     if (stepTimersNull())
     {
          log_warning("Null steps value!");
          return;
     }

     
     enable_steps();
     start_axes_timer();
     iPrinter->Flags |= (1 << FlagIMove);
     while (iPrinter->Steps.nowXsteps != 0 || iPrinter->Steps.nowYsteps != 0 || iPrinter->Steps.nowEsteps != 0 || iPrinter->Steps.nowZsteps != 0)
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

uint8_t stepTimersNull()
{
     if (iPrinter->Steps.nowXsteps && (X_Timer_Register == 0))
     {
          return 1;
     }
     if (iPrinter->Steps.nowYsteps && (Y_Timer_Register == 0))
     {
          return 1;
     }
     if (iPrinter->Steps.nowZsteps && (Z_Timer_Register == 0))
     {
          return 1;
     }
     if (iPrinter->Steps.nowEsteps && (E_Timer_Register == 0))
     {
          return 1;
     }
     return 0;
}

/////////////////////////////Gcode and commands ++

// void send_base_inforamtion()
// {
//      char data[] = PrinterName;
//      UART_send_command(EndOfData, M_Name, PrinterName);
//      UART_send_command(EndOfData, M_Type, PrinterType);

//      // SIZE
//      UART_send_command(EndOfData, M_Width, SIZE_X_MM);
//      UART_send_command(EndOfData, M_Height, SIZE_Y_MM);
//      UART_send_command(EndOfData, M_Length, SIZE_Z_MM);
// }

void send_cur_information()
{
     //Position
     UART_send_command(EndOfData, M_PositionX, iPrinter->CurrentPosition.X);
     UART_send_command(EndOfData, M_PositionY, iPrinter->CurrentPosition.Y);
     UART_send_command(EndOfData, M_PositionZ, iPrinter->CurrentPosition.Z);
     // Temp
     UART_send_command(EndOfData, BedTemp, iPrinter->tempBed,iPrinter->BedPID->needValue);
     UART_send_command(EndOfData, ExtruderTemp, iPrinter->tempNozzle,iPrinter->NozzlePID->needValue);
     //Fans
     UART_send_command(EndOfData,FanSpeed,1,iPrinter->fan1);     
     UART_send_command(EndOfData,FanSpeed,2,iPrinter->fan2);     
}


inline void execute_GCode(const char *command)
{
     int gcode;
     if (sscanf(command, "G%d", &gcode) == 1)
     {
     switch (gcode)
     {
        case 0: command_G0(command); break;
        case 1: command_G1(command); break;
        case 4: command_G4(command); break;
        case 10: Command_G10(command); break;
        case 11: Command_G11(command); break;
        case 28: home_position(); break;
        case 90: Command_G90(); break;
        case 91: Command_G91(); break;
        case 92: Command_G92(command); break;

        default:
          log_warning("command not defined:%s",command);
    }
}

// return;
//      if (!strcasestr(command, G0))
//      {
//           command_G0(command);
//      }
//      else if (!strcasestr(command, G1))
//      {
//           UART_send_message("command_G1");
//           command_G1(command);
//      }
//      else if (!strcasestr(command, G4))
//      {
//           UART_send_message("command_G4");
//           command_G4(command);
//      }
//      else if (!strcasestr(command, G10))
//      {
//           UART_send_message("Command_G10");
//           Command_G10(command);
//      }
//      else if (!strcasestr(command, G11))
//      {
//           UART_send_message("Command_G11");
//           Command_G11(command);
//      }
//      else if (!strcasestr(command, G28))
//      {
//           UART_send_message("command_G28");
//           home_position();
//      }
//      else if (!strcasestr(command, G90))
//      {
//           UART_send_message("command_90");
//           Command_G90();
//      }
//      else if (!strcasestr(command, G91))
//      {
//           UART_send_message("command_G91");
//           Command_G91();
//      }
//      else if (!strcasestr(command, G92))
//      {
//           UART_send_message("command_G92");
//           Command_G92(command);
//      }
}

inline void execute_MCode(const char *command)
{
     int MCode;

     if(sscanf(command,"M%d",&MCode)!= 1){
          log_warning("Invalid command format");
          return;
     }

     switch (MCode)
     {
     case HeatBed:  heat_bed_command(command, 0); break;
     case HeatBedAndwait:  heat_bed_command(command, 1); break;
     case GetTemps:   
          UART_send_command(EndOfData, BedTemp, iPrinter->tempBed,iPrinter->BedPID->needValue);
          UART_send_command(EndOfData, ExtruderTemp, iPrinter->tempNozzle,iPrinter->NozzlePID->needValue); 
          break;
     case HeatNozzle:  heat_nozzle_command(command, 0); break;
     case HeatNozzleAndWait:  heat_nozzle_command(command, 1); break;
     case StopHeating:  
          set_temp_bed(0);
          set_temp_nozzle(0);
          break;
     case STOP:   stop_axes_timer(); break;
     case GetPosition:  
          UART_send_command(EndOfData, M_PositionX, iPrinter->CurrentPosition.X);
          UART_send_command(EndOfData, M_PositionY, iPrinter->CurrentPosition.Y);
          UART_send_command(EndOfData, M_PositionZ, iPrinter->CurrentPosition.Z);
          break;
     case M82:  Command_M82(); break;
     case M83:  Command_M83(); break;
     case EnableStepscommand:  enable_steps(); break;
     case DisableStepscommand:  disable_steps(); break;
     case TurnOnFan:   set_fan_value(command); break;
     case TurnOfFan:   diasble_fan(command); break;
     case M220: 
          int Mult;
          if(sscanf(command,"M220 S%d",&(iPrinter->feedrate))){
               iPrinter->feedrate = Mult;
          } ;break;
     case M221:  
          int flow;
          if(sscanf(command,"M221 S%d",&(iPrinter->flowrate)))
               iPrinter->flowrate = flow;
          ;break;

     case M92:
          

     case M486:break;
     case M73: break;
     case M201:break;
     case M204:break;
     case M205:break;
     default:
          log_warning("command not defined:%s",command);
          // log_warning("Current code:%d",MCode);
     }
}

float get_extruder_move(float nowMove)
{
     if (iPrinter->Flags & (1 << FlagExtruderIsAbsalute))
     {
          return nowMove - iPrinter->CurrentPosition.E;
     }
     else
     {
          return nowMove;
     }
}

inline void heat_bed_command(const char *command, uint8_t wait)
{
     float tempBed = 0;
     while (*command != '\0')
     {
          if (*command == 'S' || *command == 's')
          {
               // tempBed = parse_GCode_from_string(command + 1);
               tempBed = strtof(++command,NULL);
               break;
          }
          command++;
     }
     PIDR_set_need_value(iPrinter->BedPID, tempBed);
     while ((wait) && (iPrinter->tempBed < tempBed))
     {
          Await();
     }
}

inline void heat_nozzle_command(const char *command, uint16_t wait)
{
     float NozzleTemp = 0;
     while (*command != '\0')
     {
          if (*command == 'S' || *command == 's')
          {
               NozzleTemp = strtof(++command,NULL);
               break;
          }
          command++;
     }
     PIDR_set_need_value(iPrinter->NozzlePID, NozzleTemp);
     while (wait && iPrinter->tempNozzle < NozzleTemp)
     {
          Await();
     }
}

void Await()
{
    if ((iPrinter->Flags & (1 << FlagUARTTimeOut)) != 0)
     {
          send_cur_information();
          iPrinter->Flags &= ~(1 << FlagUARTTimeOut);
          UATRTimeOut = 0;
     }
     if((iPrinter->Flags & (1 <<FlagDebug)) != 0){

     }
}

void move_to(float X, float Y, float Z, float E, int speedMMS)
{
     float XVec = X - iPrinter->CurrentPosition.X;
     float YVec = Y - iPrinter->CurrentPosition.Y;
     float ZVec = Z - iPrinter->CurrentPosition.Z;
     move(XVec, YVec, ZVec, E, speedMMS);
}

inline void set_temp_bed(uint8_t temp)
{
     iPrinter->BedPID->needValue = temp;
}

inline void set_temp_nozzle(int temp)
{
     iPrinter->NozzlePID->needValue = temp;
}

/////////////////////////////Gcode and commands --


void panic(char *errorMsg)
{
     UART_printf(Error, errorMsg);
     stop_print();
     blickLight(255, 0, 0);
     while (1)
     {
     }
}


void log_error(const char* format,...)
{
     va_list args;
     va_start(args, format);
     UART_printf(Error);
     UART_printf_v(format, args);
     UART_printf(EndOfData);
}

void log_warning(const char* format,...)
{
     va_list args;
     va_start(args, format);
     UART_printf(Warning);
     UART_printf_v(format, args);
     UART_printf(EndOfData);
}

void log_information(const char* format,  ...)
{
     va_list args;
     va_start(args, format);
     UART_printf(Information);
     UART_printf_v(format, args);
     UART_printf(EndOfData);

}

void log_success(const char* format, ...)
{
     va_list args;
     va_start(args, format);
     UART_printf(Success);
     UART_printf_v(format, args);
     UART_printf(EndOfData);
}

inline void blickLight(uint8_t R, uint8_t G, uint8_t B)
{
     set_light(R, G, B);
     _delay_ms(100);
     set_light(0, 0, 0);
     _delay_ms(100);
     set_light(R, G, B);
     _delay_ms(100);
     set_light(0, 0, 0);
     _delay_ms(100);
     set_light(R, G, B);
     _delay_ms(100);
     set_light(0, 0, 0);
     _delay_ms(100);
}

// inline void clear_RX(){
//      memset((void*)RX_Buffer,0,RX_Buffer_SIZE);
//      RX_Buffer[0] = '\0';
// }

void stop_print()
{
     stop_axes_timer();
     GICR &= ~(1 << INT1);
     disable_steps();
}
