#include "mc_message_buffer.h"

/* Reimplementation of sbSEND_COMPLETED(), defined as follows in FreeRTOSConfig.h:
   #define sbSEND_COMPLETED( pxStreamBuffer ) vGenerateCore2Interrupt( pxStreamBuffer )

  Called from within xMessageBufferSend().  As this function also calls
  xMessageBufferSend() itself it is necessary to guard against a recursive
  call.  If the message buffer just updated is the message buffer written to
  by this function, then this is a recursive call, and the function can just
  exit without taking further action.
*/
void vGenerateCore2Interrupt( void * xUpdatedMessageBuffer )
{
  MessageBufferHandle_t xUpdatedBuffer = ( MessageBufferHandle_t ) xUpdatedMessageBuffer;
  
  if( xUpdatedBuffer != xControlMessageBuffer )
  {
    /* Use xControlMessageBuffer to pass the handle of the message buffer
    written to by core 1 to the interrupt handler about to be generated in
    core 2. */
    xMessageBufferSend( xControlMessageBuffer, &xUpdatedBuffer, sizeof( xUpdatedBuffer ), mbaDONT_BLOCK );
    
    /* This is where the interrupt would be generated. */
    HAL_EXTI_D1_EventInputConfig(EXTI_LINE0 , EXTI_MODE_IT,  DISABLE);
    HAL_EXTI_D2_EventInputConfig(EXTI_LINE0 , EXTI_MODE_IT,  ENABLE);
    HAL_EXTI_GenerateSWInterrupt(EXTI_LINE0);
  }
}

/* Tasks ----------------------------------------------------------------*/

/* This task will periodically send data to tasks running on Core 2
   via message buffers. */
static void prvCore1Task( void *pvParameters )
{
  BaseType_t x;
  uint32_t ulNextValue = 0;
  const TickType_t xDelay = pdMS_TO_TICKS( 250 );
  char cString[ 15 ];
  
  /* Remove warning about unused parameters. */
  ( void ) pvParameters;
  
  for( ;; )
  {
    /* Create the next string to send.  The value is incremented on each
    loop iteration, and the length of the string changes as the number of
    digits in the value increases. */
    sprintf( cString, "%lu", ( unsigned long ) ulNextValue );
    
    /* Send the value from this Core to the tasks on the Core 2 via the message 
    buffers.  This will result in sbSEND_COMPLETED()
    being executed, which in turn will write the handle of the message
    buffer written to into xControlMessageBuffer then generate an interrupt
    in core 2. */
    for( x = 0; x < mbaNUMBER_OF_CORE_2_TASKS; x++ )
    {
      xMessageBufferSend( xDataMessageBuffers[ x ], 
                         ( void * ) cString,
                         strlen( cString ),
                         mbaDONT_BLOCK );
      
      /* Delay before repeating */
      vTaskDelay( xDelay );
    }

    ulNextValue++;
  }
}

/* 
  Check if the application still running
*/
static BaseType_t xAreMessageBufferAMPTasksStillRunning( void )
{
  static uint32_t ulLastCycleCounters[ mbaNUMBER_OF_CORE_2_TASKS ] = { 0 };
  BaseType_t xDemoStatus = pdPASS;
  BaseType_t x;
  
  /* Called by the check task to determine the health status of the tasks
  implemented in this demo. */
  for( x = 0; x < mbaNUMBER_OF_CORE_2_TASKS; x++ )
  {
    if( ulLastCycleCounters[ x ] == ulCycleCounters[ x ] )
    {
      xDemoStatus = pdFAIL;
    }
    else
    {
      ulLastCycleCounters[ x ] = ulCycleCounters[ x ];
    }
  }
  
  return xDemoStatus;
}

/* Check task function 
   */
static void prvCheckTask( void *pvParameters )
{
  TickType_t xNextWakeTime;
  const TickType_t xCycleFrequency = pdMS_TO_TICKS( 2000UL );
  
  /* Just to remove compiler warning. */
  ( void ) pvParameters;
  
  /* Initialise xNextWakeTime - this only needs to be done once. */
  xNextWakeTime = xTaskGetTickCount();
  
  for( ;; )
  {
    /* Place this task in the blocked state until it is time to run again. */
    vTaskDelayUntil( &xNextWakeTime, xCycleFrequency );
    
    if( xAreMessageBufferAMPTasksStillRunning() != pdPASS )
    {
      /* Application fail */
      HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    }
    else
    {
      /* Application still running */
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
      HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin)
    }
  }
}