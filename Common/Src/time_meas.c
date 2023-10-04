#include "time_meas.h"

#include "main.h"
#include "cmsis_gcc.h"
static const TIM_HandleTypeDef* htimTimeMeas = &htim5;

/* Shared variables for time measurement */
static volatile uint32_t shStartTime __attribute__((section(".shared"))); 
static volatile uint32_t shEndTime __attribute__((section(".shared"))); 

void time_startTime(void) {
    shStartTime = __HAL_TIM_GET_COUNTER(htimTimeMeas);
}

void time_endTime(void) {
    shEndTime = __HAL_TIM_GET_COUNTER(htimTimeMeas);
}

uint32_t time_getRuntime(void) {
    //todo: measure runtime offset, and take it as an intut param
    return shEndTime - shStartTime - runtimeOffset; 
}