#ifndef UART_STATE_MACHINE_H
#define UART_STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define NUM_MEAS_STRING_LEN (16)
#define MEAS_DATA_SIZE_STRING_LEN (16)

typedef enum{
    IDLE,
    NUM_OF_MEAS_NEXT,
    DATA_SIZE_NEXT,
} uartStates;

typedef struct{
    uartStates state;
    char stringNumMeas[NUM_MEAS_STRING_LEN];
    char stringMeasData[MEAS_DATA_SIZE_STRING_LEN];
    uint8_t stringIndex;
}uartStateMachine;

/** todo
 * @brief Steps state machine, returns true if measurement can be started
 * 
 * @param[in] input Input of the state machine
 * @param[inout] state State of the state machine
 * @param[out] pNumMeas Contains the measurement repetition count when the\
 *  measurement can be started 
 * @param[out] pMeasDataSize Contains the data size to be sent when the\
 *  measurement can be started
 * @returns True if the measurement can be started
 * 
 * @note Length of the string given character by character for the output\
 *  values can be NUM_MEAS_STRING_LEN and MEAS_DATA_SIZE_STRING_LEN
 **/
bool uartStateMachineStep(char input, uartStateMachine* stateMachine,
         uint32_t* pNumMeas, uint32_t* pMeasDataSize);

void resetSM(uartStateMachine *stateMachine);

#endif /* UART_STATE_MACHINE_H */