#include "uart_commands.h"

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
    if (uartInput == PUTTY_SENDS_NEW_LINE) { // line end sequence
        *echo = PUTTY_PRINTS_NEW_LINE;
        return BUFFER_DONE;
    }
    if (uartInput == PUTTY_DEL) { // delete last char on backspace
        if (0 < lineBuffer->len) {
            --lineBuffer->len;
            echoBuf[0] = PUTTY_DEL;
            *echo = echoBuf;
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
                                  const uart_controlIf* controlFuns,
                                  const char** msg){
    assert(lineBuffer != NULL && controlFuns != NULL && msg != NULL);
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
            return cmds[i].parseArgFun(toks, toklens, controlFuns, msg);
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
                                   const uart_controlIf* controlFuns,
                                   const char** msg) {
    (void)toks; (void)toklens; (void)controlFuns;
    *msg = uart_getHelpStr();
    return PARSE_OK;
}

/** @brief Sets msg to a buffer with the current parameter values */
uart_parseStatus uart_parseGetparamsCmd(const char* toks[MAX_ARG_NUM],
                                        size_t toklens[MAX_ARG_NUM],
                                        const uart_controlIf* controlFuns,
                                        const char** msg) {
    (void)toks; (void)toklens;
    assert(controlFuns->getRepeat != NULL
           && controlFuns->getDataSize != NULL
           && controlFuns->getDirection != NULL
           && controlFuns->getClks != NULL
           && controlFuns->getMem != NULL);
    size_t cursor = 0;
    cursor += addStrToBuf(&msgBuf[cursor], "Current value of parameters:");
    // Print repetition count
    cursor += addStrToBuf(&msgBuf[cursor], "\r\n\t* repeat: ");
    cursor += strn_utostrn(controlFuns->getRepeat(), 
                      &msgBuf[cursor], MAX_MSG_LEN - cursor);
    // Print data size
    cursor += addStrToBuf(&msgBuf[cursor], "\r\n\t* dataSize: ");
    cursor += strn_utostrn(controlFuns->getDataSize(), 
                      &msgBuf[cursor], MAX_MSG_LEN - cursor);
    // Print direction
    cursor += addStrToBuf(&msgBuf[cursor], "\r\n\t* direction: ");
    cursor += addStrToBuf(&msgBuf[cursor],
                          params_measDirectionToStr(controlFuns->getDirection()));
    // Print used memory
    cursor += addStrToBuf(&msgBuf[cursor], "\r\n\t* memory: ");
    cursor += addStrToBuf(&msgBuf[cursor],
                          params_memToStr(controlFuns->getMem()));

    // Print clks
    uint32_t clkM7, clkM4;
    controlFuns->getClks(&clkM7, &clkM4);
    cursor += addStrToBuf(&msgBuf[cursor], "\r\n\t* m7 clk [Hz]: ");
    cursor += strn_utostrn(clkM7, &msgBuf[cursor], MAX_MSG_LEN - cursor);
    cursor += addStrToBuf(&msgBuf[cursor], ", m4 clk [Hz]: ");
    cursor += strn_utostrn(clkM4, &msgBuf[cursor], MAX_MSG_LEN - cursor);
    cursor += addStrToBuf(&msgBuf[cursor], "\r\n");

    msgBuf[cursor++] = '\0';
    assert(cursor <= MAX_MSG_LEN);

    *msg = msgBuf;
    return PARSE_OK;
}

/** @brief Sets the start field of the controlFuns */
uart_parseStatus uart_parseStartCmd(const char* toks[MAX_ARG_NUM],
                                    size_t toklens[MAX_ARG_NUM],
                                    const uart_controlIf* controlFuns,
                                    const char** msg) {
    (void)toks; (void)toklens;
    assert(controlFuns->setStartMeas != NULL);
    // signal start of meas
    controlFuns->setStartMeas();
    *msg = NULL;
    return PARSE_OK;
}

/** @brief Parses the argument tokens, then sets the direction field of the
 *      controlFuns */
uart_parseStatus uart_parseDirectionCmd(const char* toks[MAX_ARG_NUM],
                                        size_t toklens[MAX_ARG_NUM],
                                        const uart_controlIf* controlFuns,
                                        const char** msg) {
    assert(controlFuns->setDirection != NULL);
    *msg = NULL;
    // string arg options
    if (strn_exactMatch("send", toks[0], toklens[0])
            || strn_exactMatch("s", toks[0], toklens[0])) {
        bool success = controlFuns->setDirection(M7_SEND, msg);
        assert(success);
    }
    else if (strn_exactMatch("receive", toks[0], toklens[0])
             || strn_exactMatch("r", toks[0], toklens[0])) {
        bool success = controlFuns->setDirection(M7_RECEIVE, msg);
        assert(success);
    }
    else {
        return PARSE_ARG_VAL_ERR;
    }

    return PARSE_OK;
}

/** @brief Parses the argument tokens, then sets the clk fields of the 
 *  controlFuns */
uart_parseStatus uart_parseClkCmd(const char* toks[MAX_ARG_NUM],
                                  size_t toklens[MAX_ARG_NUM],
                                  const uart_controlIf* controlFuns,
                                  const char** msg) {
    assert(controlFuns->setClks != NULL);
    *msg = NULL;
    // convert the args
    uint32_t clks[2];
    for (size_t i = 0; i < 2; ++i) {
        if (!strn_strntou(toks[i], toklens[i], &clks[i])) {
            return PARSE_ARG_VAL_ERR;
        }
    }

    if (!controlFuns->setClks(clks[0], clks[1], msg)) {
        return PARSE_ARG_VAL_ERR;
    }
    return PARSE_OK;
}

/** @brief Parses the argument tokens, then sets the repeat field of the
 *  controlFuns */
uart_parseStatus uart_parseRepeatCmd(const char* toks[MAX_ARG_NUM],
                                     size_t toklens[MAX_ARG_NUM],
                                     const uart_controlIf* controlFuns,
                                     const char** msg) {
    assert(controlFuns->setRepeat != NULL);
    *msg = NULL;
    // arg conversion from string
    uint32_t count;
    if (!strn_strntou(toks[0], toklens[0], &count)) {
        return PARSE_ARG_VAL_ERR;
    }

    if(!controlFuns->setRepeat(count, msg)) {
        return PARSE_ARG_VAL_ERR;
    }
    return PARSE_OK;
}

/** @brief Parses the argument tokens, then sets the datasize field of 
 *  controlFuns */
uart_parseStatus uart_parseDatasizeCmd(const char* toks[MAX_ARG_NUM],
                                       size_t toklens[MAX_ARG_NUM],
                                       const uart_controlIf* controlFuns,
                                       const char** msg) {    
    assert(controlFuns->setDataSize != NULL);
    *msg = NULL;
    // arg conversion from string
    uint32_t datasize;
    if (!strn_strntou(toks[0], toklens[0], &datasize)) {
        return PARSE_ARG_VAL_ERR;
    }

    if (!controlFuns->setDataSize(datasize, msg)) {
        return PARSE_ARG_VAL_ERR;
    }
    return PARSE_OK;
}

/** @brief Parses the argument tokens, then calls the setMem field of
 *  controlFuns */
uart_parseStatus uart_parseMemCmd(const char* toks[MAX_ARG_NUM],
                                  size_t toklens[MAX_ARG_NUM],
                                  const uart_controlIf* controlFuns,
                                  const char** msg) {    
    assert(controlFuns->setMem != NULL);
    *msg = NULL;
    // arg conversion from string
    if (strn_exactMatch("D1", toks[0], toklens[0])) {
        bool success = controlFuns->setMem(MEM_D1, msg);
        assert(success);
    }
    else if (strn_exactMatch("D2", toks[0], toklens[0])) {
        bool success = controlFuns->setMem(MEM_D2, msg);
        assert(success);
    }
    else if (strn_exactMatch("D3", toks[0], toklens[0])) {
        bool success = controlFuns->setMem(MEM_D3, msg);
        assert(success);
    }
    else {
        return PARSE_ARG_VAL_ERR;
    }

    return PARSE_OK;
}
