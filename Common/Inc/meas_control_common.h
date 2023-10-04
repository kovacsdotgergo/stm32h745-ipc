#ifndef MEAS_CONTROL_COMMON_H
#define MEAS_CONTROL_COMMON_H

#include <stdint.h>

#define START_MEAS_INT_EXTI_IRQ EXTI3_IRQn
#define START_MEAS_INT_EXTI_LINE EXTI_LINE3
#define START_MEAS_GPIO_PIN GPIO_PIN_3

#define END_MEAS_GPIO_PIN GPIO_PIN_2
#define END_MEAS_INT_EXTI_LINE EXTI_LINE2
#define END_MEAS_INT_EXTI_IRQ EXTI2_IRQn

typedef enum {
    M7_SEND,
    M7_RECIEVE,
} measDirection;

// Functions handling the shared variables
void ctrl_setDataSize(uint32_t dataSize);
uint32_t ctrl_getDataSize(void);

void ctrl_setDirection(measDirection direction);
measDirection ctrl_getDirection(void);

#endif // MEAS_CONTROL_COMMON_H