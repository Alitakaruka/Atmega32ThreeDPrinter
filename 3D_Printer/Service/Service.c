#include "Service.h"

#include "math.h"
#include "Serial/UART.h"
char* strconc( char* buffer,...){
    va_list args;
    va_start(args,buffer);
    char* str; 
    while((str = va_arg(args,char*)) != NULL){
        strcat(buffer,str);
    }
    va_end(args);
    return buffer;
}

//WARNING! This function allocate memory! use deleteSplit
char** strSplit(char* string, const char separator) {
    char* buffer =(char*) malloc((sizeof(char) * strlen(string))+1);

    if(buffer ==NULL){
        return NULL;
    }
    int counter = 1;

    for (char* i = string; *i != '\0';i++) {
        if (*i == separator){
            counter++;
        }
    }

    counter++;//Last is NULL
    char** result = (char**)malloc(sizeof(char*) * counter);
    result[counter - 1] = NULL; //Last is NULL
    
    int strCounter = 0;
    int nowIndex = 0;

    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == separator) {
            result[strCounter] = (char*)malloc((sizeof(char) * nowIndex) +1 );
            buffer[nowIndex] = '\0';
            strcpy(result[strCounter], buffer);
            strCounter++;
            nowIndex = 0;
            memset((void*)buffer, 0, nowIndex +1);
            continue;
        }else {
            buffer[nowIndex] = string[i];
        }
        nowIndex++;
    }

    if (nowIndex != 0) {
        result[strCounter] = (char*)malloc((sizeof(char) * nowIndex));
        if(result[strCounter] == NULL){
            for (int i = 0 ; i<strCounter; i++){
                free(result[i]);
            }
            return NULL;
        }
        memset((void*)result[strCounter], 0, ((sizeof(char) * nowIndex) + 1));
        buffer[nowIndex] = '\0';
        strcpy(result[strCounter], buffer);
    }
    free(buffer);
    return result;

}

char* get_value_before_separator(char* strSearch, char* buffer,int bufferSize, char separator){
    int i = 0;
    while(strSearch[i] != '\0' 
        && strSearch[i] != separator 
        && i < bufferSize -1){
        buffer[i] = strSearch[i];
        i++;
    }
    buffer[i] = '\0'; 
    return buffer;
}

float parse_GCode_from_string(const char* str){
    char buffer[20];
    get_value_before_separator(str,buffer,20,' ');
    return string_to_float(buffer);
}

int parse_int_from_string(const char* str){
    char buffer[10];
    get_value_before_separator(str,buffer,10,' ');
    return string_to_int(buffer);
}

void deleteSplit(const char** arrStrs){
    for (char** strs = arrStrs; *strs != NULL; strs++){
        free(*strs);
    }
    free(arrStrs);
}

int freeRam() {
    extern char __stack;
    extern char *__brkval;
    char *sp = (char *)SP;
    char *heap_end = __brkval ? __brkval : &__heap_start;
    return (int)(sp - heap_end);
}

int usedStack(void) {
    uint16_t sp = (SPH << 8) | SPL;
    return RAMEND - sp;
}

void ADC_Init(){
    DDRA = 0;
    PORTA = 0;
    ADCSRA |= (1 << ADEN) | //Turn on ADC
    (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //Preskaler = 128
    ADMUX  |= (1 << REFS0) ; //5VМ embeded
}

uint16_t ADC_read(uint8_t channel){
    if(channel > 7){
        return 0;
    }
   ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);;
    //_delay_us(10);
    ADCSRA |= (1 << ADSC);//Start
    while(!(ADCSRA & (1 << ADIF))){}
    ADCSRA |= (1 << ADIF);   
    return ADC;
}

uint8_t discharge(int value) {
	int counter = 0;
	while (value / 10 != 0) {
		value /= 10;
		counter++;
	}
	counter++;
	return counter;
}

char* float_to_string(float value, char* buffer, int len) {
	char* start = buffer;
	if (value < 0) {
		buffer[0] = '-';
		value = -value;
		len--;
		buffer++;
	}
	int whole = (int)value;
	float Fraction = value - whole;

	snprintf(buffer,len,"%d",whole);
    uint8_t dis = discharge(whole);
	buffer += dis;
	len -= dis;
	*buffer = '.';
	buffer++;
	len--;
	while (len > 1) {
		int nowValue = Fraction * 10;
		*buffer = (int)(Fraction * 10) + '0';
		buffer++;
		len--;
		Fraction *= 10;
		Fraction = Fraction - (int)Fraction;
	}
	*buffer = '\0';

	return start;
}

float string_to_float(const char* value) {
    float result = 0.0f;
    int sign = 0;

    if (*value == '-') {
        sign = 1;
        value++;
    }

    while ((*value != '.') && *value != '\0') {
        result = result * 10.0f + (*value - '0');
        value++;
    }
    float frac = 0.1f;
    while (*value != '\0') {
        if(*value == '.'){
            value++;
            continue;
        }
        result += (*value - '0') * frac;
        frac *= 0.1f;
        value++;
    }
    if (sign) {
        result = -result;
    }
    return result;
}

int string_to_int(const char* value){
    int result = 0;
    uint8_t sign = 1;
    if(*value == '-'){
        sign = -1;
        value++;
    }

    while(*value != '\0'){
        if(*value >= '0' && *value <= '9'){
            result*=10;
            result+= *value - '0';
        }
        value++;    
    }
    return result *  sign;
}