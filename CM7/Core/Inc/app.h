#ifndef APP_H
#define APP_H

/* FreeRTOS */
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "semphr.h"
#include "task.h"
/* ST */ 
#include "main.h" /* gpio pin definitions, hal include, htim5 definition, huart3 */
#include <string.h>
#include <stdio.h>
/* Own files*/
#include "ipc_mb_common.h"
#include "time_meas.h"
#include "meas_control.h"
#include "uart_state_machine.h"
#include "error_handler.h"

extern TaskHandle_t core1TaskHandle;
extern SemaphoreHandle_t endMeasSemaphore;

/* Callback handling the exti interrupts */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

/* Main m7 task, handling serial communication and measurement */
void core1MeasurementTask( void *pvParameters );
/** @brief Measures sending one message with the given size
 *  @param[in] dataSize the measured message size
 */
void app_measureCore1Sending(uint32_t dataSize);
void app_measureCore1Recieving(void);

/* MessageBuffer functionality, recieving message */
void interruptHandlerIPC_messageBuffer(void);
/* Triggering interrupt used for IPC message buffer communication */
void generateInterruptIPC_messageBuffer( void * xUpdatedMessageBuffer );

void app_createTasks(void);

#endif /* APP_H */