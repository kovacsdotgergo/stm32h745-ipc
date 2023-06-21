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

TEST_F(uartStateMachineFixture, idle_then_notStartChar) {
    /* Start meas */
    EXPECT_EQ(uart_stateMachineStep('a', &stateMachine, &numMeas, &measDataSize), false);
    /* Ouptut */
    EXPECT_EQ(numMeas, 0);
    EXPECT_EQ(measDataSize, 0);
    /* State */
    EXPECT_EQ(stateMachine.state, IDLE);
}

TEST_F(uartStateMachineFixture, idle_then_StartChar) {
    /* Start meas */
    EXPECT_EQ(uart_stateMachineStep('s', &stateMachine, &numMeas, &measDataSize), false);
    /* Ouptut */
    EXPECT_EQ(numMeas, 0);
    EXPECT_EQ(measDataSize, 0);
    /* State */
    EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
}

/* Num_of_meas_next -----------------------------------------------------*/

TEST_F(uartStateMachineFixture, num_of_meas_next_then_invalidChar){
    /* Preparation */
    stateMachine.state = NUM_OF_MEAS_NEXT;

    /* Expectation*/
    /* Start meas*/
    EXPECT_EQ(uart_stateMachineStep('a', &stateMachine, &numMeas, &measDataSize), false);
    /* Ouptut */
    EXPECT_EQ(numMeas, 0);
    EXPECT_EQ(measDataSize, 0);
    /* State */
    EXPECT_EQ(stateMachine.state, IDLE);
}

TEST_F(uartStateMachineFixture, num_of_meas_next_then_0_4_5_9){
    /* Preparation */
    stateMachine.state = NUM_OF_MEAS_NEXT;

    /* Expectation*/
    /* Start meas*/
    EXPECT_EQ(uart_stateMachineStep('0', &stateMachine, &numMeas, &measDataSize), false);
    EXPECT_EQ(uart_stateMachineStep('4', &stateMachine, &numMeas, &measDataSize), false);
    EXPECT_EQ(uart_stateMachineStep('5', &stateMachine, &numMeas, &measDataSize), false);
    EXPECT_EQ(uart_stateMachineStep('9', &stateMachine, &numMeas, &measDataSize), false);
    /* Ouptut */
    EXPECT_EQ(numMeas, 0);
    EXPECT_EQ(measDataSize, 0);
    /* State */
    EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
    EXPECT_EQ(stateMachine.stringIndex, 4);
    EXPECT_EQ(stateMachine.stringNumMeas[0], '0');
    EXPECT_EQ(stateMachine.stringNumMeas[1], '4');
    EXPECT_EQ(stateMachine.stringNumMeas[2], '5');
    EXPECT_EQ(stateMachine.stringNumMeas[3], '9');
}

TEST_F(uartStateMachineFixture, num_of_meas_next_then_enter){
    /* Preparation */
    stateMachine.state = NUM_OF_MEAS_NEXT;
    stateMachine.stringIndex = 5;

    /* Expectation*/
    /* Start meas*/
    EXPECT_EQ(uart_stateMachineStep('\r', &stateMachine, &numMeas, &measDataSize), false);
    /* Ouptut */
    EXPECT_EQ(numMeas, 0);
    EXPECT_EQ(measDataSize, 0);
    /* State */
    EXPECT_EQ(stateMachine.state, DATA_SIZE_NEXT);
    EXPECT_EQ(stateMachine.stringIndex, 0);
}

TEST_F(uartStateMachineFixture, num_of_meas_next_then_filled){
    /* Preparation */
    stateMachine.state = NUM_OF_MEAS_NEXT;

    /* Expectation*/
    /* Start meas*/
    for(int i = 0; i < NUM_MEAS_STRING_LEN; ++i){
        EXPECT_EQ(uart_stateMachineStep('0', &stateMachine, &numMeas, &measDataSize), false);
    }
    /* Ouptut */
    EXPECT_EQ(numMeas, 0);
    EXPECT_EQ(measDataSize, 0);
    /* State */
    EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
    for(int i = 0; i < NUM_MEAS_STRING_LEN; ++i){
        EXPECT_EQ(stateMachine.stringNumMeas[i], '0');
    }
    EXPECT_EQ(stateMachine.stringIndex, NUM_MEAS_STRING_LEN - 1);
}

TEST_F(uartStateMachineFixture, num_of_meas_next_filled_then_num){
    /* Preparation */
    stateMachine.state = NUM_OF_MEAS_NEXT;

    /* Expectation*/
    /* Start meas*/
    for(int i = 0; i < NUM_MEAS_STRING_LEN; ++i){
        EXPECT_EQ(uart_stateMachineStep('9', &stateMachine, &numMeas, &measDataSize), false);
    }
    EXPECT_EQ(uart_stateMachineStep('1', &stateMachine, &numMeas, &measDataSize), false);
    /* Ouptut */
    EXPECT_EQ(numMeas, 0);
    EXPECT_EQ(measDataSize, 0);
    /* State */
    EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
    for(int i = 0; i < NUM_MEAS_STRING_LEN - 1; ++i){
        EXPECT_EQ(stateMachine.stringNumMeas[i], '9');
    }
    EXPECT_EQ(stateMachine.stringNumMeas[NUM_MEAS_STRING_LEN - 1], '1');
    EXPECT_EQ(stateMachine.stringIndex, NUM_MEAS_STRING_LEN - 1);
}

