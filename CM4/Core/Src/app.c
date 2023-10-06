#include "app.h"

TaskHandle_t core2TaskHandle;

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
    app_measurementErrorHandler();
    break;
  }
}

/* m4 core task waiting for measurement and handling it */
void core2MeasurementTask( void *pvParameters )
{
  uint32_t notifiedValue;
  for( ;; )
  {   
    /* Wait for signal and direction of the measurement */
    do
    {
      /* Cast because of indefinite block*/
      (void)xTaskNotifyWait(pdFALSE, 0xffffffffUL, &notifiedValue, 
                            portMAX_DELAY);
    /* TODO Message buffer can also unblock unfortunately*/
    } while (!(notifiedValue & START_MEAS_BIT));
    
    /* Perform one measurement */
    switch (ctrl_getDirection())
    {
    case M7_SEND: /* m7 sends, this core recieves */
      app_measureCore2Recieving();
      break;
    case M7_RECIEVE:
      app_measureCore2Sending(ctrl_getDataSize());
      break;
    default:
      app_measurementErrorHandler();
      break;
    }
  }
}

void app_measureCore2Recieving(void){
  static uint8_t ulNextValue = 0;
  uint32_t xReceivedBytes, sizeFromMessage;
  static uint8_t recieveBuffer[ MAX_DATA_SIZE ];

  xReceivedBytes = xMessageBufferReceive( xDataMessageBuffers[MB1TO2_IDX],
                                          recieveBuffer,
                                          sizeof(recieveBuffer),
                                          portMAX_DELAY );
  time_endTime(); /* global shared variable */

  /* Checking the size and last element of the data */
  sscanf((char*)recieveBuffer, "%lu", &sizeFromMessage);
  if(xReceivedBytes != sizeFromMessage || 
      ((sizeFromMessage > 2) && recieveBuffer[xReceivedBytes - 1] != ulNextValue)){
    app_measurementErrorHandler();
  }

  memset( recieveBuffer, 0x00, xReceivedBytes );
  ulNextValue++;
  generateInterruptIPC_endMeasurement();
}

void app_measureCore2Sending(uint32_t dataSize){
  static char sendBuffer[MAX_DATA_SIZE];
  static uint8_t nextValue = 0;
  for (uint32_t j = 0; j < dataSize; ++j){
    sendBuffer[j] = nextValue;
  }
  sprintf((char*)sendBuffer, "%lu", dataSize);
  vTaskDelay(1/portTICK_PERIOD_MS);
  /* Start measurement */
  time_startTime();
  xMessageBufferSend(xDataMessageBuffers[MB2TO1_IDX],
                     (void*) sendBuffer,
                     dataSize,
                     mbaDONT_BLOCK);
  ++nextValue;                     
}

/* Init function */
void app_initMessageBufferAMP(void){
  /* Timer for time measurement */
  htim5.Instance = TIM5; // IMPORTANT to be able to read the timer! todo move somewhere else

  /* Int config for message buffer*/
  HAL_EXTI_EdgeConfig(MB2TO1_INT_EXTI_LINE, EXTI_RISING_EDGE);

  /* SW interrupt for message buffer */
  HAL_NVIC_SetPriority(MB1TO2_INT_EXTI_IRQ, 0xFU, 0U);
  HAL_NVIC_EnableIRQ(MB1TO2_INT_EXTI_IRQ);
  
  /* m7 core initializes the message buffers */
  if (( xControlMessageBuffer[MB1TO2_IDX] == NULL ) |
      ( xDataMessageBuffers[MB1TO2_IDX] == NULL ) |
      ( xControlMessageBuffer[MB2TO1_IDX] == NULL) |
      ( xDataMessageBuffers[MB2TO1_IDX] == NULL))
  {
    Error_Handler();
  }
}

/* Creating the tasks for the m4 core */
void app_createTasks(void){
  xTaskCreate(core2MeasurementTask, "AMPCore2", configMINIMAL_STACK_SIZE,
              NULL, tskIDLE_PRIORITY + 1, &core2TaskHandle);
  configASSERT(core2TaskHandle);
}

/* Error handler when the data isn't correct*/
void app_measurementErrorHandler(void){
  while(1){
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}