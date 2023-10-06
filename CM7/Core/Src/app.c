#include <app.h>

TaskHandle_t core1TaskHandle;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  switch (GPIO_Pin)
  {
  case END_MEAS_GPIO_PIN: // todo: make a define and hide it somehow
    interruptHandlerIPC_endMeas();
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
    ctrl_setDataSize(dataSize); /* Sharing the meas parameters */
    ctrl_setDirection((direction == SEND) ? M7_SEND : M7_RECIEVE);
    for(uint32_t i = 0; i < numMeas; ++i){
      /* Signaling to the other core*/
      generateInterruptIPC_startMeas();
      /* Waiting for message or sending message */
      switch (direction)
      {
      case M7_SEND: /* M7 sends the message */
        app_measureCore1Sending(dataSize);
        break;
      case M7_RECIEVE:
        app_measureCore1Recieving();
        break;
      default:
        app_measurementErrorHandler();
        break;
      }
      /* Printing measurement result */
      uint32_t runTime = time_getRuntime();
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
  time_startTime();
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
  time_endTime();

  /* Error checking, size and last element */
  sscanf((char*)recieveBuffer, "%lu", &sizeFromMessage);
  if(recievedBytes != sizeFromMessage ||
      (sizeFromMessage > 2 && recieveBuffer[recievedBytes - 1] != nextValue)){
    app_measurementErrorHandler();
    }

  memset(recieveBuffer, 0x00, recievedBytes);
  ++nextValue;
}

void app_initMessageBufferAMP(void){
  /* AIEC Common configuration: make CPU1 and CPU2 SWI line0
  sensitive to rising edge : Configured only once */
  HAL_EXTI_EdgeConfig(MB1TO2_INT_EXTI_LINE, EXTI_RISING_EDGE);
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
