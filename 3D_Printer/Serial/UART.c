#include "UART.h"


void (*call_back_fuck1)(char data) = NULL;

void UART_init(long int speed){
    // DDRD |= (1 << TX);
    //SETUPPORTS
    DDRD &= ~(1 << RX) & ~(1 << TX);
    PORTD |= (1 <<RX) | (1 << TX); // PULLDOWN
    //////////////////////////////////////////////

    //Speed value.
    uint16_t speedValue =  F_CPU / (speed * 16) -1; 
    UBRRL = speedValue;
    UBRRH = speedValue >> 8;
    ///////////////////////////////////////////////

    //Registers setup
    UCSRA;
    UCSRB |= (1 << TXEN) | (1 << RXEN) | (1 << RXCIE); 
    UCSRC |= (1 << URSEL)| (1 << UCSZ1) | (1 << UCSZ0);

    UDR = 0;
    /////////////////////////////////1///////////////
}

void UART_set_call_back_RX(void (*call_back)(char data)){
    call_back_fuck1 = call_back;
}

ISR (USART_RXC_vect){
    char data = UDR;
    if (call_back_fuck1 != NULL){
        call_back_fuck1 (data);
    }
}

void UART_send_byte(char byte){
    while(!(UCSRA& (1 << UDRE)));
    UDR = byte;
}

void UART_send_message(const char* message){
    uint16_t len = strlen(message);
    for(uint16_t i = 0; i < len; i++){
        UART_send_byte(message[i]);
    }
}

void UART_printf(const char* formatstr, ...){
    va_list args;
    va_start(args,formatstr);

    for(const char* i = formatstr; *i!='\0';i++){
        if(*i == '%'){
            i++;
            if(*i == 's'){
                char* c_arg = va_arg(args,char*);
                UART_printf(c_arg);
            }else if(*i == 'd'){
                int i_arg = va_arg(args,int);
                char buffer[8];
                snprintf(buffer,sizeof(buffer),"%d",i_arg);
                UART_printf(buffer);
            }else if(*i =='f'){
                float f_arg =(float) va_arg(args,double);
                char buffer[10];
                float_to_string(f_arg,buffer,sizeof(buffer));
                UART_printf(buffer);
            }
        }
        else{
            UART_send_byte(*i); 
        }
    }
    va_end(args);
}

void UART_send_command(char postfix,const char* formatstr,...){
    va_list args;
    va_start(args,formatstr);

    for(const char* i = formatstr; *i!='\0';i++){
        if(*i == '%'){
            i++;
            if(*i == 's'){
                char* c_arg = va_arg(args,char*);
                UART_printf(c_arg);
            }else if(*i == 'd'){
                int i_arg = va_arg(args,int);
                char buffer[8];
                snprintf(buffer,sizeof(buffer),"%d",i_arg);
                UART_printf(buffer);
            }else if(*i =='f'){
                float f_arg =(float) va_arg(args,double);
                char buffer[10];
                float_to_string(f_arg,buffer,sizeof(buffer));
                UART_printf(buffer);
            }
        }
        else{
            UART_send_byte(*i); 
        }
    }
    va_end(args);
    UART_send_byte(postfix);
}

void UART_println(const char* formatstr, ...){
    va_list args;
    va_start(args,formatstr);

    for(const char* i = formatstr; *i!='\0';i++){
        if(*i == '%'){
            i++;
            if(*i == 's'){
                char* c_arg = va_arg(args,char*);
                UART_printf(c_arg);
            }else if(*i == 'd'){
                float f_arg =(float) va_arg(args,double);
                char buffer[8];
                float_to_string(f_arg,buffer,sizeof(buffer));
                UART_printf(buffer);
            }
        }
        else{
            UART_send_byte(*i);
        }
    }
    va_end(args);
    UART_send_byte('\n');
} 
