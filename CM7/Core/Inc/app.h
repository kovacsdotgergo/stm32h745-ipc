#ifndef APP_H
#define APP_H

/* FreeRTOS */
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"
/* ST */ 
#include "main.h" /* gpio pin definitions, hal include, htim5 definition, huart3 */
#include <string.h>
#include <stdio.h>
/* Own files*/
#include "shared_variables.h"
#include "time_meas.h"
#include "uart_state_machine.h"

extern TaskHandle_t core1TaskHandle;

/* Callback handling the exti interrupts */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

/* Main m7 task, handling serial communication and measurement */
void core1MeasurementTask( void *pvParameters );
/** @brief Measures sending one message with the given size
 *  @param[in] dataSize the measured message size
 */
void app_measureCore1Sending(uint32_t dataSize);
void app_measureCore1Recieving(void);

/* Interrupt handler for int triggered from Core2, used for singaling end
    of measurement */
void interruptHandlerIPC_endMeas( void );
/* MessageBuffer functionality, recieving message */
void interruptHandlerIPC_messageBuffer(void);
/* Triggering interrupt used for IPC message buffer communication */
void generateInterruptIPC_messageBuffer( void * xUpdatedMessageBuffer );
/* Triggering interupt used for IPC signaling start of meas */
void generateInterruptIPC_startMeas(void);

void app_initMessageBufferAMP(void);
void app_createMessageBuffers(void);
void app_createTasks(void);
/* Infinite loop in case of error in the main task */
void app_measurementErrorHandler(void);



#endif /* APP_H */