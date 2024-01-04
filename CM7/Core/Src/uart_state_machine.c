#include "uart_state_machine.h"

uart_BufferStatus uart_addCharToBuffer(char uartInput, uart_LineBuffer* const lineBuffer) {
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
    uint32_t n = 0;
    while (len--) {
        if (!isdigit(*str)) {
            return PARSE_ARG_VAL_ERR;
        }
        n = 10 * n + *(str++) - '0'; 
    }
    *res = n;
    return PARSE_OK;
}

/**
 * @brief Splits the not null-terminated string of args into the given num
 *  tokens
 * @param[in] args the string of the arguments
 * @param[in] len length of the args string
 * @param[in] num the number of expected tokens
 * @param[out] toks the (num long) array of found tokens
 * @param[out] tokLens the (num long) array of token lengths
 * @returns PARSE_ARG_NUM_ERR if incorrect number of tokens, PARSE_OK
 *  otherwise
*/
static uart_parseStatus getArgTokens(const char* args, size_t len, size_t num,
                                     char* toks, size_t* tokLens) {
    assert(num == 0 || (toks != NULL && tokLens != NULL && args != NULL));
    // find args
    for (size_t i = 0; i < num; ++i) {
        toks[i] = strntok(args, len, &tokLens[i], CMD_DELIMITERS);
        if (toks[i] == NULL) {
            return PARSE_ARG_NUM_ERR;
        }
        args += tokLens[i];
        len -= tokLens[i];
    }

    // check if there are tokens left
    size_t tmp;
    if (strntok(args, len, &tmp, CMD_DELIMITERS) != NULL) {
        return PARSE_ARG_NUM_ERR;
    }

    return PARSE_OK;
}

/**
 * @brief Parses the arguments of the 'clk' command and modifies the uartParams
 *  accordingly
*/
static uart_parseStatus parseClkCmd(const char* args, size_t len,
                                    uart_measParams* const uartParams) {
    // find args
    size_t divTokLens[3], tmp;
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

    uartParams->clk_div1 = divs[0];
    uartParams->clk_div2 = divs[1];
    uartParams->clk_div3 = divs[2];
    return PARSE_OK;
}

/**
 * @brief Parses the arguments of the 'direction' command and modifies the
 *  uartParams accordingly
*/
static uart_parseStatus parseDirectionCmd(const char* args, size_t len,
                                          uart_measParams* uartParams) {
    (void)args;
    (void)uartParams;
    return PARSE_OK;
}

/**
 * @brief Parses the arguments of the 'start' command and modifies the
 *  uartParams accordingly 
*/
static uart_parseStatus parseStartCmd(const char* args, size_t len,
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

/**
 * @brief returns if the char is in the null-terminated string
*/
static bool charInStr(char c, const char* str) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (c == str[i]) return true;
    }
    return false;
}

/**
 * @brief Static local function that splits a calculates the first token in
 *  a not null-terminated string
 * @param[in] str the string to split
 * @param[in] len the length of the input string
 * @param[out] tok_len the length of the output token
 * @param[in] delimiters the token delimiter characters in a null-terminated
 *  string
 * @returns NULL if no token found, or the pointer to the first element of
 *  the token
*/
static const char* strntok(const char* str, size_t len,
                           size_t* const tok_len, 
                           const char* delimiters){
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

uart_parseStatus uart_parseBuffer(const uart_LineBuffer* lineBuffer,
                                  uart_measParams* uartParams){
    size_t cmdlen;
    const char* const cmdtok = strntok(lineBuffer->buffer, lineBuffer->len,
                                       &cmdlen, CMD_DELIMITERS);
    if (cmdtok == NULL) {
        return PARSE_COMMAND_ERR;
    }

    // selecting the command
    const char* const argsBeg = lineBuffer->buffer + cmdlen;
    size_t argsLen = lineBuffer->len - cmdlen;
    if (strncmp(cmdtok, "clk", cmdlen) == 0) {
        return parseClkCmd(argsBeg, argsLen, uartParams);
    }
    else if (strncmp(cmdtok, "direction", cmdlen) == 0) {
        return parseDirectionCmd(argsBeg, argsLen, uartParams);
    }
    else {
        return PARSE_COMMAND_ERR;
    }
}

// reset all the used storage and state
void uart_resetSM(uartStateMachine *stateMachine){
    stateMachine->state = IDLE;
    memset(stateMachine->stringNumMeas, 0x00, NUM_MEAS_STRING_LEN);
    memset(stateMachine->stringMeasData, 0x00, MEAS_DATA_SIZE_STRING_LEN);
    stateMachine->stringIndex = 0;
}

// perform a state transition on the parameter state machine used for
//      handling the charachters received over uart
bool uart_stateMachineStep(char input, uartStateMachine* stateMachine,
        uint32_t* pNumMeas, uint32_t* pMeasDataSize,
        uart_measDirection* pMeasDirection){
    bool ret = false;

    switch(stateMachine->state){
    case IDLE:
        // start character indicating the direction
        if(input == 's' || input == 'r'){
            stateMachine->state = NUM_OF_MEAS_NEXT;
            stateMachine->direction = (input == 's') ? SEND : RECIEVE;
        }
        else{
            uart_resetSM(stateMachine);
        }
    break;
    case NUM_OF_MEAS_NEXT:
        if(isdigit(input)){
            stateMachine->stringNumMeas[stateMachine->stringIndex] = input;
            if (stateMachine->stringIndex < NUM_MEAS_STRING_LEN - 1){
                ++(stateMachine->stringIndex);
            }
        }
        else if(input == '\r'){
            stateMachine->state = DATA_SIZE_NEXT;
            stateMachine->stringIndex = 0;
        }
        else{
            uart_resetSM(stateMachine);
        }
    break;
    case DATA_SIZE_NEXT:
        if(isdigit(input)){
            stateMachine->stringMeasData[stateMachine->stringIndex] = input;
            if (stateMachine->stringIndex < MEAS_DATA_SIZE_STRING_LEN - 1){
                ++(stateMachine->stringIndex);
            }
        }
        else if(input == '\r'){ // the meas can be started
            *pNumMeas = atoi(stateMachine->stringNumMeas);
            *pMeasDataSize = atoi(stateMachine->stringMeasData);
            *pMeasDirection = stateMachine->direction;
            uart_resetSM(stateMachine);
            ret = true;
        }
        else{
            uart_resetSM(stateMachine);
        }
    break;
    default:
        uart_resetSM(stateMachine);
    break;
    }
    return ret;   
}
