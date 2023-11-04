#include <app.h>

TaskHandle_t core1TaskHandle;
SemaphoreHandle_t endMeasSemaphore = NULL;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  switch (GPIO_Pin)
  {
  case END_MEAS_GPIO_PIN:
    interruptHandlerIPC_endMeas();
    break;
  case MB2TO1_GPIO_PIN:
    interruptHandlerIPC_messageBuffer();
    HAL_EXTI_D1_ClearFlag(MB2TO1_GPIO_PIN);
    break;
  default:
    ErrorHandler();
    break;
  }
}

void core1MeasurementTask( void *pvParameters ){
  ( void ) pvParameters;

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

  endMeasSemaphore = xSemaphoreCreateBinary();
  
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
        ErrorHandler();
        break;
      }
      /* Printing measurement result */
      uint32_t localOffset = time_measureOffset();
      // Uncomment to observe the offset on the other core
      // uint32_t m4Offset = time_getSharedOffset();
      uint32_t runTime = time_getRuntime(localOffset);
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
  xSemaphoreTake(endMeasSemaphore, portMAX_DELAY);
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
    ErrorHandler();
    }

  memset(recieveBuffer, 0x00, recievedBytes);
  ++nextValue;
}

void app_createTasks(void){
  // creating the tasks for the M7 core
  const uint8_t mainAMP_TASK_PRIORITY = configMAX_PRIORITIES - 2;
  xTaskCreate(core1MeasurementTask, "AMPCore1", configMINIMAL_STACK_SIZE, \
      NULL, mainAMP_TASK_PRIORITY, &core1TaskHandle);
  configASSERT( core1TaskHandle );
}
