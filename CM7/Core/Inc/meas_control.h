#ifndef MEAS_CONTROL_H
#define MEAS_CONTROL_H

#include "hw_control_common.h"
#include "meas_control_common.h"
#include "app.h"

enum { // rm0399, pg. 166, number of wait states based on the axi clk
    VOS0_0WS_MAX_AXI_CLK = 70000000U,
    VOS0_1WS_MAX_AXI_CLK = 140000000U,
    VOS0_2WS_MAX_AXI_CLK = 185000000U,
    VOS0_3WS_MAX_AXI_CLK = 225000000U,
    VOS0_4WS_MAX_AXI_CLK = 240000000U,
};

/** @brief Sets up the required peripherals for the inter-core interrupt*/
void ctrl_initInterrupts(void);

/** @brief Triggers interupt used for IPC signaling start of meas */
void ctrl_generateInterruptIPC_startMeas(void);

/** @brief Interrupt handler for int triggered from Core2, used for singaling end
    of measurement */
void ctrl_interruptHandlerIPC_endMeas( void );

/** 
 * @brief Sets the clock dividers on the fly for the M7 and M4 cores
 * @param[in] m7clk M7 core clock in [Hz]
 * @param[in] m4clk M4 core clock in [Hz]
 * @param[in] msg optional output message
 * @returns true on success, false otherwise
*/
bool ctrl_setClks(uint32_t m7clk, uint32_t m4clk, const char** msg);

/**
 * @brief Sets the clock output parameters
 * @param[out] m7clk M7 core clock in [Hz]
 * @param[out] m4clk M4 core clock in [Hz]
*/
void ctrl_getClks(uint32_t* m7clk, uint32_t* m4clk);

#endif // MEAS_CONTROL_H