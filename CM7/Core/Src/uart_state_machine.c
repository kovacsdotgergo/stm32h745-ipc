#include "uart_state_machine.h"

static inline bool uart_isdigit(char ch){
    return ch >= '0' && ch <= '9';
}

void resetSM(uartStateMachine *stateMachine){
    stateMachine->state = IDLE;
    memset(stateMachine->stringNumMeas, 0x00, NUM_MEAS_STRING_LEN);
    memset(stateMachine->stringMeasData, 0x00, MEAS_DATA_SIZE_STRING_LEN);
    stateMachine->stringIndex = 0;
}

bool uartStateMachineStep(char input, uartStateMachine* stateMachine,
         uint32_t* pNumMeas, uint32_t* pMeasDataSize){
    bool ret = false;

    switch(stateMachine->state){
    case IDLE:
        if(input == 's'){
            stateMachine->state = NUM_OF_MEAS_NEXT;
        }
        else{
            resetSM(stateMachine);
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
            resetSM(stateMachine);
        }
    break;
    case DATA_SIZE_NEXT:
        if(uart_isdigit(input)){
            stateMachine->stringMeasData[stateMachine->stringIndex] = input;
            if (stateMachine->stringIndex < MEAS_DATA_SIZE_STRING_LEN - 1){
                ++(stateMachine->stringIndex);
            }
        }
        else if(input == '\r'){
            *pNumMeas = atoi(stateMachine->stringNumMeas);
            *pMeasDataSize = atoi(stateMachine->stringMeasData);
            resetSM(stateMachine);
            ret = true;
        }
        else{
            resetSM(stateMachine);
        }
    break;
    default:
        resetSM(stateMachine);
    break;
    }
    return ret;   
}
