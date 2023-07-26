#include <gtest/gtest.h>

extern "C" {
    #include "uart_state_machine.h"
}

class uartStateMachineFixture : public ::testing::Test{
public:
    uint32_t numMeas;
    uint32_t measDataSize;
    uart_measDirection measDirection;
    uartStateMachine stateMachine;

    uartStateMachineFixture();
};

uartStateMachineFixture::uartStateMachineFixture(){
    numMeas = 0;
    measDataSize = 0;
    measDirection = SEND; // invalid
    uart_resetSM(&stateMachine);
}

/* Idle -----------------------------------------------------------------*/
TEST_F(uartStateMachineFixture, idle_then_StartChar) {
    /* Start meas */
    EXPECT_EQ(uart_stateMachineStep('s', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
    /* State */
    EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
}

/* Num_of_meas_next -----------------------------------------------------*/
TEST_F(uartStateMachineFixture, num_of_meas_next_then_CR){
    /* Preparation */
    stateMachine.state = NUM_OF_MEAS_NEXT;

    /* Expectation*/
    /* CR input */
    EXPECT_EQ(uart_stateMachineStep('\r', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
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
        EXPECT_EQ(uart_stateMachineStep(((i&1U) == 1) ? '0' : '9', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
        /* State */
        EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
    }
}

/* Data_size_next -------------------------------------------------------*/
TEST_F(uartStateMachineFixture, data_size_next_then_0_and_9){
    /* Preparation */
    stateMachine.state = DATA_SIZE_NEXT;

    /* Expectation*/
    /* 0 and 9 digit as input*/
    for(uint8_t i = 0; i < MEAS_DATA_SIZE_STRING_LEN; ++i){
        /* Return value */
        EXPECT_EQ(uart_stateMachineStep(((i&1U) == 0) ? '0' : '9', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
        /* State */
        EXPECT_EQ(stateMachine.state, DATA_SIZE_NEXT);
    }
}

/* Complete input sequence ----------------------------------------------*/
TEST_F(uartStateMachineFixture, idle_then_complete_input){ 
    /* Preparation */
    char stringNumMeas[] = "256";
    char stringMeasData[] = "134";

    /* Expectation*/
    /* start char*/
    EXPECT_EQ(uart_stateMachineStep('r', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
    /* meas num */
    for(uint8_t i = 0; i < sizeof(stringNumMeas)-1; ++i){
        EXPECT_EQ(uart_stateMachineStep(stringNumMeas[i], &stateMachine, &numMeas, &measDataSize, &measDirection), false);
    }
    EXPECT_EQ(uart_stateMachineStep('\r', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
    /* meas data size */
    for(uint8_t i = 0; i < sizeof(stringNumMeas)-1; ++i){
        EXPECT_EQ(uart_stateMachineStep(stringMeasData[i], &stateMachine, &numMeas, &measDataSize, &measDirection), false);
    }
    /* CR, input ended and correct*/
    EXPECT_EQ(uart_stateMachineStep('\r', &stateMachine, &numMeas, &measDataSize, &measDirection), true);
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
        EXPECT_EQ(uart_stateMachineStep('a', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
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