#ifndef commandS_H
#define commandS_H

#define Debug_Flag1 UART_println("1")
#define Debug_Flag2 UART_println("2")
#define Debug_Flag3 UART_println("3")
#define Debug_Flag4 UART_println("4")
#define Debug_Flag5 UART_println("5")
#define Debug_Flag6 UART_println("6")
#define Debug_Flag7 UART_println("7")
#define Debug_Flag8 UART_println("8")
#define EndOfData "\r\n"
// #define EndcommandByte        ';'
// #define UnknownValue         "__"

#define MM_X (1.0 / X_STEPS_MM)
#define MM_Y (1.0 / Y_STEPS_MM)
#define MM_Z (1.0 / Z_STEPS_MM)
#define MM_E (1.0 / E_STEPS_MM)

//////////////////////////////////////////////////To the printer
#define Identification "Identification"
#define SetKey "SetKey:%s"

// #define StopPrint            "!_"
// #define GetAllInformation    "#_"
// #define GetBaseInformation   "&_"
// #define Check                "*_"
// #define ReadyToRead          "R_"
// #define BuffercommandSize     "S_"
// #define GetADCValue     "(_"

#define GetTemps 105
#define HeatBed 140
#define HeatBedAndwait 190
#define HeatNozzle 104
#define HeatNozzleAndWait 109
#define TurnOnFan 106
#define TurnOfFan 107
#define StopHeating 108
#define STOP 112
#define GetPosition 114
#define GetVersion 115

///////////////////////////////////////////////////From the printer
#define M_PositionX "X:%f"
#define M_PositionY "Y:%f"
#define M_PositionZ "Z:%f"

// #define ExtruderTempPref  "N0:"
// #define	BedTempPref       "B0:"
// #define	FanSpeedPref      "FAN:"
#define ExtruderTemp "N0:%d/%d"
#define Extruder2Temp "N1:%d/%d"
#define BedTemp "B0:%d/%d"
#define FanSpeed "FAN:F%d/T%d"

#define MyBufferLen "M_Buff_Len:%d"
#define M_Name "M_Name:%s"
#define M_Type "M_Type:%d"
// #define M_Version            "V_%s"
#define M_Width "M_Width:%d"
#define M_Length "M_Length:%d"
#define M_Height "M_Height:%d"
#define DEVICE_CHIP_NAME "Device_chip_name:%s"

#define SwitchTimeout "Switch_Timeout:"
#define SwitchHasLight "HasLight:"
#define SwitchRGBLight "RGBLight:"
#define SYNC "SYNC"
#define MyKey "UniqueKey:%s"

#define DebugMode "DebugMode:%d"

// #define I_DidntDefCommand    "!_%s"
// #define It_UnsuportedCommand "@_%s"
#define ACK "ok"
// #define CommandImHere        "*_"
// #define ClearBuffer           "!!!!!"
////////////////////////////////////////////////////GCode
#define G0 0   // Fast move
#define G1 1   // move
#define G4 4   // sleep
#define G10 10 // retract
#define G11 11 // unretract
#define G21 21 // unretract
#define G28 28 // Home position
#define G90 90 // set absolute coord
#define G91 91
#define G92 92 // Set position

#define EnableStepscommand 17
#define DisableStepscommand 18

#define M82 82
#define M83 83

#define M92 92

#define M140 140
#define M190 190
#define M221 221 // flow
#define M220 220

///////////////////////////////////////////////////// Unsuported

#define M900 900 // linear advance
#define M486 486
#define M73 73
#define M201 201 // todo
#define M203 203 // todo
#define M204 204
#define M205 205

#define G20 "G20"
#define G29 "G29"

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
#define M110 "M110"
#define M119 "M119" // TODO
#define M200 "M200" // todo
#define M202 "M202"
#define M203 "M203"
#define M206 "M206"
#define M207 "M207"
#define M208 "M208"
#define M209 "M209"
#define M218 "M218"
#define M301 "M301"
#define M303 "M303"
#define M404 "M404"
#define M420 "M420"
#define M500 "M500"
#define M501 "M501"
#define M600 "M600"

/////////////////////////////////////////////////////

/////////////////////////////////////////////////////Errors code
#define Error "Error:"
#define Warning "Warning:"
#define Information "Information:"
#define Success "Success:"
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
