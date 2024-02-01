#ifndef MEAS_CONTROL_H
#define MEAS_CONTROL_H

#include "meas_control_common.h"
#include "app.h"

enum { // rm0399, pg. 166, number of wait states based on the axi clk
    VOS0_0WS_MAX_AXI_CLK = 70000000U,
    VOS0_1WS_MAX_AXI_CLK = 140000000U,
    VOS0_2WS_MAX_AXI_CLK = 185000000U,
    VOS0_3WS_MAX_AXI_CLK = 225000000U,
    VOS0_4WS_MAX_AXI_CLK = 240000000U,
};

typedef enum {
    CLK_M7_ERR,
    CLK_M4_ERR,
    CLK_OK,
} ClkErr;

/* Setting up the required peripherals for the inter-core interrupt*/
void ctrl_initInterrupts(void);

/* Triggering interupt used for IPC signaling start of meas */
void ctrl_generateInterruptIPC_startMeas(void);

/* Interrupt handler for int triggered from Core2, used for singaling end
    of measurement */
void ctrl_interruptHandlerIPC_endMeas( void );

/* Setting the clock dividers on the fly for the M7 and M4 cores */
ClkErr ctrl_setupClk(uint32_t m7clk, uint32_t m4clk);

/* Returns the clock frequencies in Hz */
void ctrl_getClks(uint32_t* m7clk, uint32_t* m4clk);

/* Checks if the given clocks can be set in the system 
    this function could be eliminated if the command would set the actual parameters not the measparams struct */
ClkErr ctrl_validateClks(uint32_t m7clk, uint32_t m4clk);


#endif // MEAS_CONTROL_H