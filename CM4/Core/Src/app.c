#include "app.h"

TaskHandle_t core2TaskHandle;
SemaphoreHandle_t startMeasSemaphore = NULL;

/* Override the callback function to handle interrupts */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
  case MB1TO2_GPIO_PIN:
    interruptHandlerIPC_messageBuffer();
    HAL_EXTI_D2_ClearFlag(MB1TO2_INT_EXTI_LINE);
    break;
  case START_MEAS_GPIO_PIN:
    interruptHandlerIPC_startMeas();
    break;
  default:
    ErrorHandler();
    break;
  }
}

/* m4 core task waiting for measurement and handling it */
void core2MeasurementTask( void *pvParameters )
{
  startMeasSemaphore = xSemaphoreCreateBinary();

  for( ;; )
  {   
    /* Wait for start signal and direction of the measurement */
    (void)xSemaphoreTake( startMeasSemaphore, portMAX_DELAY ); // indefinite block
    
    /* Perform one measurement */
    switch (ctrl_getDirection())
    {
    case M7_SEND: /* m7 sends, this core recieves */
      app_measureCore2Recieving();
      break;
    case M7_RECEIVE:
      app_measureCore2Sending(ctrl_getDataSize());
      break;
    default:
      ErrorHandler();
      break;
    }
  }
}

void app_measureCore2Recieving(void){
  static uint8_t ulNextValue = 0;
  uint32_t xReceivedBytes, sizeFromMessage;
  static uint8_t recieveBuffer[ MB_MAX_DATA_SIZE ];

  xReceivedBytes = xMessageBufferReceive( xDataMessageBuffers[MB1TO2_IDX],
                                          recieveBuffer,
                                          sizeof(recieveBuffer),
                                          portMAX_DELAY );
  time_endTime(); /* global shared variable */
  time_setSharedOffset();
  /* Checking the size and last element of the data */
  sscanf((char*)recieveBuffer, "%lu", &sizeFromMessage);
  if(xReceivedBytes != sizeFromMessage || 
      ((sizeFromMessage > 2) && recieveBuffer[xReceivedBytes - 1] != ulNextValue)){
    ErrorHandler();
  }

  memset( recieveBuffer, 0x00, xReceivedBytes );
  ulNextValue++;
  generateInterruptIPC_endMeasurement();
}

void app_measureCore2Sending(uint32_t dataSize){
  static char sendBuffer[MB_MAX_DATA_SIZE];
  static uint8_t nextValue = 0;
  for (uint32_t j = 0; j < dataSize; ++j){
    sendBuffer[j] = nextValue;
  }
  sprintf((char*)sendBuffer, "%lu", dataSize);
  vTaskDelay(1/portTICK_PERIOD_MS);
  time_setSharedOffset();
  /* Start measurement */
  time_startTime();
  xMessageBufferSend(xDataMessageBuffers[MB2TO1_IDX],
                     (void*) sendBuffer,
                     dataSize,
                     mbaDONT_BLOCK);
  ++nextValue;                     
}

/* Creating the tasks for the m4 core */
void app_createTasks(void){
  xTaskCreate(core2MeasurementTask, "AMPCore2", configMINIMAL_STACK_SIZE,
              NULL, tskIDLE_PRIORITY + 1, &core2TaskHandle);
  configASSERT(core2TaskHandle);
}
