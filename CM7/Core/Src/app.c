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

// todo seperate files for the measurement task ==========================
// todo forward declare the static functions and define them under the meas task

static void printUartMeasTask(const char* msg) {
  // todo
  // uart transmit
  // - with max delay waits in while loop
  // - only returns then with ok
}

static char scanUartMeasTask() {
  // todo
  // same as print
}


/**
 * @brief Echos the input characters, currently blocking
*/
void echoInput(char input) {
  if (input == '\r') {
    HAL_UART_Transmit(&huart3, (unsigned char*)"\r\n", 2, HAL_MAX_DELAY);
  }
  else {
    HAL_UART_Transmit(&huart3, (unsigned char*)&input, 1, HAL_MAX_DELAY);
  }
}

/** 
 * @brief Processes the UART control messages and returns when the
 *  measurement can be started
 * @param[inout] uartParams parameters of the measurement to be performed
*/
static void processUartControl(uart_measParams* uartParams) {
  uart_clearUartMeasParams(uartParams); // to wait for start

  uart_parseStatus uartControlStatus;
  do {
    uart_BufferStatus bufferStatus;
    uart_LineBuffer lineBuffer;
    uart_initLineBuffer(&lineBuffer);

    HAL_UART_Transmit(&huart3, (unsigned char*)uart_getPrompt(), PROMPT_STR_LEN, HAL_MAX_DELAY);
    do {
      // Blocking wait for UART, processing by single characters
      uint8_t uartInput;
      HAL_StatusTypeDef receiveSuccess;
      receiveSuccess = HAL_UART_Receive(&huart3, &uartInput,
          sizeof(uartInput), HAL_MAX_DELAY);
      if (receiveSuccess != HAL_OK) {
        // todo handle error in status
        assert(false);
      }
      
      const char* echo = NULL;
      bufferStatus = uart_addCharToBuffer(uartInput, &lineBuffer, &echo);
      
      if (echo != NULL) {
        HAL_UART_Transmit(&huart3, (const unsigned char*)echo, strlen(echo), HAL_MAX_DELAY);
      }

      if (bufferStatus == BUFFER_OVERFLOW) {
        // todo print overflow message
        HAL_UART_Transmit(&huart3, (const unsigned char*)"\r\nInput buffer overflow\r\n", 24, HAL_MAX_DELAY);
        uart_clearLineBuffer(&lineBuffer);
      }
    } while (bufferStatus != BUFFER_DONE);
      const char* msg = NULL;
      uartControlStatus = uart_parseBuffer(&lineBuffer, uartParams, &msg);
      uart_clearLineBuffer(&lineBuffer);

      switch (uartControlStatus)
      {
      case PARSE_COMMAND_ERR:
        HAL_UART_Transmit(&huart3, (unsigned char*)"Command parsing error\r\n", 24, HAL_MAX_DELAY);
        break;
      case PARSE_ARG_NUM_ERR:
        HAL_UART_Transmit(&huart3, (unsigned char*)"Argument number error while parsing\r\n", 38, HAL_MAX_DELAY);
        break;
      case PARSE_ARG_VAL_ERR:
        HAL_UART_Transmit(&huart3, (unsigned char*)"Argument value error while parsing\r\n", 37, HAL_MAX_DELAY);
        break;
      case PARSE_OK:
        break;
      default:
        assert(false);
        break;
      }

      if (msg != NULL) {
        size_t msglen = strlen(msg);
        HAL_UART_Transmit(&huart3, (unsigned char*)msg, msglen, HAL_MAX_DELAY);
      }

  } while (!uartParams->startMeas);
}

/**
 * @brief Shares the measurement params with the other measurement tasks
 *  and sets up the required parameters e.g. clk frequency
*/
static void prepareMeasParams(uart_measParams params) {
  // set the shared variables
  ctrl_setDataSize(params.dataSize); /* Sharing the meas parameters */
  ctrl_setDirection((params.direction == SEND) ? M7_SEND : M7_RECIEVE);
  // setup what is needed e.g. clk, memory buffer selection
}

void core1MeasurementTask( void *pvParameters ){
  ( void ) pvParameters;

  uart_measParams uartParams;
  uart_initUartMeasParams(&uartParams);

  uint8_t uartOutputBuffer[32];
  endMeasSemaphore = xSemaphoreCreateBinary(); // todo add init function
  
  HAL_UART_Transmit(&huart3, (unsigned char*)uart_getInitStr(), INIT_STR_LEN, HAL_MAX_DELAY);

  for( ;; )
  {
    processUartControl(&uartParams);

    prepareMeasParams(uartParams); // share with the other core

    for(uint32_t i = 0; i < uartParams.numMeas; ++i){
      /* Signaling to the other core*/
      generateInterruptIPC_startMeas(); // todo signalPartner func
      /* Waiting for message or sending message */
      switch (uartParams.direction)
      {
      case M7_SEND: /* M7 sends the message */
        app_measureCore1Sending(uartParams.dataSize);
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
// end of measurement task ===============================================

void app_createTasks(void){
  // creating the tasks for the M7 core
  const uint8_t mainAMP_TASK_PRIORITY = configMAX_PRIORITIES - 2;
  xTaskCreate(core1MeasurementTask, "AMPCore1", configMINIMAL_STACK_SIZE, \
      NULL, mainAMP_TASK_PRIORITY, &core1TaskHandle);
  configASSERT( core1TaskHandle );
}
