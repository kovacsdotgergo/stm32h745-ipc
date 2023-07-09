#ifndef TIME_MEAS_H
#define TIME_MEAS_H

#include <stdint.h>

/* Maximum size for the messages */
#define MAX_DATA_SIZE (32764)

#define START_MEAS_INT_EXTI_IRQ EXTI3_IRQn
#define START_MEAS_INT_EXTI_LINE EXTI_LINE3
#define START_MEAS_GPIO_PIN GPIO_PIN_3 


typedef enum {
    M7_SEND,
    M7_RECIEVE,
} measDirection;

/* Shared variables used for storing the configurable measurement
    parameters */
/* TODO numemas and datasize not shared for now, remove */
static volatile uint32_t shNumMeas __attribute__((section(".RAM_D3_Z3"))); 
static volatile uint32_t shDataSize __attribute__((section(".RAM_D3_Z3")));
static volatile measDirection shDirection __attribute__((section(".RAM_D3_Z3")));

/** Shared variables for time measurement
 *  Only for gcc compiler */
static volatile uint32_t shStartTime __attribute__((section(".RAM_D3_Z3")));
static volatile uint32_t shEndTime __attribute__((section(".RAM_D3_Z3")));

static const uint32_t runtimeOffset = 24;

#endif /* TIME_MEAS_H */