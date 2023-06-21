#include "app.h"

/* Error handler when the data isn't correct*/
void app_measurementErrorHandler(void){
  while(1){
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

/* Override the callback function to handle interrupts */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  prvCore2InterruptHandler();
  HAL_EXTI_D2_ClearFlag(EXTI_LINE0);
}

/* Handler for the interrupts that are triggered on core 1 but execute on core 2. */
void prvCore2InterruptHandler( void ) // TODO rename to messagebuffer_interrupthandler
{
  MessageBufferHandle_t xUpdatedMessageBuffer;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  /* xControlMessageBuffer contains the handle of the message buffer that
  contains data. */
  if( xMessageBufferReceiveFromISR( xControlMessageBuffer,
                                   &xUpdatedMessageBuffer,
                                   sizeof( xUpdatedMessageBuffer ),
                                   &xHigherPriorityTaskWoken ) == sizeof( xUpdatedMessageBuffer ) )
  {
    /* API function notifying any task waiting for the messagebuffer*/
    xMessageBufferSendCompletedFromISR( xUpdatedMessageBuffer, &xHigherPriorityTaskWoken );
  }
  /* Scheduling with normal FreeRTOS semantics */
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/* Init function */
void app_initMessageBufferAMP(void){
  /* Timer for time measurement */
  MX_USART3_UART_Init();
  htim5.Instance = TIM5;
  /* TODO */
  HAL_EXTI_EdgeConfig(EXTI_LINE2, EXTI_RISING_EDGE);

  /* Interrupt line to handle the interrupt signal from the m7 core*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0xFU, 0U);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  
  /* m7 core initializes the message buffers */
  if (( xControlMessageBuffer == NULL )|( xDataMessageBuffers == NULL ))
  {
    Error_Handler();
  }
}

/* Creating the tasks for the m4 core */
void app_createTasks(void){
    xTaskCreate( prvCore2Task,
              "AMPCore2",
              configMINIMAL_STACK_SIZE,
              NULL,
              tskIDLE_PRIORITY + 1,
              NULL );
}

/* Interrupt for m7 core, signaling end of measurement */
void signalCore1_endMeasurement(void){
  HAL_EXTI_D2_EventInputConfig(EXTI_LINE2 , EXTI_MODE_IT,  DISABLE);
  HAL_EXTI_D1_EventInputConfig(EXTI_LINE2 , EXTI_MODE_IT,  ENABLE);
  HAL_EXTI_GenerateSWInterrupt(EXTI_LINE2);
}

/* m4 core task waiting for measurement and handling it */
void prvCore2Task( void *pvParameters )
{
  uint32_t xReceivedBytes, sizeFromMessage;
  uint8_t ulNextValue = 0;
  static uint8_t receivedBuffer[ MAX_DATA_SIZE ];

  for( ;; )
  {   
    /* Wait to receive the next message from core 1. */
    xReceivedBytes = xMessageBufferReceive( xDataMessageBuffers,
                                            receivedBuffer,
                                            sizeof(receivedBuffer),
                                            portMAX_DELAY );
    endTime = __HAL_TIM_GET_COUNTER(&htim5);

    sscanf((char*)receivedBuffer, "%lu", &sizeFromMessage);
    /* Checking the size and last element of the data */
    if(xReceivedBytes != sizeFromMessage || 
        ((sizeFromMessage > 2) && receivedBuffer[xReceivedBytes - 1] != ulNextValue)){
      app_measurementErrorHandler();
    }
    memset( receivedBuffer, 0x00, xReceivedBytes );
    ulNextValue++;
    signalCore1_endMeasurement();
  }
}
