#ifndef APP_H
#define APP_H

/* FreeRTOS */
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"
/* ST */
#include "main.h" //gpio pin definitions, htim5, hal inc
/* STD */
#include <stdio.h>
#include <string.h>
/* Own files */
#include "ipc_mb_common.h"
#include "time_meas.h"
#include "meas_control.h"
#include "error_handler.h"

#define START_MEAS_BIT 0x01

extern TaskHandle_t core2TaskHandle;

/* Callback handling the exti interrupts */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

/* Main m4 task handling the measurement */
void core2MeasurementTask( void *pvParameters );
/* Receiving a message, error checking and registering the end of meas */
void app_measureCore2Recieving(void);
void app_measureCore2Sending(uint32_t dataSize);

void app_createTasks(void);

#endif /* APP_H */