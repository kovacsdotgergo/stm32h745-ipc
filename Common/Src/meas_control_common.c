#include "meas_control_common.h"

#include "stm32h7xx_hal.h"

static volatile uint32_t shDataSize __attribute__((section(".shared"))); 
static volatile measDirection shDirection __attribute__((section(".shared")));

void ctrl_setDataSize(uint32_t dataSize) {
    shDataSize = dataSize;
}

uint32_t ctrl_getDataSize(void) {
    return shDataSize;
}

void ctrl_setDirection(measDirection direction) {
    shDirection = direction;
}

measDirection ctrl_getDirection(void) {
    return shDirection;
}

void generateIT_IPC(uint32_t EXTI_Line) {
#ifdef CORE_CM4
    HAL_EXTI_D2_EventInputConfig(EXTI_Line, EXTI_MODE_IT, DISABLE);
    HAL_EXTI_D1_EventInputConfig(EXTI_Line, EXTI_MODE_IT, ENABLE);
    HAL_EXTI_GenerateSWInterrupt(EXTI_Line);
#elif defined CORE_CM7
    HAL_EXTI_D1_EventInputConfig(EXTI_Line, EXTI_MODE_IT, DISABLE);
    HAL_EXTI_D2_EventInputConfig(EXTI_Line, EXTI_MODE_IT, ENABLE);
    HAL_EXTI_GenerateSWInterrupt(EXTI_Line);
#else
    #error Neither core is defined
#endif // CORE
}
