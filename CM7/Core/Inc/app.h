#ifndef APP_H
#define APP_H

// FreeRTOS
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"
// ST
#include "main.h" //gpio pin definitions, hal include, htim5 definition, huart3
// Own files
#include "MessageBufferAMP.h"
#include "time_meas.h"
#include "uart_state_machine.h"
#include <string.h>
#include <stdio.h>

extern TaskHandle_t core1TaskHandle;

// Callback handling the exti interrupts 
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

// M7 tasks
void prvCore1Task( void *pvParameters );
void prvCore1InterruptHandler( void );
void vGenerateCore2Interrupt( void * xUpdatedMessageBuffer );

void app_initMessageBufferAMP(void);
void app_createMessageBuffers(void);
void app_createTasks(void);

#endif // APP_H