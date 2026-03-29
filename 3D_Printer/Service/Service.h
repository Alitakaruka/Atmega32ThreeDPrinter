#ifndef SERVICE_H
#define SERVICE_H
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>

extern int __heap_start; // конец статических переменных
extern int __bss_end;    // конец BSS
extern int *__brkval;    // текущий указатель кучи

char* strconc( char* buffer,...);
int free_memory();
char** strSplit(char* string, const char separator);
void deleteSplit(const char** arrStrs);
void ADC_Init();
uint16_t ADC_read(uint8_t channel);

char* get_value_before_separator(char* strSearch, char* buffer,int bufferSize, char separator);
float parse_GCode_from_string(const char* str);
int parse_int_from_string(const char* str);

char* float_to_string(float value, char* buffer, int precision);
float string_to_float(const char* value);
int string_to_int(const char* value);

// static void move_ptr_to_char(char* ptr, char CHAR){
//     while(){

//     }
// }
#endif 