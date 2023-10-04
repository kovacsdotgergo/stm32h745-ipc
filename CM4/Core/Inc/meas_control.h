#ifndef MEAS_CONTROL_H
#define MEAS_CONTROL_H

#include "meas_control_common.h"
#include "app.h" // todo: remove this, it is used for the task handle, the init task should instantiate a semaphore used for signaling, and the task should use that

void ctrl_initInterrupts(void);

void generateInterruptIPC_endMeasurement(void);
void interruptHandlerIPC_startMeas(void);

#endif // MEAS_CONTROL_H