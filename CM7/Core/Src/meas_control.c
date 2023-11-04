#include "meas_control.h"

#include "FreeRTOS.h"
#include "stm32h7xx_hal.h"

void ctrl_initInterrupts(void) {
    /* SW interrupt for end of measurement */
    HAL_NVIC_SetPriority(END_MEAS_INT_EXTI_IRQ, 0xFU, 0U);
    HAL_NVIC_EnableIRQ(END_MEAS_INT_EXTI_IRQ);
    /* SW interrupt for start of measurement */
    HAL_EXTI_EdgeConfig(START_MEAS_INT_EXTI_LINE, EXTI_RISING_EDGE);
}

void interruptHandlerIPC_endMeas( void ){
    /* Signaling to task with notification*/
    BaseType_t xHigherPriorityTaskWoken;
    xSemaphoreGiveFromISR( endMeasSemaphore, &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

    HAL_EXTI_D1_ClearFlag(END_MEAS_INT_EXTI_LINE);
}

void generateInterruptIPC_startMeas(void){
    generateIT_IPC(START_MEAS_INT_EXTI_LINE);
}