#ifndef MEAS_CONTROL_COMMON_H
#define MEAS_CONTROL_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "mb_config.h" // for datasize limit
#include "shared_param_types.h"

#define START_MEAS_INT_EXTI_IRQ EXTI3_IRQn
#define START_MEAS_INT_EXTI_LINE EXTI_LINE3
#define START_MEAS_GPIO_PIN GPIO_PIN_3

#define END_MEAS_GPIO_PIN GPIO_PIN_2
#define END_MEAS_INT_EXTI_LINE EXTI_LINE2
#define END_MEAS_INT_EXTI_IRQ EXTI2_IRQn

// Values used for validation of parameters
#define REPETITION_UP_LIMIT 16384

#define DATASIZE_UP_LIMIT MB_MAX_DATA_SIZE
#define DATASIZE_LOW_LIMIT 1

//static_assert(DATASIZE_LOW_LIMIT <= DATASIZE_UP_LIMIT);

// Functions handling the shared variables
/** 
 * @brief Sets the shared datasize variable
 * @param[out] msg optional message
 * @returns true on success, false otherwise
*/
bool ctrl_setDataSize(uint32_t dataSize, const char** msg);
/** @brief Returns the shared datasize variable */
uint32_t ctrl_getDataSize(void);

/** 
 * @brief Sets the shared direction variable
 * @param[out] msg optional message
 * @returns true on success, false otherwise
*/
bool ctrl_setDirection(params_direction direction, const char** msg);
/** @brief Returns the shared direction variable */
params_direction ctrl_getDirection(void);

/** 
 * @brief Sets the shared repeat variable
 * @param[out] msg optional message
 * @returns true on success, false otherwise
*/
bool ctrl_setRepeat(uint32_t repeat, const char** msg);
/** @brief Returns the shared repeat variable */
uint32_t ctrl_getRepeat(void);

/** @brief  Generates an interrupt used for signaling during IPC */
void generateIT_IPC(uint32_t EXTI_Line);


#endif // MEAS_CONTROL_COMMON_H