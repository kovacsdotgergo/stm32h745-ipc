#ifndef MC_MESSAGE_BUFFER_H
#define MC_MESSAGE_BUFFER_H

#include "stm32h7xx_hal.h"
#include "MessageBufferAMP.h"
#include "main.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

// SENDER of the message buffer message between cores

void prvCore1Task( void *pvParameters );
void prvCheckTask( void *pvParameters );
BaseType_t xAreMessageBufferAMPTasksStillRunning( void );

#endif //MC_MESSAGE_BUFFER_H