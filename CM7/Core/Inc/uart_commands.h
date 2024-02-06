#ifndef UART_COMMANDS_H
#define UART_COMMANDS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "string_n.h"
#include "shared_param_types.h"

#define LINE_BUFFER_LEN 64
#define CMD_DELIMITERS " \t"

#define PUTTY_DEL 127
#define PUTTY_SENDS_NEW_LINE '\r'
#define PUTTY_PRINTS_NEW_LINE "\r\n"

// X macro for commands
// X(command, capital command, arg num, help string)
#define COMMANDS(X) \
    X(help, Help, 0, "help: displays this message") \
    X(getparams, Getparams, 0, "getparams: displays the current value of the measurement parameters") \
    X(start, Start, 0, "start: starts a measurement with the given parameters") \
    X(direction, Direction, 1, "direction <dir>: sets the direction from the M7 viewpoint\r\n\t\tdir can be 'send', 's', 'receive' or 'r'") \
    X(clk, Clk, 2, "clk <m7clk[Hz]> <m4clk[Hz]>: sets the clk frequencies if possible") \
    X(repeat, Repeat, 1, "repeat <num>: sets the repetition count of the measurement, can be saturated") \
    X(datasize, Datasize, 1, "datasize <size>: sets the size of the measured message, can be saturated") \
    X(mem, Mem, 1, "mem <memory>: sets the memory used by the Message Buffers\r\n\t\tmemory can be 'D1', 'D2' or 'D3'")
    // todo add reset

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
                 "Backspace is supported when typing in commands\r\n\n" \
                 HELP_STR // todo possibly hal, freertos version
#define INIT_STR_LEN (sizeof(INIT_STR) - 1)

#define PROMPT_STR "> "
#define PROMPT_STR_LEN (sizeof(PROMPT_STR) - 1)

typedef enum {
    BUFFER_OVERFLOW,
    BUFFER_DONE,
    BUFFER_OK,
} uart_BufferStatus;

typedef enum {
    PARSE_COMMAND_ERR,
    PARSE_ARG_NUM_ERR,
    PARSE_ARG_VAL_ERR,
    PARSE_OK,
} uart_parseStatus;

typedef struct { // NOTE: the functions return true on success, false otherwise
    bool (*setClks)(uint32_t clkM7, uint32_t clkM4, const char** msg);
    void (*getClks)(uint32_t* clkM7, uint32_t* clkM4);
    bool (*setDataSize)(uint32_t datasize, const char** msg);
    uint32_t (*getDataSize)(void);
    bool (*setRepeat)(uint32_t repeat, const char** msg);
    uint32_t (*getRepeat)(void);
    bool (*setDirection)(params_direction direction, const char** msg);
    params_direction (*getDirection)(void);
    bool (*setMem)(params_mem mem, const char** msg);
    params_mem (*getMem)(void);
    void (*setStartMeas)(void);
    // todo endpoints possibly on the same processor ~ enum source and target
} uart_controlIf;

typedef struct {
    char buffer[LINE_BUFFER_LEN];
    size_t len;
} uart_LineBuffer;

typedef struct {
    const char* cmd;
    const uint32_t arg_num;
    uart_parseStatus (*parseArgFun)(const char* args[MAX_ARG_NUM],
                                    size_t len[MAX_ARG_NUM],
                                    const uart_controlIf* controlFuns,
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
                                  const uart_controlIf* controlFuns,
                                  const char** msg);

// function definition for each parsing function 
#define X_TO_TOKENIZE_FUN_DECL(command, cap, num, help) \
    uart_parseStatus \
    uart_parse##cap##Cmd( \
        const char* toks[MAX_ARG_NUM], size_t toklens[MAX_ARG_NUM], \
        const uart_controlIf* controlFuns, const char** msg);
COMMANDS(X_TO_TOKENIZE_FUN_DECL)

#endif /* UART_COMMANDS_H */
