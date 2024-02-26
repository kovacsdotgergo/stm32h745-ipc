#include "time_meas.h"

volatile uint32_t shStartTime __attribute__((section(".shared"))); 
volatile uint32_t shEndTime __attribute__((section(".shared")));
volatile uint32_t shOffset __attribute__((section(".shared")));

void time_initTimers(void) {
#ifdef CORE_CM4
    // IMPORTANT to be able to read the timer!
    TIMER_HTIM_CHANNEL.Instance = TIMER_TIM_TYPEDEF;
#endif
}

uint32_t time_getSharedOffset() {
    return shOffset;
}

uint32_t time_getRuntime(uint32_t localOffset) {
    return shEndTime - shStartTime - (localOffset + shOffset)/2; 
}