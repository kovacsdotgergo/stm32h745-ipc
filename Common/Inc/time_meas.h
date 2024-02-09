#ifndef TIME_MEAS_H
#define TIME_MEAS_H

#include <stdint.h>
#include "main.h"
#include "cmsis_gcc.h"

#define TIMER_HTIM_CHANNEL htim5
#define TIMER_TIM_TYPEDEF TIM5

/* Shared variables for time measurement, not in c due to inline funs */
extern volatile uint32_t shStartTime;
extern volatile uint32_t shEndTime;
extern volatile uint32_t shOffset;

/** @brief Initializes the used timers */
void time_initTimers(void);

/** @brief Time measurement functions interfacing the shared variables */
static inline void time_startTime(void) {
    __COMPILER_BARRIER();
    shStartTime = __HAL_TIM_GET_COUNTER(&TIMER_HTIM_CHANNEL);
    __COMPILER_BARRIER();
}

static inline void time_endTime(void) {
    __COMPILER_BARRIER();
    shEndTime = __HAL_TIM_GET_COUNTER(&TIMER_HTIM_CHANNEL);
    __COMPILER_BARRIER();
}

/** @returns the offset of time measurement on the caller processor*/
static inline uint32_t time_measureOffset(void) {
    __COMPILER_BARRIER();
    volatile uint32_t st = __HAL_TIM_GET_COUNTER(&TIMER_HTIM_CHANNEL);
    __COMPILER_BARRIER();
    volatile uint32_t end = __HAL_TIM_GET_COUNTER(&TIMER_HTIM_CHANNEL);
    __COMPILER_BARRIER();
    return end - st;
}

/** @brief Measures the offset and sets a shared variable */
static inline void time_setSharedOffset(void) {
    shOffset = time_measureOffset();
}

/** @returns the offset from the shared variable */
uint32_t time_getSharedOffset(void);

/** @returns the runtime compensated with the local offset and the shared offset */
uint32_t time_getRuntime(uint32_t localOffset);

#endif /* TIME_MEAS_H */