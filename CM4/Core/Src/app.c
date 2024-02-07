#include "app.h"

TaskHandle_t core2TaskHandle;

/* Override the callback function to handle interrupts */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
  case MB1TO2_GPIO_PIN:
    mb_interruptHandlerIPC_messageBuffer();
    HAL_EXTI_D2_ClearFlag(MB1TO2_INT_EXTI_LINE);
    break;
  case START_MEAS_GPIO_PIN:
    interruptHandlerIPC_startMeas();
    break;
  default:
    assert(false);
    break;
  }
}

/* m4 core task waiting for measurement and handling it */
void core2MeasurementTask( void *pvParameters )
{
  (void)pvParameters;
  SemaphoreHandle_t startMeasSemaphore = xSemaphoreCreateBinary();
  ctrl_setStartMeasSemaphore(startMeasSemaphore);
  while (1)
  {   
    /* Wait for start signal and direction of the measurement */
    (void)xSemaphoreTake( startMeasSemaphore, portMAX_DELAY ); // indefinite block
    
    mb_setUsedMemory(ctrl_getMemory());
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
      assert(false);
      break;
    }
  }
}

void app_measureCore2Recieving(void){
  static uint8_t ulNextValue = 0;
  uint32_t xReceivedBytes, sizeFromMessage;
  static uint8_t recieveBuffer[ MB_MAX_DATA_SIZE ];

  xReceivedBytes = xMessageBufferReceive( mb_gpCurrentDataMB[DATA_RECV_IDX],
                                          recieveBuffer,
                                          sizeof(recieveBuffer),
                                          portMAX_DELAY );
  time_endTime(); /* global shared variable */
  time_setSharedOffset();
  /* Checking the size and last element of the data */
  sscanf((char*)recieveBuffer, "%lu", &sizeFromMessage);
  if(xReceivedBytes != sizeFromMessage || 
      ((sizeFromMessage > 2) && recieveBuffer[xReceivedBytes - 1] != ulNextValue)){
    assert(false);
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
  xMessageBufferSend(mb_gpCurrentDataMB[DATA_SEND_IDX],
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
