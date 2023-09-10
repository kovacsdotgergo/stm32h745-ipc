#ifndef TIME_MEAS_H
#define TIME_MEAS_H

#include <stdint.h>

/* Maximum size for the messages */
#define MAX_DATA_SIZE (16376)

#define START_MEAS_INT_EXTI_IRQ EXTI3_IRQn
#define START_MEAS_INT_EXTI_LINE EXTI_LINE3
#define START_MEAS_GPIO_PIN GPIO_PIN_3

#define MB2TO1_INT_EXTI_IRQ EXTI4_IRQn
#define MB2TO1_INT_EXTI_LINE EXTI_LINE4
#define MB2TO1_GPIO_PIN GPIO_PIN_4

typedef enum {
    M7_SEND,
    M7_RECIEVE,
} measDirection;

static const uint32_t runtimeOffset = 24;

#endif /* TIME_MEAS_H */