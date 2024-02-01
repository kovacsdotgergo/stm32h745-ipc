#ifndef MEAS_TASK_H
#define MEAS_TASK_H

// FreeRTOS
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "semphr.h"
#include "task.h"
// std
#include <string.h>
#include <assert.h>
#include <stdio.h>
// Own files
#include "ipc_mb_common.h"
#include "time_meas.h"
#include "meas_control.h"
#include "uart_commands.h"

#define MEAS_UART_CHANNEL huart3 // todo use this
#define MEASTASK_STACK_SIZE (2*configMINIMAL_STACK_SIZE)

/* Main m7 task, handling serial communication and measurement */
void meastask_core1MeasurementTask( void *pvParameters );
/** @brief Measures sending one message with the given size
 *  @param[in] dataSize the measured message size
 */
void meastask_measureCore1Sending(uint32_t dataSize);
void meastask_measureCore1Recieving(void);

#endif // MEAS_TASK_H