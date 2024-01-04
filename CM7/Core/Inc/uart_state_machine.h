#ifndef UART_STATE_MACHINE_H
#define UART_STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define NUM_MEAS_STRING_LEN 16
#define MEAS_DATA_SIZE_STRING_LEN 16

#define LINE_BUFFER_LEN 64
#define CMD_DELIMITERS " \t"
#define DIV_LIMITS {1, 1, 1} // todo div limits

typedef struct {
    char buffer[LINE_BUFFER_LEN];
    size_t len;
} uart_LineBuffer;

typedef enum {
    BUFFER_OVERFLOW,
    BUFFER_DONE,
    BUFFER_OK,
} uart_BufferStatus;

typedef enum { // todo could merge with the buffer errors
    PARSE_COMMAND_ERR,
    PARSE_ARG_NUM_ERR,
    PARSE_ARG_VAL_ERR,
    PARSE_OK,
} uart_parseStatus;

typedef enum{
    IDLE,
    NUM_OF_MEAS_NEXT,
    DATA_SIZE_NEXT,
    CLOCKS_NEXT,
} uartStates;

typedef enum{
    SEND,
    RECIEVE,
} uart_measDirection;

typedef struct {
    uint32_t numMeas;
    uint32_t dataSize;
    uart_measDirection direction;
    uint8_t clk_div1;
    uint8_t clk_div2;
    uint8_t clk_div3;
    bool startMeas;
    // todo memory
    // todo endpoints possibly on the same processor ~ enum source and target
} uart_measParams;

typedef struct{
    uartStates state;
    char stringNumMeas[NUM_MEAS_STRING_LEN];
    char stringMeasData[MEAS_DATA_SIZE_STRING_LEN];
    uint8_t stringIndex;
    uart_measDirection direction;
} uartStateMachine;

/** 
 * @brief Steps state machine, returns true if measurement can be started
 * 
 * @param[in] input Input of the state machine
 * @param[inout] stateMachine State of the state machine
 * @param[out] pNumMeas Contains the measurement repetition count when the\
 *  measurement can be started 
 * @param[out] pMeasDataSize Contains the data size to be sent when the\
 *  measurement can be started
 * @param[out] pMeasDirection Contains the measurement direction when the\
 *  measurement can be started
 * @returns True if the measurement can be started
 * 
 * @note Length of the string given character by character for the output\
 *  values can be NUM_MEAS_STRING_LEN and MEAS_DATA_SIZE_STRING_LEN
 **/
bool uart_stateMachineStep(char input, uartStateMachine* stateMachine,
        uint32_t* pNumMeas, uint32_t* pMeasDataSize, 
        uart_measDirection* pMeasDirection);

/**
 * @brief Resets the state of the state machine and the internal arrays as 
 *  well
*/
void uart_resetSM(uartStateMachine *stateMachine);

/** 
 * @brief Store characters in a buffer of length LINE_BUFFER_LEN until newline char
 * @note using a single CR as the line end character as it is the PUTTY default 
 * @param[in] uartInput input char
 * @param[in] lineBuffer the buffer to store characters to
 * @returns status of the buffer
*/
uart_BufferStatus uart_addCharToBuffer(char uartInput, 
                                       uart_LineBuffer* const lineBuffer);


/**
 * @brief Parse the line buffer and execute the corresponding command on uartParams
 * @note case insensitive commands containing alphas
 * @param[in] lineBuffer the input linebuffer
 * @param[in] uartParams the params to modify
 * @returns false if the parsing failed
*/
uart_parseStatus uart_parseBuffer(const uart_LineBuffer* const lineBuffer,
                                  uart_measParams* const uartParams);

#endif /* UART_STATE_MACHINE_H */