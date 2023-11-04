#include "time_meas.h"

#include "main.h"
#include "cmsis_gcc.h"
static const TIM_HandleTypeDef* htimTimeMeas = &htim5;

/* Shared variables for time measurement */
static volatile uint32_t shStartTime __attribute__((section(".shared"))); 
static volatile uint32_t shEndTime __attribute__((section(".shared")));
static volatile uint32_t shOffset __attribute__((section(".shared")));

void time_startTime(void) {
    shStartTime = __HAL_TIM_GET_COUNTER(htimTimeMeas);
    __COMPILER_BARRIER();
}

void time_endTime(void) {
    __COMPILER_BARRIER();
    shEndTime = __HAL_TIM_GET_COUNTER(htimTimeMeas);
}

void time_setSharedOffset(void) {
    shOffset = time_measureOffset();
}

uint32_t time_getSharedOffset() {
    return shOffset;
}

uint32_t time_measureOffset(void) {
    volatile uint32_t st = __HAL_TIM_GET_COUNTER(htimTimeMeas);
    __COMPILER_BARRIER();
    volatile uint32_t end = __HAL_TIM_GET_COUNTER(htimTimeMeas);
    return end - st;
}

uint32_t time_getRuntime(uint32_t localOffset) {
    //todo: measure runtime offset, and take it as an intut param
    return shEndTime - shStartTime - (localOffset + shOffset)/2; 
}