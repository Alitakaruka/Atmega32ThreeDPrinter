#ifndef UART_H
#define UARTH 
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
//#include "3D_Printer/Configuration.h"

#define RX PORTD0
#define TX PORTD1

#include <avr/interrupt.h>
#include <util/delay.h>
#include "string.h"


extern void (*call_back_fuck1)(char data);

void UART_set_call_back_RX(void (*call_back)(char data));

void UART_init(long int speed);
void UART_send_byte(char byte);
void UART_send_message(const char* message);
void UART_printf(const char* formatstr, ...);
void UART_println(const char* formatstr, ...);
void UART_send_command(char postfix,const char* formatstr,...);
#endif