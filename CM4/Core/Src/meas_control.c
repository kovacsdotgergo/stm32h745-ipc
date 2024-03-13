#include "meas_control.h"

#include "stm32h7xx_hal.h"
#include "FreeRTOS.h"

static SemaphoreHandle_t g_startMeasSemaphore = NULL;

void ctrl_initInterrupts(void) {
    /* Int config for end of meas */
    HAL_EXTI_EdgeConfig(END_MEAS_INT_EXTI_LINE, EXTI_RISING_EDGE);

    /* SW interrupt to signal start of meas */
    HAL_NVIC_SetPriority(START_MEAS_INT_EXTI_IRQ, 14U, 1U);
    HAL_NVIC_EnableIRQ(START_MEAS_INT_EXTI_IRQ);
}

void ctrl_setStartMeasSemaphore(SemaphoreHandle_t startMeasSemaphore) {
    assert(startMeasSemaphore != NULL);
    g_startMeasSemaphore = startMeasSemaphore;
}

void generateInterruptIPC_endMeas(void) {
    generateIT_IPC(END_MEAS_INT_EXTI_LINE);
}

void interruptHandlerIPC_startMeas(void) {
    /* Signaling to task with notification*/
    BaseType_t xHigherPriorityTaskWoken;
    xSemaphoreGiveFromISR( g_startMeasSemaphore, &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

    HAL_EXTI_D2_ClearFlag(START_MEAS_INT_EXTI_LINE);
}