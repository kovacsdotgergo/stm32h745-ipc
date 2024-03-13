#ifndef MEAS_CONTROL_H
#define MEAS_CONTROL_H

#include "hw_control_common.h"
#include "meas_control_common.h"
#include "semphr.h"

/**
 * @brief Initializes the line for the startMeas and endMeas interrupts in
 *  the hardware used for inter-processor interrupts
*/
void ctrl_initInterrupts(void);
/**
 * @brief Sets the semaphore signaling the start of the measurement
 * @param[in] startMeasSemaphore semaphore signaling the startMeas
 *  interrupt
*/
void ctrl_setStartMeasSemaphore(SemaphoreHandle_t startMeasSemaphore);

void generateInterruptIPC_endMeas(void);
void interruptHandlerIPC_startMeas(void);

#endif // MEAS_CONTROL_H