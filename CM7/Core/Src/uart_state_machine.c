#include "uart_state_machine.h"

bool uartStateMachineStep(char input, uint32_t* pNumMeas, uint32_t* pMeasDataSize){
    static uartStates state = IDLE;
    bool retval = false;
    static char stringNumMeas[NUM_MEAS_STRING_LEN];
    static char iNumMeas = 0;
    static char stringMeasData[MEAS_DATA_SIZE_STRING_LEN];
    static char iMeasData = 0;

    if(state == IDLE && input == 's'){
        state = NUM_OF_MEAS;
    }
    else if (state == NUM_OF_MEAS && input >= '0' && input <= '9'){
        stringNumMeas[iNumMeas] = input;
    }
    else if (state == NUM_OF_MEAS && input == '\r' || input == '\n'){

    }
           
}