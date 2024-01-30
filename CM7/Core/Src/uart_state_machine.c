#include "uart_state_machine.h"

void uart_initUartMeasParams(uart_measParams* measParams) {
    measParams->numMeas = 1;
    measParams->dataSize = DATASIZE_LOW_LIMIT;
    measParams->direction = SEND;
    measParams->clk_m7 = 1; // todo meaningful init
    measParams->clk_m4 = 1;
    measParams->startMeas = false;
}

void uart_clearUartMeasParams(uart_measParams* measParams) {
    measParams->startMeas = false;
}

void uart_initLineBuffer(uart_LineBuffer* lineBuffer) {
    lineBuffer->len = 0;
}

void uart_clearLineBuffer(uart_LineBuffer* lineBuffer) {
    lineBuffer->len = 0;
}

uart_BufferStatus uart_addCharToBuffer(char uartInput,
                                       uart_LineBuffer* lineBuffer,
                                       const char** echo) {
    assert(lineBuffer != NULL);
    static char echoBuf[2] = {0};
    *echo = NULL;
    if (uartInput == '\r') { // line end sequence
        *echo = "\r\n";
        return BUFFER_DONE;
    }
    if (uartInput == '\b') { // delete ch on backspace
        if (0 < lineBuffer->len) {
            --lineBuffer->len;
            *echo = "\b";
        }
        return BUFFER_OK;
    }
    if (LINE_BUFFER_LEN <= lineBuffer->len) {
        return BUFFER_OVERFLOW;
    }
    lineBuffer->buffer[lineBuffer->len++] = uartInput;
    echoBuf[0] = uartInput;
    *echo = echoBuf;
    return BUFFER_OK;
}

/**
 * @brief Copies the null terminated string to the buffer
 * @returns the number of characters written
 * @note The input pointers can not overlap, no bound checking
*/
static size_t addStrToBuf(char* restrict buf, const char* restrict str){
    strcpy(buf, str);
    return strlen(str);
}

/**
 * @brief Splits the not null-terminated string of args into the given num
 *  tokens using CMD_DELIMITERS
 * @param[in] args the string of the arguments
 * @param[in] len length of the args string
 * @param[in] num the number of expected tokens
 * @param[out] toks the (num long) array of found tokens, set to point to 
 *  the elements of args
 * @param[out] tokLens the (num long) array of token lengths
 * @returns PARSE_ARG_NUM_ERR if incorrect number of tokens, PARSE_OK
 *  otherwise
*/
static uart_parseStatus getArgTokens(const char* args, size_t len, 
                                     size_t num,
                                     const char** toks, size_t* tokLens) {
    assert(len == 0 || args != NULL);
    assert(num == 0 || (toks != NULL && tokLens != NULL));
    // find args
    for (size_t i = 0; i < num; ++i) {
        toks[i] = strn_strntok(args, len, &tokLens[i], CMD_DELIMITERS);
        if (toks[i] == NULL) {
            return PARSE_ARG_NUM_ERR;
        }
        size_t skippedLen = toks[i] - args;
        args += skippedLen + tokLens[i];
        len -= skippedLen + tokLens[i];
    }

    // check if there are tokens left
    size_t tmp;
    if (strn_strntok(args, len, &tmp, CMD_DELIMITERS) != NULL) {
        return PARSE_ARG_NUM_ERR;
    }

    return PARSE_OK;
}

uart_parseStatus uart_parseBuffer(const uart_LineBuffer* lineBuffer,
                                  uart_measParams* uartParams,
                                  const char** msg){
    assert(lineBuffer != NULL && uartParams != NULL && msg != NULL);
    *msg = NULL;

    size_t cmdlen;
    const char* const cmdtok = strn_strntok(lineBuffer->buffer, lineBuffer->len,
                                       &cmdlen, CMD_DELIMITERS);
    if (cmdtok == NULL) {
        return PARSE_COMMAND_ERR;
    }

    // selecting the command
    const char* const argsBeg = cmdtok + cmdlen;
    size_t skippedLen = cmdtok - lineBuffer->buffer;
    size_t argsLen = lineBuffer->len - skippedLen - cmdlen;
    const uart_Command cmds[] = COMMANDS_STRUCT;
    for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); ++i) {
        // executing the matching command with the args
        if (strn_exactMatch(cmds[i].cmd, cmdtok, cmdlen)) {
            // argument tokens
            const size_t argnum = cmds[i].arg_num;
            size_t toklens[MAX_ARG_NUM];
            const char* toks[MAX_ARG_NUM];
            uart_parseStatus status 
                = getArgTokens(argsBeg, argsLen, argnum, toks, toklens);
            if (status != PARSE_OK) {
                return status;
            }

            // processing the tokens
            return cmds[i].parseArgFun(toks, toklens, uartParams, msg);
        }
    }
    // no command matches
    return PARSE_COMMAND_ERR;
}

// Parsing function definitions ===========================================
#define MAX_MSG_LEN 256 // static local buffers for return messages
static char msgBuf[MAX_MSG_LEN];

/**
 * @brief Sets msg to the help string
*/
uart_parseStatus uart_parseHelpCmd(const char* toks[MAX_ARG_NUM],
                                   size_t toklens[MAX_ARG_NUM],
                                   uart_measParams* uartParams,
                                   const char** msg) {
    (void)toks; (void)toklens; (void)uartParams;
    *msg = uart_getHelpStr();
    return PARSE_OK;
}

