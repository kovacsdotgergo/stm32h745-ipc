#include <app.h>

TaskHandle_t core1TaskHandle;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  interruptHandlerIPC_endMeas();
  HAL_EXTI_D1_ClearFlag(EXTI_LINE2);
}

void core1MeasurementTask( void *pvParameters ){
  const TickType_t uartDelay = pdMS_TO_TICKS( 100 );
  uint8_t uartInputBuffer, uartOutputBuffer[32];
  uint32_t numMeas, dataSize;
  uart_measDirection direction;
  uartStateMachine stateMachine = {
    .state = IDLE,
    .stringNumMeas = {0},
    .stringMeasData = {0},
    .stringIndex = 0,
  };
  uartStates lastState = IDLE;
  bool startMeas;

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
            (uint32_t*)&numMeas, (uint32_t*)&dataSize, &direction); /* Cast removing the volatile, the variable doesn't change during execution*/
        /* Echoing */
        HAL_UART_Transmit(&huart3, &uartInputBuffer, sizeof(uartInputBuffer), 0);
        if(lastState != stateMachine.state){
          HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n", 2, 100);
        }
        lastState = stateMachine.state;
      }

      vTaskDelay(uartDelay); // vtaskdelay_until could be used
    } while(receiveSuccess != HAL_OK || !startMeas);
    /* Saturating the data size */
    if(dataSize > MAX_DATA_SIZE){
      dataSize = MAX_DATA_SIZE;
    }
    shDataSize = dataSize; /* Sharing the meas parameters */
    shNumMeas = numMeas;
    shDirection = (direction == SEND) ? M7_SEND : M7_RECIEVE;
    for(uint32_t i = 0; i < numMeas; ++i){
      /* Signaling to the other core*/
      /* TODO */
      /* Waiting for message or sending message */
      switch (shDirection)
      {
      case M7_SEND: /* M7 sends the message */
        /* TODO delay while other core is not ready */
        app_measureCore1Sending((uint32_t)shDataSize); /* Cast to remove volatile, size doesn't change during mesurement */
        break;
      case M7_RECIEVE:
        app_measureCore1Recieving();
        break;
      default:
        app_measurementErrorHandler();
        break;
      }
      /* Printing measurement result */
      uint32_t runTime = shEndTime - shStartTime - runtimeOffset;
      memset(uartOutputBuffer, 0, sizeof(uartOutputBuffer));
      sprintf((char*)uartOutputBuffer, "%lu\r\n", runTime);
      HAL_UART_Transmit(&huart3, uartOutputBuffer, strlen((char*)uartOutputBuffer), HAL_MAX_DELAY);
    }
  }
}

void app_measureCore1Sending(uint32_t dataSize){
  /* Assembling the message*/
  static char sendBuffer[MAX_DATA_SIZE];
  static uint8_t nextValue = 0;
  for(size_t j = 0; j < dataSize; ++j){
    sendBuffer[j] = nextValue;
  }
  sprintf((char*)sendBuffer, "%lu", dataSize);

  /* Start of measurement and sending the data */
  shStartTime = __HAL_TIM_GET_COUNTER(&htim5);
  xMessageBufferSend( xDataMessageBuffers, 
                      ( void * ) sendBuffer,
                      dataSize,
                      mbaDONT_BLOCK );
  
  nextValue = nextValue + 1;
  /* Waiting for the signal from the other core */
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void app_measureCore1Recieving(void){
  /* TODO */
}

void interruptHandlerIPC_endMeas( void ){
  /* Signaling to task with notification*/
  BaseType_t xHigherPriorityTaskWoken;
  vTaskNotifyGiveFromISR(core1TaskHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void generateInterruptIPC_messageBuffer( void * xUpdatedMessageBuffer )
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
  xTaskCreate(core1MeasurementTask, "AMPCore1", configMINIMAL_STACK_SIZE, \
      NULL, mainAMP_TASK_PRIORITY, &core1TaskHandle);
  configASSERT( core1TaskHandle );
}

void app_measurementErrorHandler(void){
  /* TODO: disable interrupt? */
  while(1){} /* Blocking in case of error */
}
