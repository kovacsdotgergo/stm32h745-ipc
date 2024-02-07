#include <app.h>

TaskHandle_t core1TaskHandle;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  switch (GPIO_Pin)
  {
  case END_MEAS_GPIO_PIN:
    ctrl_interruptHandlerIPC_endMeas();
    break;
  case MB2TO1_GPIO_PIN:
    mb_interruptHandlerIPC_messageBuffer();
    HAL_EXTI_D1_ClearFlag(MB2TO1_GPIO_PIN);
    break;
  default:
    assert(false);
    break;
  }
}

void app_createTasks(UART_HandleTypeDef* meastaskHuart){
  // m7 meas task
  const uint8_t mainAMP_TASK_PRIORITY = configMAX_PRIORITIES - 2;
  xTaskCreate(meastask_core1MeasurementTask, "AMPCore1",
              MEASTASK_STACK_SIZE, meastaskHuart,
              mainAMP_TASK_PRIORITY, &core1TaskHandle);
  configASSERT( core1TaskHandle );
}
