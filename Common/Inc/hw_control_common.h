#ifndef HW_CONTROL_COMMON_H
#define HW_CONTROL_COMMON_H

#include <stdint.h>
#include "stm32h7xx_hal.h"

/** @brief  Generates an interrupt used for signaling during IPC */
void generateIT_IPC(uint32_t EXTI_Line);

#endif // HW_CONTROL_COMMON_H