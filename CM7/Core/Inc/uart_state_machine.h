#ifndef UART_STATE_MACHINE_H
#define UART_STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#define NUM_MEAS_STRING_LEN 16
#define MEAS_DATA_SIZE_STRING_LEN 16 // todo remove if not needed, cleanup this file

#define LINE_BUFFER_LEN 64
#define CMD_DELIMITERS " \t"
#define DIV_LIMITS {16, 16, 16} // todo div limits
#define REPETITION_LIMIT 2048 // todo limit
#define DATASIZE_LIMIT 16376
#define COMMANDS {{.cmd = "clk", .parseArgFun = uart_parseClkCmd,}, \
                  {.cmd = "direction", .parseArgFun = uart_parseDirectionCmd,}, \
                  {.cmd = "start", .parseArgFun = uart_parseStartCmd,}, \
                  {.cmd = "repeat", .parseArgFun = uart_parseRepeatCmd,}, \
                  {.cmd = "datasize", .parseArgFun = uart_parseDatasizeCmd,}, \
                  {.cmd = "getparams", .parseArgFun = uart_parseGetparamsCmd,}, \
                  {.cmd = "help", .parseArgFun = uart_parseHelpCmd},} 
                  // todo add reset
                  // todo add help

#define HELP_STR "Available commands:\r\n" \
                 "\t* help: displays this message\r\n" \
                 "\t* clk <div...> <div...> <div...>: " \
                    "sets the clk division registers\r\n"\
                 "\t* direction <dir>: sets the direction " \
                    "from the M7 viewpoint\r\n" \
                    "\t\tdir can be 'send', 's', 'receive' or 'r'\r\n" \
                 "\t* start: starts a measurement with the given " \
                    "parameters\r\n" \
                 "\t* repeat <num>: sets the repetition count of the " \
                    "measurement, can be saturated\r\n" \
                 "\t* datasize <size>: sets the size of the measured " \
                    " message, can be saturated\r\n"
#define HELP_STR_LEN (sizeof(HELP_STR) - 1)

#define INIT_STR "IPC performance measurement application for FreeRTOS" \
                    "Message Buffers\r\n\n" \
                 HELP_STR // todo possibly hal, freertos version
#define INIT_STR_LEN (sizeof(INIT_STR) - 1)

#define PROMPT_STR "> "
#define PROMPT_STR_LEN (sizeof(PROMPT_STR) - 1)

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
    bool printHelp;
    bool printParams;
    // todo memory
    // todo endpoints possibly on the same processor ~ enum source and target
    // todo getparams to print
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
 * @brief Returns prompt string
*/
static inline const char* uart_getPrompt(void) {
    return PROMPT_STR;
}

/**
 * @brief Returns welcome message string for the application 
*/
static inline const char* uart_getInitStr(void) {
    return INIT_STR;
}

/**
 * @brief Returns the help string for the UART commands
*/
static inline const char* uart_getHelpStr(void) {
    return HELP_STR;
}

/** 
 * @brief Store characters in a buffer of length LINE_BUFFER_LEN until newline char
 * @note using a single CR as the line end character as it is the PUTTY default 
 * @param[in] uartInput input char
 * @param[in] lineBuffer the buffer to store characters to
 * @returns status of the buffer
*/
uart_BufferStatus uart_addCharToBuffer(char uartInput, 
                                       uart_LineBuffer* lineBuffer);


/**
 * @brief Parse the line buffer and execute the corresponding command on uartParams
 * @note case insensitive commands containing alphas
 * @param[in] lineBuffer the input linebuffer
 * @param[in] uartParams the params to modify
 * @returns false if the parsing failed
*/
uart_parseStatus uart_parseBuffer(const uart_LineBuffer* lineBuffer,
                                  uart_measParams* uartParams);

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

/**
 * @brief Parses the arguments of the 'getparams' command and modifies the uartParams
 *  accordingly
*/
uart_parseStatus uart_parseGetparamsCmd(const char* args, size_t len,
                                    uart_measParams* const uartParams);

/**
 * @brief Parses the arguments of the 'help' command and modifies the uartParams
*/
uart_parseStatus uart_parseHelpCmd(const char* args, size_t len,
                                    uart_measParams* const uartParams);

#endif /* UART_STATE_MACHINE_H */