#include "meas_control.h"

#include "stm32h7xx_hal.h"
#include "FreeRTOS.h"

void ctrl_initInterrupts(void) {
    /* Int config for end of meas */
    HAL_EXTI_EdgeConfig(END_MEAS_INT_EXTI_LINE, EXTI_RISING_EDGE);

    /* SW interrupt to signal start of meas */
    HAL_NVIC_SetPriority(START_MEAS_INT_EXTI_IRQ, 0xFU, 1U);
    HAL_NVIC_EnableIRQ(START_MEAS_INT_EXTI_IRQ);
}

void generateInterruptIPC_endMeasurement(void) {
    HAL_EXTI_D2_EventInputConfig(END_MEAS_INT_EXTI_LINE, EXTI_MODE_IT, DISABLE);
    HAL_EXTI_D1_EventInputConfig(END_MEAS_INT_EXTI_LINE, EXTI_MODE_IT, ENABLE);
    HAL_EXTI_GenerateSWInterrupt(END_MEAS_INT_EXTI_LINE);
}

void interruptHandlerIPC_startMeas(void) {
    /* Signaling to task with notification*/
    BaseType_t xHigherPriorityTaskWoken;
    /* TODO bad solution, message buffer uses notification as well*/
    (void)xTaskNotifyFromISR(core2TaskHandle, START_MEAS_BIT, eSetBits,
                        &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

    HAL_EXTI_D2_ClearFlag(START_MEAS_INT_EXTI_LINE);
}