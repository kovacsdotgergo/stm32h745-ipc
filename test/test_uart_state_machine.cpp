#include <gtest/gtest.h>

extern "C" {
    #include "uart_state_machine.h"
}

class uartStateMachineFixture : public ::testing::Test{
public:
    uint32_t numMeas;
    uint32_t measDataSize;
    uartStateMachine stateMachine;

    uartStateMachineFixture();
};

uartStateMachineFixture::uartStateMachineFixture(){
    numMeas = 0;
    measDataSize = 0;
    resetSM(&stateMachine);
}

/* Idle -----------------------------------------------------------------*/
TEST_F(uartStateMachineFixture, idle_then_StartChar) {
    /* Start meas */
    EXPECT_EQ(uart_stateMachineStep('s', &stateMachine, &numMeas, &measDataSize), false);
    /* State */
    EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
}

/* Num_of_meas_next -----------------------------------------------------*/

TEST_F(uartStateMachineFixture, num_of_meas_next_then_CR){
    /* Preparation */
    stateMachine.state = NUM_OF_MEAS_NEXT;

    /* Expectation*/
    /* CR input */
    EXPECT_EQ(uart_stateMachineStep('\r', &stateMachine, &numMeas, &measDataSize), false);
    /* State */
    EXPECT_EQ(stateMachine.state, DATA_SIZE_NEXT);
}

TEST_F(uartStateMachineFixture, num_of_meas_next_then_digit){
    /* Preparation */
    stateMachine.state = NUM_OF_MEAS_NEXT;

    /* Expectation*/
    /* 0 and 9 digit as input*/
    for(int i = 0; i < NUM_MEAS_STRING_LEN; ++i){
        /* Return value */
        EXPECT_EQ(uart_stateMachineStep('0', &stateMachine, &numMeas, &measDataSize), false);
        /* State */
        EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
    }
    for(int i = 0; i < NUM_MEAS_STRING_LEN; ++i){
        /* Return value */
        EXPECT_EQ(uart_stateMachineStep('9', &stateMachine, &numMeas, &measDataSize), false);
        /* State */
        EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
    }
}

/* Data_size_next -------------------------------------------------------*/
TEST_F(uartStateMachineFixture, data_size_next_then_0_2_7_9){
    /* Preparation */
    stateMachine.state = DATA_SIZE_NEXT;

    /* Expectation*/
    /* 0 and 9 digit as input*/
    for(int i = 0; i < MEAS_DATA_SIZE_STRING_LEN; ++i){
        /* Return value */
        EXPECT_EQ(uart_stateMachineStep('0', &stateMachine, &numMeas, &measDataSize), false);
        /* State */
        EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
    }
    for(int i = 0; i < MEAS_DATA_SIZE_STRING_LEN; ++i){
        /* Return value */
        EXPECT_EQ(uart_stateMachineStep('9', &stateMachine, &numMeas, &measDataSize), false);
        /* State */
        EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
    }
}

/* Complete input sequence ----------------------------------------------*/
// TODO
TEST_F(uartStateMachineFixture, idle_then_complete_input){ 
    /* Preparation */
    char stringNumMeas[] = "256";
    char stringMeasData[] = "134";

    /* Expectation*/
    EXPECT_EQ(uart_stateMachineStep('r', &stateMachine, &numMeas, &measDataSize, &pMeasDirection), false);
    /* CR, input ended and correct*/
    EXPECT_EQ(uart_stateMachineStep('\r', &stateMachine, &numMeas, &measDataSize), true);
    /* Ouptut */
    EXPECT_EQ(numMeas, 256);
    EXPECT_EQ(measDataSize, 134);
    /* State */
    EXPECT_EQ(stateMachine.state, IDLE);
    EXPECT_EQ(stateMachine.stringIndex, 0);
    for(int i = 0; i < NUM_MEAS_STRING_LEN; ++i){
        EXPECT_EQ(stateMachine.stringMeasData[i], 0);
    }
    for(int i = 0; i< MEAS_DATA_SIZE_STRING_LEN; ++i){
        EXPECT_EQ(stateMachine.stringNumMeas[i], 0);
    }
}

/* All states return ----------------------------------------------------*/

TEST_F(uartStateMachineFixture, allStates_then_returnToIdle) {
    uartStates states[3] = {IDLE, NUM_OF_MEAS_NEXT, DATA_SIZE_NEXT};
    for (int j = 0; j < 3; ++j){
        stateMachine.state = states[j];
        stateMachine.stringIndex = 5;
        memset(stateMachine.stringNumMeas, 1, sizeof(stateMachine.stringNumMeas));
        memset(stateMachine.stringMeasData, 6, sizeof(stateMachine.stringMeasData));

        /* Invalid input */
        EXPECT_EQ(uart_stateMachineStep('a', &stateMachine, &numMeas, &measDataSize), false);
        /* Ouptut */
        EXPECT_EQ(numMeas, 0);
        EXPECT_EQ(measDataSize, 0);
        /* State */
        EXPECT_EQ(stateMachine.state, IDLE);
        EXPECT_EQ(stateMachine.stringIndex, 0);
        for(int i = 0; i < NUM_MEAS_STRING_LEN; ++i){
            EXPECT_EQ(stateMachine.stringMeasData[i], 0);
        }
        for(int i = 0; i< MEAS_DATA_SIZE_STRING_LEN; ++i){
            EXPECT_EQ(stateMachine.stringNumMeas[i], 0);
        }
    }
}