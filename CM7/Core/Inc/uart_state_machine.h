#ifndef UART_STATE_MACHINE_H
#define UART_STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#define NUM_MEAS_STRING_LEN 16
#define MEAS_DATA_SIZE_STRING_LEN 16

#define LINE_BUFFER_LEN 64
#define CMD_DELIMITERS " \t"
#define DIV_LIMITS {16, 16, 16} // todo div limits
#define REPETITION_LIMIT 2048
#define DATASIZE_LIMIT 2048
#define COMMANDS {{.cmd = "clk", .parseArgFun = uart_parseClkCmd,}, \
                  {.cmd = "direction", .parseArgFun = uart_parseDirectionCmd,}, \
                  {.cmd = "start", .parseArgFun = uart_parseStartCmd,}, \
                  {.cmd = "repeat", .parseArgFun = uart_parseRepeatCmd,}, \
                  {.cmd = "datasize", .parseArgFun = uart_parseDatasizeCmd,},} // todo

typedef enum{
    SEND,
    RECEIVE,
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

typedef struct {
    const char* cmd;
    uart_parseStatus (*parseArgFun)(const char* args, size_t len,
                                    uart_measParams* uartParams);
} uart_Command;

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

/**
 * @brief Parses the arguments of the 'datasize' command and modifies the
 *  uartParams accordingly
*/
uart_parseStatus uart_parseDatasizeCmd(const char* args, size_t len,
                                         uart_measParams* uartParams);

/**
 * @brief Parses the arguments of the 'repeat' command and modifies the
 *  uartParams accordingly
*/
uart_parseStatus uart_parseRepeatCmd(const char* args, size_t len,
                                      uart_measParams* uartParams);

/**
 * @brief Parses the arguments of the 'start' command and modifies the
 *  uartParams accordingly 
*/
uart_parseStatus uart_parseStartCmd(const char* args, size_t len,
                                      uart_measParams* uartParams);

/**
 * @brief Parses the arguments of the 'direction' command and modifies the
 *  uartParams accordingly
*/
uart_parseStatus uart_parseDirectionCmd(const char* args, size_t len,
                                          uart_measParams* uartParams);

/**
 * @brief Parses the arguments of the 'clk' command and modifies the uartParams
 *  accordingly
*/
uart_parseStatus uart_parseClkCmd(const char* args, size_t len,
                                    uart_measParams* const uartParams);

#endif /* UART_STATE_MACHINE_H */