/* Data_size_next -------------------------------------------------------*/

TEST_F(uartStateMachineFixture, data_size_next_then_0_2_7_9){
    /* Preparation */
    stateMachine.state = DATA_SIZE_NEXT;

    /* Expectation*/
    /* Start meas*/
    EXPECT_EQ(uart_stateMachineStep('0', &stateMachine, &numMeas, &measDataSize), false);
    EXPECT_EQ(uart_stateMachineStep('2', &stateMachine, &numMeas, &measDataSize), false);
    EXPECT_EQ(uart_stateMachineStep('7', &stateMachine, &numMeas, &measDataSize), false);
    EXPECT_EQ(uart_stateMachineStep('9', &stateMachine, &numMeas, &measDataSize), false);
    /* Ouptut */
    EXPECT_EQ(numMeas, 0);
    EXPECT_EQ(measDataSize, 0);
    /* State */
    EXPECT_EQ(stateMachine.state, DATA_SIZE_NEXT);
    EXPECT_EQ(stateMachine.stringIndex, 4);
    EXPECT_EQ(stateMachine.stringMeasData[0], '0');
    EXPECT_EQ(stateMachine.stringMeasData[1], '2');
    EXPECT_EQ(stateMachine.stringMeasData[2], '7');
    EXPECT_EQ(stateMachine.stringMeasData[3], '9');
}

TEST_F(uartStateMachineFixture, data_size_next_then_enter){ // todo
    /* Preparation */
    stateMachine.state = DATA_SIZE_NEXT;
    stateMachine.stringIndex = 5;
    stateMachine.stringNumMeas[0] = '2';
    stateMachine.stringNumMeas[1] = '5';
    stateMachine.stringNumMeas[2] = '6';
    stateMachine.stringMeasData[0] = '1';
    stateMachine.stringMeasData[1] = '3';
    stateMachine.stringMeasData[2] = '4';

    /* Expectation*/
    /* Start meas*/
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

TEST_F(uartStateMachineFixture, data_size_next_then_filled){
    /* Preparation */
    stateMachine.state = DATA_SIZE_NEXT;

    /* Expectation*/
    /* Start meas*/
    for(int i = 0; i < MEAS_DATA_SIZE_STRING_LEN; ++i){
        EXPECT_EQ(uart_stateMachineStep('0', &stateMachine, &numMeas, &measDataSize), false);
    }
    /* Ouptut */
    EXPECT_EQ(numMeas, 0);
    EXPECT_EQ(measDataSize, 0);
    /* State */
    EXPECT_EQ(stateMachine.state, DATA_SIZE_NEXT);
    for(int i = 0; i < MEAS_DATA_SIZE_STRING_LEN; ++i){
        EXPECT_EQ(stateMachine.stringMeasData[i], '0');
    }
    EXPECT_EQ(stateMachine.stringIndex, MEAS_DATA_SIZE_STRING_LEN - 1);
}

TEST_F(uartStateMachineFixture, data_size_next_filled_then_num){
    /* Preparation */
    stateMachine.state = DATA_SIZE_NEXT;

    /* Expectation*/
    /* Start meas*/
    for(int i = 0; i < MEAS_DATA_SIZE_STRING_LEN; ++i){
        EXPECT_EQ(uart_stateMachineStep('9', &stateMachine, &numMeas, &measDataSize), false);
    }
    EXPECT_EQ(uart_stateMachineStep('1', &stateMachine, &numMeas, &measDataSize), false);
    /* Ouptut */
    EXPECT_EQ(numMeas, 0);
    EXPECT_EQ(measDataSize, 0);
    /* State */
    EXPECT_EQ(stateMachine.state, DATA_SIZE_NEXT);
    for(int i = 0; i < MEAS_DATA_SIZE_STRING_LEN - 1; ++i){
        EXPECT_EQ(stateMachine.stringMeasData[i], '9');
    }
    EXPECT_EQ(stateMachine.stringMeasData[MEAS_DATA_SIZE_STRING_LEN - 1], '1');
    EXPECT_EQ(stateMachine.stringIndex, MEAS_DATA_SIZE_STRING_LEN - 1);
}

/* All states return ----------------------------------------------------*/

TEST_F(uartStateMachineFixture, allStates_then_returnToIdle) {
    uartStates states[3] = {IDLE, NUM_OF_MEAS_NEXT, DATA_SIZE_NEXT};
    for (int j = 0; j < 3; ++j){
        stateMachine.state = states[j];
        stateMachine.stringIndex = 5;
        memset(stateMachine.stringNumMeas, 1, sizeof(stateMachine.stringNumMeas));
        memset(stateMachine.stringMeasData, 6, sizeof(stateMachine.stringMeasData));

        /* Start meas */
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