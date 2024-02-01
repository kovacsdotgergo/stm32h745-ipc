#ifndef APP_H
#define APP_H

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
/* ST */ 
#include "main.h" /* gpio pin definitions, hal include, htim5 definition, huart3 */
/* Own files*/
#include "ipc_mb_common.h"
#include "meas_task.h"

extern TaskHandle_t core1TaskHandle;
extern SemaphoreHandle_t app_endMeasSemaphore;

/* Callback handling the exti interrupts */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

void app_createTasks(void);

#endif /* APP_H */