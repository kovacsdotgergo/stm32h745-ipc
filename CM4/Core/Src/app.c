#include "app.h"

TaskHandle_t core2TaskHandle;

/* Override the callback function to handle interrupts */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
  case GPIO_PIN_0:
    interruptHandlerIPC_messageBuffer();
    HAL_EXTI_D2_ClearFlag(EXTI_LINE0);
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

/* Interrupt for m7 core, handling message buffer function */
void generateInterruptIPC_messageBuffer(void* updatedMessageBuffer){
  MessageBufferHandle_t xUpdatedBuffer = ( MessageBufferHandle_t ) updatedMessageBuffer;
  
  if( xUpdatedBuffer != xControlMessageBuffer[MB2TO1_IDX] )
  {
    /* Use xControlMessageBuffer to pass the handle of the message buffer
    written to by core 1 to the interrupt handler about to be generated in
    core 2. */
    xMessageBufferSend( xControlMessageBuffer[MB2TO1_IDX], &xUpdatedBuffer,
                        sizeof( xUpdatedBuffer ), mbaDONT_BLOCK );
    
    /* This is where the interrupt would be generated. */
    HAL_EXTI_D2_EventInputConfig(MB2TO1_INT_EXTI_LINE, EXTI_MODE_IT, DISABLE);
    HAL_EXTI_D1_EventInputConfig(MB2TO1_INT_EXTI_LINE, EXTI_MODE_IT, ENABLE);
    HAL_EXTI_GenerateSWInterrupt(MB2TO1_INT_EXTI_LINE);
  }
}

/* Handler for the interrupts that are triggered on core 1 but execute on core 2. */
void interruptHandlerIPC_messageBuffer( void ){
  MessageBufferHandle_t xUpdatedMessageBuffer;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  /* xControlMessageBuffer contains the handle of the message buffer that
  contains data. */
  if( xMessageBufferReceiveFromISR( xControlMessageBuffer[MB1TO2_IDX],
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
  htim5.Instance = TIM5; // IMPORTANT to be able to read the timer! todo move somewhere else

  /* Int config for message buffer*/
  HAL_EXTI_EdgeConfig(MB2TO1_INT_EXTI_LINE, EXTI_RISING_EDGE);

  /* SW interrupt for message buffer */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0xFU, 0U);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  
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