#include "hw_control_common.h"

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