/** @brief Sets msg to a buffer with the current parameter values */
uart_parseStatus uart_parseGetparamsCmd(const char* toks[MAX_ARG_NUM],
                                        size_t toklens[MAX_ARG_NUM],
                                        uart_measParams* uartParams,
                                        const char** msg) {
    (void)toks; (void)toklens;
    size_t cursor = 0;
    cursor += addStrToBuf(&msgBuf[cursor], "Current value of parameters:");

    cursor += addStrToBuf(&msgBuf[cursor], "\r\n\t* numMeas: ");
    cursor += strn_utostrn(uartParams->numMeas, 
                      &msgBuf[cursor], MAX_MSG_LEN - cursor);

    cursor += addStrToBuf(&msgBuf[cursor], "\r\n\t* dataSize: ");
    cursor += strn_utostrn(uartParams->dataSize, 
                      &msgBuf[cursor], MAX_MSG_LEN - cursor);

    cursor += addStrToBuf(&msgBuf[cursor], "\r\n\t* direction: ");
    cursor += addStrToBuf(&msgBuf[cursor],
                          uart_measDirectionToStr(uartParams->direction));

    cursor += addStrToBuf(&msgBuf[cursor], "\r\n\t* m7 clk [MHz]: ");
    cursor += strn_utostrn(uartParams->clk_m7, 
                      &msgBuf[cursor], MAX_MSG_LEN - cursor);
    cursor += addStrToBuf(&msgBuf[cursor], ", m4 clk [MHz]: ");
    cursor += strn_utostrn(uartParams->clk_m4, 
                      &msgBuf[cursor], MAX_MSG_LEN - cursor);
    cursor += addStrToBuf(&msgBuf[cursor], "\r\n");

    msgBuf[cursor++] = '\0';
    assert(cursor <= MAX_MSG_LEN);

    *msg = msgBuf;
    return PARSE_OK;
}

/** @brief Sets the start field of the uartParams */
uart_parseStatus uart_parseStartCmd(const char* toks[MAX_ARG_NUM],
                                    size_t toklens[MAX_ARG_NUM],
                                    uart_measParams* uartParams,
                                    const char** msg) {
    (void)toks; (void)toklens;
    // signal start of meas
    uartParams->startMeas = true;
    *msg = NULL;
    return PARSE_OK;
}

/** @brief Parses the argument tokens, then sets the direction field of the
 *      uartParams */
uart_parseStatus uart_parseDirectionCmd(const char* toks[MAX_ARG_NUM],
                                        size_t toklens[MAX_ARG_NUM],
                                        uart_measParams* uartParams,
                                        const char** msg) {
    *msg = NULL;
    // string arg options
    if (strn_exactMatch("send", toks[0], toklens[0])
            || strn_exactMatch("s", toks[0], toklens[0])) {
        uartParams->direction = SEND;
    }
    else if (strn_exactMatch("receive", toks[0], toklens[0])
             || strn_exactMatch("r", toks[0], toklens[0])) {
        uartParams->direction = RECEIVE;
    }
    else {
        return PARSE_ARG_VAL_ERR;
    }

    return PARSE_OK;
}

/** @brief Parses the argument tokens, then sets the clk fields of the 
 *  uartParams */
uart_parseStatus uart_parseClkCmd(const char* toks[MAX_ARG_NUM],
                                  size_t toklens[MAX_ARG_NUM],
                                  uart_measParams* uartParams,
                                  const char** msg) {
    *msg = NULL;
    // convert the args
    uint32_t clks[2];
    for (size_t i = 0; i < 2; ++i) {
        if (!strn_strntou(toks[i], toklens[i], &clks[i])) {
            return PARSE_ARG_VAL_ERR;
        }
    }

    ClkErr err = ctrl_validateClks(clks[0], clks[1]);
    if (err == CLK_M7_ERR) {
        *msg = "Invalid m7 clk frequency\r\n";
        return PARSE_ARG_VAL_ERR;
    }
    else if (err == CLK_M4_ERR) {
        *msg = "Invalid m4 clk frequency\r\n";
        return PARSE_ARG_VAL_ERR;
    }
    else if (err != CLK_OK) {
        assert(false);
    }

    uartParams->clk_m7 = clks[0];
    uartParams->clk_m4 = clks[1];
    return PARSE_OK;
}

/** @brief Parses the argument tokens, then sets the numMeas field of the
 *  uartParams */
uart_parseStatus uart_parseRepeatCmd(const char* toks[MAX_ARG_NUM],
                                     size_t toklens[MAX_ARG_NUM],
                                     uart_measParams* uartParams,
                                     const char** msg) {
    *msg = NULL;
    // arg conversion from string
    uint32_t count;
    if (!strn_strntou(toks[0], toklens[0], &count)) {
        return PARSE_ARG_VAL_ERR;
    }

    // validating the argument
    if (REPETITION_UP_LIMIT < count) {
        *msg = "Repetition saturated to upper limit\r\n";
        count = REPETITION_UP_LIMIT;
    }
    uartParams->numMeas = count;
    return PARSE_OK;
}

/** @brief Parses the argument tokens, then sets the datasize field of 
 *  uartParams */
uart_parseStatus uart_parseDatasizeCmd(const char* toks[MAX_ARG_NUM],
                                       size_t toklens[MAX_ARG_NUM],
                                       uart_measParams* uartParams,
                                       const char** msg) {    
    *msg = NULL;
    // arg conversion from string
    uint32_t datasize;
    if (!strn_strntou(toks[0], toklens[0], &datasize)) {
        return PARSE_ARG_VAL_ERR;
    }

    if (DATASIZE_UP_LIMIT < datasize) {
        *msg = "Datasize saturated to upper limit\r\n";
        datasize = DATASIZE_UP_LIMIT;
    }
    else if (datasize < DATASIZE_LOW_LIMIT) {
        *msg = "Datasize saturated to lower limit\r\n";
        datasize = DATASIZE_LOW_LIMIT;
    }
    uartParams->dataSize = datasize;
    return PARSE_OK;
}
