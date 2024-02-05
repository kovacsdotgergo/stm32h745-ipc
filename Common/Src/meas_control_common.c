#include "meas_control_common.h"

#include "stm32h7xx_hal.h"

static volatile uint32_t shDataSize __attribute__((section(".shared"))) = 1; 
static volatile params_direction shDirection __attribute__((section(".shared"))) = M7_SEND;
static uint32_t g_repeat = 1; // no need to be shared

bool ctrl_setDataSize(uint32_t dataSize, const char** msg) {
    if (dataSize < DATASIZE_LOW_LIMIT) {
        if (msg != NULL) *msg = "Datasize saturated to lower limit\r\n";
        shDataSize = DATASIZE_LOW_LIMIT;
    }
    else if (DATASIZE_UP_LIMIT < dataSize) {
        if (msg != NULL) *msg = "Datasize saturated to upper limit\r\n";
        shDataSize = DATASIZE_UP_LIMIT;
    }
    else {
        shDataSize = dataSize;
    }
    return true;
}

uint32_t ctrl_getDataSize(void) {
    return shDataSize;
}

bool ctrl_setDirection(params_direction direction, const char** msg) {
    (void)msg;
    shDirection = direction;
    return true;
}

params_direction ctrl_getDirection(void) {
    return shDirection;
}

bool ctrl_setRepeat(uint32_t repeat, const char** msg) {
    if (REPETITION_UP_LIMIT < repeat) {
        if (msg != NULL) *msg = "Repetition saturated to upper limit\r\n";
        g_repeat = REPETITION_UP_LIMIT;
    }
    else {
        g_repeat = repeat;
    }
    return true;
}

uint32_t ctrl_getRepeat(void) {
    return g_repeat;
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
