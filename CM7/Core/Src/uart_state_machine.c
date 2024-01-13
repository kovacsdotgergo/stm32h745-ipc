#include "uart_state_machine.h"

uart_BufferStatus uart_addCharToBuffer(char uartInput, uart_LineBuffer* lineBuffer) {
    assert(lineBuffer != NULL);
    // todo could add backspace support
    if (uartInput == '\r') {
        return BUFFER_DONE;
    }
    if (LINE_BUFFER_LEN <= lineBuffer->len) {
        return BUFFER_OVERFLOW;
    }
    lineBuffer->buffer[lineBuffer->len++] = uartInput;
    return BUFFER_OK;
}

/**
 * @brief Converts a not null-terminated string containing digits to uint 
 * @returns PARSE_ARG_VAL_ERR if found not digit PARSE_OK otherwise
*/
static uart_parseStatus strntou(const char* str, size_t len, uint32_t* res) {
    // todo input greater than 2**32 e.g. assert len < 10, but return some err
    assert(len != 0 && str != NULL);
    assert(res != NULL);
    uint32_t n = 0;
    while (len--) {
        if (!isdigit((unsigned char)*str)) {
            return PARSE_ARG_VAL_ERR;
        }
        n = 10 * n + *(str++) - '0'; 
    }
    *res = n;
    return PARSE_OK;
}

/**
 * @brief returns if the char is in the null-terminated string
*/
static bool charInStr(char c, const char* str) {
    assert(str != NULL);
    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (c == str[i]) return true;
    }
    return false;
}

/**
 * @brief Static local function that splits a not null terminated string by
 *  calculating the beginning of the first token
 * @param[in] str the string to split
 * @param[in] len the length of the input string
 * @param[out] tok_len the length of the output token
 * @param[in] delimiters the token delimiter characters in a null-terminated
 *  string
 * @returns NULL if no token found, or the pointer to the first element of
 *  the token
*/
static const char* strntok(const char* str, size_t len,
                           size_t* tok_len, 
                           const char* delimiters){
    assert(delimiters != NULL && tok_len != NULL);
    assert(len == 0 || str != NULL);
    size_t i = 0;
    // skip delimiters
    while(i < len && charInStr(str[i], delimiters)) { ++i; }
    const size_t begIdx = i;
    // find end of token
    while(i < len && !charInStr(str[i], delimiters)) { ++i; }
    const size_t endIdx = i;
    
    *tok_len = endIdx - begIdx;
    if (*tok_len == 0) {
        return NULL;
    }
    else {
        return str + begIdx;
    }
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
        toks[i] = strntok(args, len, &tokLens[i], CMD_DELIMITERS);
        if (toks[i] == NULL) {
            return PARSE_ARG_NUM_ERR;
        }
        size_t skippedLen = toks[i] - args;
        args += skippedLen + tokLens[i];
        len -= skippedLen + tokLens[i];
    }

    // check if there are tokens left
    size_t tmp;
    if (strntok(args, len, &tmp, CMD_DELIMITERS) != NULL) {
        return PARSE_ARG_NUM_ERR;
    }

    return PARSE_OK;
}

uart_parseStatus uart_parseClkCmd(const char* args, size_t len,
                                    uart_measParams* const uartParams) {
    // find args
    size_t divTokLens[3];
    const char* divToks[3];
    uart_parseStatus status = getArgTokens(args, len, 3, divToks, divTokLens);
    if (status != PARSE_OK) {
        return status;
    }

    // convert the args
    uint32_t divs[3];
    const uint32_t limits[3] = DIV_LIMITS;
    for (size_t i = 0; i < 3; ++i) {
        if (strntou(divToks[i], divTokLens[i], &divs[i]) != PARSE_OK) {
            return PARSE_ARG_VAL_ERR;
        }
        // validating the arg values
        if (limits[i] < divs[i]) {
            // todo print message about saturating argument or signal it somehow
            divs[i] = limits[i];
        }
    }

    uartParams->clk_div1 = (uint8_t)divs[0];
    uartParams->clk_div2 = (uint8_t)divs[1];
    uartParams->clk_div3 = (uint8_t)divs[2];
    return PARSE_OK;
}

