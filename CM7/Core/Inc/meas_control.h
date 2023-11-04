#ifndef MEAS_CONTROL_H
#define MEAS_CONTROL_H

#include "meas_control_common.h"
#include "app.h"

/* Setting up the required peripherals for the inter-core interrupt*/
void ctrl_initInterrupts(void);

/* Triggering interupt used for IPC signaling start of meas */
void generateInterruptIPC_startMeas(void);

/* Interrupt handler for int triggered from Core2, used for singaling end
    of measurement */
void interruptHandlerIPC_endMeas( void );

#endif // MEAS_CONTROL_H