#ifndef MEAS_CONTROL_H
#define MEAS_CONTROL_H

#include "hw_control_common.h"
#include "meas_control_common.h"
#include "app.h"

void ctrl_initInterrupts(void);

void generateInterruptIPC_endMeasurement(void);
void interruptHandlerIPC_startMeas(void);

#endif // MEAS_CONTROL_H