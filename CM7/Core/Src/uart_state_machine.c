#include "uart_state_machine.h"

/* Static local function returning if the char is a digit*/
static inline bool uart_isdigit(char ch){
    return ch >= '0' && ch <= '9';
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
        if(uart_isdigit(input)){
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
        if(uart_isdigit(input)){
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
