#ifndef UART_STATE_MACHINE_H
#define UART_STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdalign.h>
#include "string_n.h"

#define LINE_BUFFER_LEN 64
#define CMD_DELIMITERS " \t"

#define CLK_DIV_UP_LIMIT 16 // todo div limits
#define CLK_DIV_LOW_LIMIT 1
static_assert(CLK_DIV_LOW_LIMIT <= CLK_DIV_UP_LIMIT);

#define REPETITION_UP_LIMIT 2048 // todo limit

#define DATASIZE_UP_LIMIT 16376
#define DATASIZE_LOW_LIMIT 1
static_assert(DATASIZE_LOW_LIMIT <= DATASIZE_UP_LIMIT);

// X macro for commands
// X(command, capital command, arg num, help string)
#define COMMANDS(X) \
    X(help, Help, 0, "help: displays this message") \
    X(getparams, Getparams, 0, "getparams: displays the current value of the measurement parameters") \
    X(start, Start, 0, "start: starts a measurement with the given parameters") \
    X(direction, Direction, 1, "direction <dir>: sets the direction from the M7 viewpoint\r\n\t\tdir can be 'send', 's', 'receive' or 'r'") \
    X(clk, Clk, 3, "clk <div...> <div...> <div...>: sets the clk division registers") \
    X(repeat, Repeat, 1, "repeat <num>: sets the repetition count of the measurement, can be saturated") \
    X(datasize, Datasize, 1, "datasize <size>: sets the size of the measured message, can be saturated")
    // todo add reset
    // todo add help

// maximum of the arg num parameter, char always has the strictest alignment (1)
#define X_TO_MAX_ARGNUM_UNION(command, cap, num, help) uint8_t command[num];
typedef union {
    COMMANDS(X_TO_MAX_ARGNUM_UNION)
} max_argnum_t;
#define MAX_ARG_NUM sizeof(max_argnum_t)

// structure holding the command and the function parsing the arguments
#define X_TO_UART_COMMANDS_STRUCT(command, cap, num, help) \
    {.cmd = #command, .arg_num = num, .parseArgFun = uart_parse##cap##Cmd,},
#define COMMANDS_STRUCT \
    { \
        COMMANDS(X_TO_UART_COMMANDS_STRUCT) \
    }

// constant help string
#define X_TO_HELP_STR(command, cap, num, help) "\t*" help "\r\n"
#define HELP_STR \
    "Available commands:\r\n" \
    COMMANDS(X_TO_HELP_STR)
#define HELP_STR_LEN (sizeof(HELP_STR) - 1)

#define INIT_STR "\r\nIPC performance measurement application for FreeRTOS" \
                    " Message Buffers\r\n\n" \
                 HELP_STR // todo possibly hal, freertos version
#define INIT_STR_LEN (sizeof(INIT_STR) - 1)

#define PROMPT_STR "> "
#define PROMPT_STR_LEN (sizeof(PROMPT_STR) - 1)

typedef enum{
    SEND,
    RECEIVE,
} uart_measDirection;

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
    uint32_t numMeas;
    uint32_t dataSize;
    uart_measDirection direction;
    uint8_t clk_div1;
    uint8_t clk_div2;
    uint8_t clk_div3;
    bool startMeas;
    // todo memory
    // todo endpoints possibly on the same processor ~ enum source and target
    // todo getparams to print
} uart_measParams;

typedef struct {
    char buffer[LINE_BUFFER_LEN];
    size_t len;
} uart_LineBuffer;

typedef struct {
    const char* cmd;
    const uint32_t arg_num;
    uart_parseStatus (*parseArgFun)(const char* args[MAX_ARG_NUM],
                                    size_t len[MAX_ARG_NUM],
                                    uart_measParams* uartParams,
                                    const char** msg);
} uart_Command;

/** @brief Returns prompt string */
static inline const char* uart_getPrompt(void) {
    return PROMPT_STR;
}

/** @brief Returns welcome message string for the application */
static inline const char* uart_getInitStr(void) {
    return INIT_STR;
}

/** @brief Returns the help string for the UART commands */
static inline const char* uart_getHelpStr(void) {
    return HELP_STR;
}

/** @brief Initializes the structure to the default values */
void uart_initUartMeasParams(uart_measParams* measParams);

/** @brief Clears some of the the values to prepare for the next parse */
void uart_clearUartMeasParams(uart_measParams* measParams);

/** @returns The string equivalent for the enum */
static inline const char* uart_measDirectionToStr(uart_measDirection dir) {
    assert(dir == SEND || dir == RECEIVE);
    return dir == SEND ? "M7 send" : "M7 receive";
}

/** @brief Initializes the buffer to the default values */
void uart_initLineBuffer(uart_LineBuffer* lineBuffer);

/** @brief Clears the buffer */
void uart_clearLineBuffer(uart_LineBuffer* lineBuffer);

/** 
 * @brief Store characters in a buffer of length LINE_BUFFER_LEN until newline char
 * @note using a single CR as the line end character as it is the PUTTY default 
 * @param[in] uartInput input char
 * @param[in] lineBuffer the buffer to store characters to
 * @param[out] echo string to echo
 * @returns status of the buffer
*/
uart_BufferStatus uart_addCharToBuffer(char uartInput, 
                                       uart_LineBuffer* lineBuffer,
                                       const char** echo);

/**
 * @brief Parse the line buffer and execute the corresponding command on uartParams
 * @note case insensitive commands containing alphas
 * @param[in] lineBuffer the input linebuffer
 * @param[inout] uartParams the params to modify
 * @param[out] msg warning and info messages
 * @returns false if the parsing failed
*/
uart_parseStatus uart_parseBuffer(const uart_LineBuffer* lineBuffer,
                                  uart_measParams* uartParams,
                                  const char** msg);

// function definition for each parsing function 
#define X_TO_TOKENIZE_FUN_DECL(command, cap, num, help) \
    uart_parseStatus \
    uart_parse##cap##Cmd( \
        const char* toks[MAX_ARG_NUM], size_t toklens[MAX_ARG_NUM], \
        uart_measParams* uartParams, const char** msg);
COMMANDS(X_TO_TOKENIZE_FUN_DECL)

#ifdef TEST
    
#endif

#endif /* UART_STATE_MACHINE_H */