uart_parseStatus uart_parseDirectionCmd(const char* args, size_t len,
                                          uart_measParams* uartParams) {
    // find the single expected argument
    const char* dirTok;
    size_t dirTokLen;
    uart_parseStatus status = getArgTokens(args, len, 1, &dirTok, &dirTokLen);
    if (status != PARSE_OK) {
        return status;
    }

    // string arg options
    if (strncmp(dirTok, "send", dirTokLen) == 0
        || strncmp(dirTok, "s", dirTokLen) == 0) {
        uartParams->direction = SEND;
    }
    else if (strncmp(dirTok, "receive", dirTokLen) == 0
             || strncmp(dirTok, "r", dirTokLen) == 0) {
        uartParams->direction = RECEIVE;
    }
    else {
        return PARSE_ARG_VAL_ERR;
    }

    return PARSE_OK;
}

uart_parseStatus uart_parseStartCmd(const char* args, size_t len,
                                      uart_measParams* uartParams) {
    // check if there are arguments, zero needed
    uart_parseStatus status = getArgTokens(args, len, 0, NULL, NULL);
    if (status != PARSE_OK) {
        return status;
    }

    // signal start of meas
    uartParams->startMeas = true;
    return PARSE_OK;
}

uart_parseStatus uart_parseRepeatCmd(const char* args, size_t len,
                                      uart_measParams* uartParams) {
    // find the single expected token
    const char* repeatTok;
    size_t repeatTokLen;
    uart_parseStatus status = getArgTokens(args, len, 1, &repeatTok, &repeatTokLen);
    if (status != PARSE_OK) {
        return status;
    }

    // arg conversion from string
    uint32_t count;
    status = strntou(repeatTok, repeatTokLen, &count);
    if (status != PARSE_OK) {
        return status;
    }

    // validating the argument
    if (REPETITION_LIMIT < count) {
        // todo warn about saturation
        count = REPETITION_LIMIT;
    }

    uartParams->numMeas = count;
    return PARSE_OK;
}

uart_parseStatus uart_parseDatasizeCmd(const char* args, size_t len,
                                         uart_measParams* uartParams) {
    // find the single expected arg
    const char* sizeTok;
    size_t sizeTokLen;
    uart_parseStatus status = getArgTokens(args, len, 1, &sizeTok, &sizeTokLen);
    if (status != PARSE_OK) {
        return status;
    }
    
    // arg conversion from string
    uint32_t datasize;
    status = strntou(sizeTok, sizeTokLen, &datasize);
    if (status != PARSE_OK) {
        return status;
    }

    if (DATASIZE_LIMIT < datasize) {
        // todo wand about saturation
        datasize = DATASIZE_LIMIT;
    }
    uartParams->dataSize = datasize;
    return PARSE_OK;
}

uart_parseStatus uart_parseBuffer(const uart_LineBuffer* lineBuffer,
                                  uart_measParams* uartParams){
    size_t cmdlen;
    const char* const cmdtok = strntok(lineBuffer->buffer, lineBuffer->len,
                                       &cmdlen, CMD_DELIMITERS);
    if (cmdtok == NULL) {
        return PARSE_COMMAND_ERR;
    }

    // selecting the command
    const char* const argsBeg = cmdtok + cmdlen;
    size_t skippedLen = cmdtok - lineBuffer->buffer;
    size_t argsLen = lineBuffer->len - skippedLen - cmdlen;
    const uart_Command cmds[] = COMMANDS;
    for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); ++i) {
        // executing the matching command with the args
        if (strncmp(cmdtok, cmds[i].cmd, cmdlen) == 0) {
            return cmds[i].parseArgFun(argsBeg, argsLen, uartParams);
        }
    }
    // no command matches
    return PARSE_COMMAND_ERR;
}
