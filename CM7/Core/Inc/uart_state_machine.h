#ifndef UART_STATE_MACHINE_H
#define UART_STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_MEAS_STRING_LEN (16)
#define MEAS_DATA_SIZE_STRING_LEN (16)

typedef enum{
    IDLE = 0,
    NUM_OF_MEAS = 1,
    NUM_OF_MEAS_MIGHT_END = 2,
    DATA_SIZE = 3,
    DATA_SIZE_MIGHT_END = 3,
} uartStates;

/**
 * @brief Steps state machine, returns true if measurement can be started
 * 
 * @param[in] input State machine input
 * @param[out] pNumMeas Contains the measurement repetition count when the\
 *  measurement can be started 
 * @param[out] pMeasDataSize Contains the data size to be sent when the\
 *  measurement can be started
 * @returns True if the measurement can be started
 * 
 * @note Length of the string given character by character for the output\
 *  values can be NUM_MEAS_STRING_LEN and MEAS_DATA_SIZE_STRING_LEN
 **/
bool uartStateMachineStep(uint8_t input, uint32_t* pNumMeas, uint32_t* pMeasDataSize);

#endif /* UART_STATE_MACHINE_H */