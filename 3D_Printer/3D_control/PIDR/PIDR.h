#ifndef PIDR_H
#define PIDR_H

#include <stdlib.h>
#include <avr/io.h>
#define Alpha 0.95f
#define Betta 0.5f
#define ForgettingRate Alpha

typedef struct 
{
    float needValue; 
    volatile uint16_t* controlregister;

    float proportionallyCoef;
    float integralCoef;
    float differinialCoef;

    float integral;
    float differential;
}PIDR;

static inline PIDR* new_PIDR(float P_coef, float I_coef, float D_coef,int8_t* ControlReg){
    PIDR* pid = (PIDR*)malloc(sizeof(PIDR));
    if(pid == NULL){
        return NULL;
    }
    pid->differinialCoef    = D_coef;
    pid->integralCoef       = I_coef;
    pid->proportionallyCoef = P_coef;
    pid->needValue   = 0;
    pid->integral   = 0;
    pid->differential = 0;
    pid->controlregister = ControlReg;
    return pid;
}

static inline void close_PIDR(PIDR* PIDR){
    free(PIDR);
}

void PIDR_calculate_new_value(PIDR* PID, float nowValue,float deltaTime_S);

extern inline void PIDR_set_need_value(PIDR* pid,float needValue);
unsigned int PIDR_get_new_value(PIDR* PID, float nowValue, float deltaTime_S);
#endif