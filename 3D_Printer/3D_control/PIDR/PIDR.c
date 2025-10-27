#include "PIDR.h"

void PIDR_calculate_new_value(PIDR* PID, float nowValue, float deltaTime_S){
   
    float nowError = PID->needValue - nowValue;

// Если цель = 0, отключаем всё, и сбрасываем накопления
if (PID->needValue == 0) {
    PID->integral = 0;
    PID->differential = 0;
    *(PID->controlregister) = 0;
    return;
}

// Если ошибка нулевая — не меняем управление, но сохраняем прошлое значение
if (nowError == 0) {
    PID->differential = 0;
    return;
}

// --- Интегральная составляющая (с ограничением)
if (nowError <= 50.0){
    PID->integral += nowError * deltaTime_S;
    if (PID->integral > 1000.0f)
        PID->integral = 1000.0f;
    if (PID->integral < -1000.0f)
        PID->integral = 1000.0f;
}
// --- Дифференциальная составляющая (при нормальном deltaTime)
float D = 0.0f;
if (deltaTime_S > 0.001f) {  // минимум 1 мс
    D = (nowError - PID->differential) / deltaTime_S;
}
PID->differential = nowError;

// --- PID-выход
float output = 
    nowError * PID->proportionallyCoef + 
    PID->integral * PID->integralCoef +
    D * PID->differinialCoef;

// --- Ограничение выходного сигнала
if (output < 0) output = 0;
if (output > 255) output = 255;

*(PID->controlregister) = (uint8_t)(output + 0.5f);  // округление

}

unsigned int PIDR_get_new_value(PIDR* PID, float nowValue, float deltaTime_S){
    float nowError = PID->needValue - nowValue;

    PID->integral = (PID->integral + nowError * (1.0f - Alpha)) * Alpha;

    // Дифференциальная составляющая
    float D = 0.0f;
    if (deltaTime_S > 0){
        D = (nowError - PID->differential) / deltaTime_S;
    }
    PID->differential = nowError;  // обновляем прошлую ошибку

    // ПИД-выход
    float output = nowError * PID->proportionallyCoef +
                   PID->integral * PID->integralCoef +
                   D * PID->differinialCoef;

    int val = (int)output;

    // Ограничение выходного значения от 0 до 255 (например, для ШИМ)
    if (val < 0) val = 0;
    if (val > 1023) val = 1023;

    return (unsigned int)val;
}

inline void PIDR_set_need_value(PIDR* pid,float needValue){
    //Bring 


    if(needValue == 0){
        pid->differential = 0;
        pid->integral = 0;
    }

    pid->needValue = needValue; 
}


