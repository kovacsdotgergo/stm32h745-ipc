#include <app.h>

TaskHandle_t core1TaskHandle;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  prvCore1InterruptHandler();
  HAL_EXTI_D1_ClearFlag(EXTI_LINE2);
}

void prvCore1Task( void *pvParameters ){
 uint8_t ulNextValue = 0;
  const TickType_t uartDelay = pdMS_TO_TICKS( 100 );
  static char sendBuffer[MAX_DATA_SIZE];
  uint8_t uartInputBuffer;
  uint8_t uartOutputBuffer[32];

  uartStateMachine stateMachine = {
    .state = IDLE,
    .stringNumMeas = {0},
    .stringMeasData = {0},
    .stringIndex = 0,
  };
  bool startMeas;
  uint32_t numMeas, dataSize;
  uartStates lastState = IDLE;
  
  /* Remove warning about unused parameters. */
  ( void ) pvParameters;
  
  for( ;; )
  {
    /* Waiting for a start signal */
    HAL_StatusTypeDef receiveSuccess;
    do {
      receiveSuccess = HAL_UART_Receive(&huart3, &uartInputBuffer,
          sizeof(uartInputBuffer), 0);
      
      if(receiveSuccess == HAL_OK){
        /* Step if new char */
        startMeas = uart_stateMachineStep(uartInputBuffer, &stateMachine,
            &numMeas, &dataSize);
        /* Echoing */
        HAL_UART_Transmit(&huart3, &uartInputBuffer, sizeof(uartInputBuffer), 0);
        if(lastState != stateMachine.state){
          HAL_UART_Transmit(&huart3, "\r\n", 2, 100);
        }
        lastState = stateMachine.state;
      }

      vTaskDelay(uartDelay); // vtaskdelay_until could be used
    } while(receiveSuccess != HAL_OK || !startMeas);
    /* Sending data size for error deteciton */
    if(dataSize > MAX_DATA_SIZE){
      dataSize = MAX_DATA_SIZE;
    }
    for(uint32_t i = 0; i < numMeas; ++i){
      /* Sending the data. The size is given over uart */
      for(size_t j = 0; j < dataSize; ++j){
        sendBuffer[j] = ulNextValue;
      }

      sprintf((char*)sendBuffer, "%u", dataSize);
      
      /* Start of measurement and sending the data */
      startTime = __HAL_TIM_GET_COUNTER(&htim5);
      xMessageBufferSend( xDataMessageBuffers, 
                          ( void * ) sendBuffer,
                          dataSize,
                          mbaDONT_BLOCK );
      
      /* Waiting for the signal from the other core */
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      uint32_t runTime = endTime - startTime - runtimeOffset;
      /* Printing measurement */
      memset(uartOutputBuffer, 0, sizeof(uartOutputBuffer));
      sprintf(uartOutputBuffer, "%lu\r\n", runTime);
      HAL_UART_Transmit(&huart3, uartOutputBuffer, strlen(uartOutputBuffer), HAL_MAX_DELAY);
      
      ulNextValue = ulNextValue + 1;
    }
  }
}

void prvCore1InterruptHandler( void ){
  /* Signaling to task with notification*/
  BaseType_t xHigherPriorityTaskWoken;
  vTaskNotifyGiveFromISR(core1TaskHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

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

void app_initMessageBufferAMP(void){
      /* AIEC Common configuration: make CPU1 and CPU2 SWI line0
  sensitive to rising edge : Configured only once */
  HAL_EXTI_EdgeConfig(EXTI_LINE0 , EXTI_RISING_EDGE);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0xFU, 0U);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}

void app_createMessageBuffers(void){
  /* Create control message buffer */
  xControlMessageBuffer = xMessageBufferCreateStatic( mbaCONTROL_MESSAGE_BUFFER_SIZE,ucStorageBuffer_ctr ,&xStreamBufferStruct_ctrl);  
  /* Create data message buffer */
  xDataMessageBuffers = xMessageBufferCreateStatic( mbaTASK_MESSAGE_BUFFER_SIZE, &ucStorageBuffer[0], &xStreamBufferStruct);
  configASSERT( xDataMessageBuffers );
}

void app_createTasks(void){
  // creating the tasks for the M7 core
  const uint8_t mainAMP_TASK_PRIORITY = configMAX_PRIORITIES - 2;
  xTaskCreate(prvCore1Task, "AMPCore1", configMINIMAL_STACK_SIZE, \
      NULL, mainAMP_TASK_PRIORITY, &core1TaskHandle);
  configASSERT( core1TaskHandle );
}
