#include <app.h>

TaskHandle_t core1TaskHandle;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  switch (GPIO_Pin)
  {
  case GPIO_PIN_2:
    interruptHandlerIPC_endMeas();
    HAL_EXTI_D1_ClearFlag(EXTI_LINE2);
    break;
  case MB2TO1_GPIO_PIN:
    interruptHandlerIPC_messageBuffer();
    HAL_EXTI_D1_ClearFlag(MB2TO1_GPIO_PIN);
    break;
  default:
    app_measurementErrorHandler();
    break;
  }
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
      generateInterruptIPC_startMeas();
      /* Waiting for message or sending message */
      switch (shDirection)
      {
      case M7_SEND: /* M7 sends the message */
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
  for(uint32_t j = 0; j < dataSize; ++j){
    sendBuffer[j] = nextValue;
  }
  sprintf((char*)sendBuffer, "%lu", dataSize);
  vTaskDelay(1/portTICK_PERIOD_MS);
  /* Start of measurement and sending the data */
  shStartTime = __HAL_TIM_GET_COUNTER(&htim5);
  xMessageBufferSend( xDataMessageBuffers[MB1TO2_IDX], 
                      ( void * ) sendBuffer,
                      dataSize,
                      mbaDONT_BLOCK );
  
  ++nextValue;
  /* Waiting for the signal from the other core */
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void app_measureCore1Recieving(void){
  static uint8_t nextValue = 0;
  uint32_t recievedBytes, sizeFromMessage;
  static uint8_t recieveBuffer[MAX_DATA_SIZE];

  recievedBytes = xMessageBufferReceive(xDataMessageBuffers[MB2TO1_IDX],
                                        recieveBuffer,
                                        sizeof(recieveBuffer),
                                        portMAX_DELAY);
  shEndTime = __HAL_TIM_GET_COUNTER(&htim5);

  /* Error checking, size and last element */
  sscanf((char*)recieveBuffer, "%lu", &sizeFromMessage);
  if(recievedBytes != sizeFromMessage ||
      (sizeFromMessage > 2 && recieveBuffer[recievedBytes - 1] != nextValue)){
    app_measurementErrorHandler();
    }

  memset(recieveBuffer, 0x00, recievedBytes);
  ++nextValue;
}

/* TODO refactor both it handler and it generation for mb, almost the same
  even the measurement task can be a parametrized single func*/
void interruptHandlerIPC_messageBuffer(void){
  MessageBufferHandle_t xUpdatedMessageBuffer;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  /* xControlMessageBuffer contains the handle of the message buffer that
  contains data. */
  if( xMessageBufferReceiveFromISR( xControlMessageBuffer[MB2TO1_IDX],
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

void interruptHandlerIPC_endMeas( void ){
  /* Signaling to task with notification*/
  BaseType_t xHigherPriorityTaskWoken;
  vTaskNotifyGiveFromISR(core1TaskHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void generateInterruptIPC_messageBuffer( void * xUpdatedMessageBuffer )
{
  MessageBufferHandle_t xUpdatedBuffer = ( MessageBufferHandle_t ) xUpdatedMessageBuffer;
  
  if( xUpdatedBuffer != xControlMessageBuffer[MB1TO2_IDX] )
  {
    /* Use xControlMessageBuffer to pass the handle of the message buffer
    written to by core 1 to the interrupt handler about to be generated in
    core 2. */
    xMessageBufferSend( xControlMessageBuffer[MB1TO2_IDX], &xUpdatedBuffer, sizeof( xUpdatedBuffer ), mbaDONT_BLOCK );
    
    /* This is where the interrupt would be generated. */
    HAL_EXTI_D1_EventInputConfig(EXTI_LINE0, EXTI_MODE_IT, DISABLE);
    HAL_EXTI_D2_EventInputConfig(EXTI_LINE0, EXTI_MODE_IT, ENABLE);
    HAL_EXTI_GenerateSWInterrupt(EXTI_LINE0);
  }
}

void generateInterruptIPC_startMeas(void){
  HAL_EXTI_D1_EventInputConfig(START_MEAS_INT_EXTI_LINE,
                               EXTI_MODE_IT, DISABLE);
  HAL_EXTI_D2_EventInputConfig(START_MEAS_INT_EXTI_LINE,
                               EXTI_MODE_IT, ENABLE);
  HAL_EXTI_GenerateSWInterrupt(START_MEAS_INT_EXTI_LINE);
}

void app_initMessageBufferAMP(void){
  /* AIEC Common configuration: make CPU1 and CPU2 SWI line0
  sensitive to rising edge : Configured only once */
  HAL_EXTI_EdgeConfig(EXTI_LINE0, EXTI_RISING_EDGE);
  /* SW interrupt for start of measurement */
  HAL_EXTI_EdgeConfig(START_MEAS_INT_EXTI_LINE, EXTI_RISING_EDGE);
  /* SW interrupt for end of measurement */
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0xFU, 0U);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  /* SW interrupt for message buffer */
  HAL_NVIC_SetPriority(MB2TO1_INT_EXTI_IRQ, 0xFU, 1U);
  HAL_NVIC_EnableIRQ(MB2TO1_INT_EXTI_IRQ);
}

void app_createMessageBuffers(void){
  /* MBs used for m7->m4 communication */
  /* Create control message buffer */
  xControlMessageBuffer[MB1TO2_IDX] = xMessageBufferCreateStatic(
      mbaCONTROL_MESSAGE_BUFFER_SIZE, ucStorageBuffer_ctrl[MB1TO2_IDX], 
      &xStreamBufferStruct[MB1TO2_IDX*2]);  
  /* Create data message buffer */
  xDataMessageBuffers[MB1TO2_IDX] = xMessageBufferCreateStatic(
      mbaTASK_MESSAGE_BUFFER_SIZE, &ucStorageBuffer[MB1TO2_IDX][0],
      &xStreamBufferStruct[MB1TO2_IDX*2 + 1]);
  configASSERT( xDataMessageBuffers[MB1TO2_IDX] );
  configASSERT( xControlMessageBuffer[MB1TO2_IDX] );
  
  /* MBs used for m4->m7 communication */
  xControlMessageBuffer[MB2TO1_IDX] = xMessageBufferCreateStatic(
      mbaCONTROL_MESSAGE_BUFFER_SIZE, ucStorageBuffer_ctrl[MB2TO1_IDX], 
      &xStreamBufferStruct[MB2TO1_IDX*2]);  
  /* Create data message buffer */
  xDataMessageBuffers[MB2TO1_IDX] = xMessageBufferCreateStatic(
      mbaTASK_MESSAGE_BUFFER_SIZE, &ucStorageBuffer[MB2TO1_IDX][0],
      &xStreamBufferStruct[MB2TO1_IDX*2 + 1]);
  configASSERT( xDataMessageBuffers[MB2TO1_IDX] );
  configASSERT( xControlMessageBuffer[MB2TO1_IDX] );
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
