#include "uart_state_machine.h"

static inline bool uart_isdigit(char ch){
    return ch > '0' && ch < '9';
}

bool uartStateMachineStep(char input, uint32_t* pNumMeas, uint32_t* pMeasDataSize){
    static uartStates state = IDLE;
    bool ret = false;
    static char stringNumMeas[NUM_MEAS_STRING_LEN] = {0};
    static uint8_t iNumMeas = 0;
    static char stringMeasData[MEAS_DATA_SIZE_STRING_LEN] = {0};
    static uint8_t iMeasData = 0;

    if (state == IDLE && input == 's'){
        state = NUM_OF_MEAS_NEXT;
    }
    else if (state == NUM_OF_MEAS_NEXT && uart_isdigit(input)){
        stringNumMeas[iNumMeas] = input;
        if (iNumMeas < NUM_MEAS_STRING_LEN - 1){
            ++iNumMeas;
        }
    }
    else if (state == NUM_OF_MEAS_NEXT && input == '\r'){
        state = DATA_SIZE_NEXT;
    }
    else if (state == DATA_SIZE_NEXT && uart_isdigit(input)){
        stringMeasData[iMeasData] = input;
        if (iMeasData < DATA_SIZE_NEXT - 1){
            ++iMeasData;
        }
    }
    else if (state == DATA_SIZE_NEXT && input == '\r'){
        state = IDLE;
        *pNumMeas = atoi(stringNumMeas);
        *pMeasDataSize = atoi(stringMeasData);
        ret = true;
    }
    else{
        state = IDLE;
        memset(stringNumMeas, 0x00, sizeof(stringNumMeas));
        memset(stringMeasData, 0x00, sizeof(stringMeasData));
        iNumMeas = 0;
        iMeasData = 0;
    }
    return ret;   
}
