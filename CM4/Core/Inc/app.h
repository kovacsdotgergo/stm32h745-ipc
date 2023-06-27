#ifndef APP_H
#define APP_H

// FreeRTOS
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"
// ST
#include "main.h" //gpio pin definitions, htim5, hal inc
// Own files
#include "MessageBufferAMP.h"
#include "time_meas.h"
#include <string.h>

void app_measurementErrorHandler(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void prvCore2InterruptHandler( void );
void signalCore1_endMeasurement(void);

void prvCore2Task( void *pvParameters );

void app_initMessageBufferAMP(void);
void app_createTasks(void);

#endif // APP_H