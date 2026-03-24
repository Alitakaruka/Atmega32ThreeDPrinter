#ifndef commandS_H
#define commandS_H


#define Debug_Flag1          UART_send_byte('1')
#define Debug_Flag2          UART_send_byte('2')
#define Debug_Flag3          UART_send_byte('3')
#define Debug_Flag4          UART_send_byte('4')
#define Debug_Flag5          UART_send_byte('5')
#define Debug_Flag6          UART_send_byte('6')
#define Debug_Flag7          UART_send_byte('7')
#define Debug_Flag8          UART_send_byte('8')
#define EndOfData            "\r\n"
// #define EndcommandByte        ';'
#define UnknownValue         "__"

//////////////////////////////////////////////////To the printer
#define Identification "Identification"
#define StopPrint            "!_"
#define GetAllInformation    "#_"
#define GetBaseInformation   "&_"
#define Check                "*_"
#define GetTemps             "M105"
#define NowTemperatureBed    "B:"
#define NowTemperatureNozzle "N:"
#define ReadyToRead          "R_"
#define BuffercommandSize     "S_"
#define GetADCValue     "(_" 
#define HeatBed "M140"
#define HeatBedAndwait "M190"
#define HeatNozzle "M104"
#define HeatNozzleAndWait "M109"
#define TurnOnFan "M106"
#define TurnOfFan "M107"
#define StopHeating "M108"
#define STOP "M112"
#define GetPosition "M114"
#define GetVersion "M115"

///////////////////////////////////////////////////From the printer
#define M_TemperatureNozzle  "N:%d"
#define M_TemperatureBed     "B:%d"
#define M_PositionX          "X:%f"
#define M_PositionY          "Y:%f"
#define M_PositionZ          "Z:%f"
#define MyBufferLen          "M_Buff_Len:%d"
#define M_Name               "M_Name:%s"
#define M_Type               "M_Type:%d"
// #define M_Version            "V_%s"
#define M_Width              "M_Width:%d"
#define M_Length             "M_Length:%d"
#define M_Height             "M_Height:%d"
#define DEVICE_CHIP_NAME     "Device_chip_name:%s"

#define SwitchTimeout  "Switch_Timeout:"
#define SwitchHasLight  "HasLight:"
#define SwitchRGBLight  "RGBLight:"
#define SYNC            "SYNC"

#define I_DidntDefCommand    "!_%s"
#define It_UnsuportedCommand "@_%s"
#define ACK                    "ok"
#define CommandImHere        "*_"
#define ClearBuffer           "!!!!!"
////////////////////////////////////////////////////GCode
#define G0  "G0" //Fast move 
#define G1  "G1" //move
#define G4  "G4" //sleep
#define G10 "G10"//retract
#define G11 "G11" //unretract
#define G28 "G28" //Home position
#define G90 "G90" //set absolute coord
#define G91 "G91"
#define G92 "G92" //Set position

#define EnableStepscommand  "M17"
#define DisableStepscommand  "M18"

#define M82 "M82"
#define M83 "M83"

#define M140 "M140"
#define M190 "M190"


///////////////////////////////////////////////////// Unsuported
#define G20  "G20"
#define G21  "G21"
#define G29  "G29"

#define M20 "M20"
#define M21 "M21"
#define M22 "M22"
#define M23 "M23"
#define M24 "M24"
#define M25 "M25"
#define M29 "M29"
#define M30 "M30"
#define M32 "M32"
#define M80 "M80"
#define M81 "M81"
#define M92 "M92" //TODO
#define M110 "M110"
#define M119 "M119" //TODO
#define M200 "M200"//todo
#define M201 "M201"
#define M202 "M202"
#define M203 "M203"
#define M204 "M204"
#define M205 "M205"
#define M206 "M206"
#define M207 "M207"
#define M208 "M208"
#define M209 "M209"
#define M218 "M218"
#define M221 "M221" //flow
#define M301 "M301"
#define M303 "M303"
#define M404 "M404"
#define M420 "M420"
#define M500 "M500"
#define M501 "M501"
#define M600 "M600"


/////////////////////////////////////////////////////

/////////////////////////////////////////////////////Errors code
#define Error "Error:%s"
#define Warning "Warning:%s"
#define Information "Information:%s"
#define Success "Success:%s"
// #define MemoryAllocError            "E_0x01"
// #define ParseCommandError           "E_0x02"
// #define UndefinedCommandError       "E_0x03"
// #define OutOfRange_Error            "E_0x04"
// #define BufferOverflowError         "E_0x05"
// #define RXBufferOverfloError        "E_0x06"
// #define EmbededError                "E_0x07"
// #define NullStepsValueError         "E_0x08"
// #define NullVectorError             "E_0x09"
// #define HomePositionTimeOutError    "E_0x0A"
/////////////////////////////////////////////////////

#endif
