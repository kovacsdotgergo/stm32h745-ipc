#include <gtest/gtest.h>

extern "C" {
    #include "uart_state_machine.c"
}

// class uartStateMachineFixture : public ::testing::Test{
// public:
//     uint32_t numMeas;
//     uint32_t measDataSize;
//     uart_measDirection measDirection;
//     uartStateMachine stateMachine;

//     uartStateMachineFixture();
// };

// uartStateMachineFixture::uartStateMachineFixture(){
//     numMeas = 0;
//     measDataSize = 0;
//     measDirection = SEND; // invalid
//     uart_resetSM(&stateMachine);
// }

// uartLineBufferTest =====================================================
TEST(uartLineBufferTest, CorrectString) {
    const char inString[] = "  input  ";
    const char len = sizeof(inString);
    ASSERT_LT(len, LINE_BUFFER_LEN);
    uart_LineBuffer lineBuffer = {
        .buffer = {0},
        .len = 0,
    };

    for (size_t i = 0; i < len; ++i) {
        auto ret = uart_addCharToBuffer(inString[i], &lineBuffer);
        EXPECT_EQ(BUFFER_OK, ret);
        EXPECT_EQ(i + 1, lineBuffer.len);
    }
    auto ret = uart_addCharToBuffer('\r', &lineBuffer);

    EXPECT_EQ(BUFFER_DONE, ret);
    EXPECT_EQ(len, lineBuffer.len);
    EXPECT_EQ(0, memcmp(inString, lineBuffer.buffer, len));
}

TEST(uartLineBufferTest, Overflow) {
    uart_LineBuffer lineBuffer = {
        .buffer = {0},
        .len = LINE_BUFFER_LEN,
    };
    // testing for a uniformly filled string
    auto ret = uart_addCharToBuffer('a', &lineBuffer);
    EXPECT_EQ(BUFFER_OVERFLOW, ret);
}

// uartLineParserTest =====================================================
TEST(uartLineParserTest, EmptyLine) {
    uart_LineBuffer lineBuffer = {
        .buffer = " \t\t ",
        .len = 4,
    };
    uart_measParams measParams;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams);
    EXPECT_EQ(PARSE_COMMAND_ERR, ret);
}

TEST(uartLineParserTest, CmdAtTheEnd) {
    
}

TEST(uartLineParserTest, CmdInTheMiddle) {

}

TEST(uartLineParserTest, CmdAtTheBeginning) {

}

TEST(uartLineParserTest, InvalidCmd) {

}

TEST(uartLineParserTest, WrongNumOfArgs) {

}

TEST(uartLineParserTest, InvalidArg) {

}

TEST(uartLineParserTest, ValidArg) {

}

// strntouTest ============================================================
TEST(strntouTest, NotDigitError) {
    const char input[] = "10a98";
    size_t len = sizeof(input) - 1;
    uint32_t res;

    uart_parseStatus ret = strntou(input, len, &res);

    EXPECT_EQ(PARSE_ARG_VAL_ERR, ret);
}

TEST(strntouTest, Zero) {
    const char input[] = "0000";
    size_t len = sizeof(input) - 1;
    uint32_t res;

    uart_parseStatus ret = strntou(input, len, &res);

    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(0, res);
}

TEST(strntouTest, BigNum) {
    const char input[] = "4294967295"; // 2**32 - 1
    size_t len = sizeof(input) - 1;
    uint32_t res;

    uart_parseStatus ret = strntou(input, len, &res);

    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(4294967295U, res);
}

// charInStrTest ==========================================================
TEST(charInStrTest, charIsInString) {
    const char c = ' ';
    const char str[] = " \n";

    bool ret = charInStr(c, str);

    EXPECT_EQ(true, ret);
}

TEST(charInStrTest, charIsNotInString) {
    const char c = 'a';
    const char str[] = "cd";
    
    bool ret = charInStr(c, str);

    EXPECT_EQ(false, ret);
}

TEST(charInStrTest, charIsNull) {
    const char c = '\0';
    const char str[] = " ";
    
    bool ret = charInStr(c, str);

    EXPECT_EQ(false, ret);
}

// State machine ==========================================================
/* Idle -----------------------------------------------------------------*/
// TEST_F(uartStateMachineFixture, idle_then_StartChar) {
//     /* Start meas */
//     EXPECT_EQ(uart_stateMachineStep('s', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
//     /* State */
//     EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
// }

// /* Num_of_meas_next -----------------------------------------------------*/
// TEST_F(uartStateMachineFixture, num_of_meas_next_then_CR){
//     /* Preparation */
//     stateMachine.state = NUM_OF_MEAS_NEXT;

//     /* Expectation*/
//     /* CR input */
//     EXPECT_EQ(uart_stateMachineStep('\r', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
//     /* State */
//     EXPECT_EQ(stateMachine.state, DATA_SIZE_NEXT);
// }

// TEST_F(uartStateMachineFixture, num_of_meas_next_then_digit){
//     /* Preparation */
//     stateMachine.state = NUM_OF_MEAS_NEXT;

//     /* Expectation*/
//     /* 0 and 9 digit as input*/
//     for(int i = 0; i < NUM_MEAS_STRING_LEN; ++i){
//         /* Return value */
//         EXPECT_EQ(uart_stateMachineStep(((i&1U) == 1) ? '0' : '9', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
//         /* State */
//         EXPECT_EQ(stateMachine.state, NUM_OF_MEAS_NEXT);
//     }
// }

// /* Data_size_next -------------------------------------------------------*/
// TEST_F(uartStateMachineFixture, data_size_next_then_0_and_9){
//     /* Preparation */
//     stateMachine.state = DATA_SIZE_NEXT;

//     /* Expectation*/
//     /* 0 and 9 digit as input*/
//     for(uint8_t i = 0; i < MEAS_DATA_SIZE_STRING_LEN; ++i){
//         /* Return value */
//         EXPECT_EQ(uart_stateMachineStep(((i&1U) == 0) ? '0' : '9', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
//         /* State */
//         EXPECT_EQ(stateMachine.state, DATA_SIZE_NEXT);
//     }
// }

// /* Complete input sequence ----------------------------------------------*/
// TEST_F(uartStateMachineFixture, idle_then_complete_input){ 
//     /* Preparation */
//     char stringNumMeas[] = "256";
//     char stringMeasData[] = "134";

//     /* Expectation*/
//     /* start char*/
//     EXPECT_EQ(uart_stateMachineStep('r', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
//     /* meas num */
//     for(uint8_t i = 0; i < sizeof(stringNumMeas)-1; ++i){
//         EXPECT_EQ(uart_stateMachineStep(stringNumMeas[i], &stateMachine, &numMeas, &measDataSize, &measDirection), false);
//     }
//     EXPECT_EQ(uart_stateMachineStep('\r', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
//     /* meas data size */
//     for(uint8_t i = 0; i < sizeof(stringNumMeas)-1; ++i){
//         EXPECT_EQ(uart_stateMachineStep(stringMeasData[i], &stateMachine, &numMeas, &measDataSize, &measDirection), false);
//     }
//     /* CR, input ended and correct*/
//     EXPECT_EQ(uart_stateMachineStep('\r', &stateMachine, &numMeas, &measDataSize, &measDirection), true);
//     /* Ouptut */
//     EXPECT_EQ(numMeas, 256);
//     EXPECT_EQ(measDataSize, 134);
//     /* State */
//     EXPECT_EQ(stateMachine.state, IDLE);
//     EXPECT_EQ(stateMachine.stringIndex, 0);
//     for(int i = 0; i < NUM_MEAS_STRING_LEN; ++i){
//         EXPECT_EQ(stateMachine.stringMeasData[i], 0);
//     }
//     for(int i = 0; i< MEAS_DATA_SIZE_STRING_LEN; ++i){
//         EXPECT_EQ(stateMachine.stringNumMeas[i], 0);
//     }
// }

// /* All states return ----------------------------------------------------*/
// TEST_F(uartStateMachineFixture, allStates_then_returnToIdle) {
//     uartStates states[3] = {IDLE, NUM_OF_MEAS_NEXT, DATA_SIZE_NEXT};
//     for (int j = 0; j < 3; ++j){
//         stateMachine.state = states[j];
//         stateMachine.stringIndex = 5;
//         memset(stateMachine.stringNumMeas, 1, sizeof(stateMachine.stringNumMeas));
//         memset(stateMachine.stringMeasData, 6, sizeof(stateMachine.stringMeasData));

//         /* Invalid input */
//         EXPECT_EQ(uart_stateMachineStep('a', &stateMachine, &numMeas, &measDataSize, &measDirection), false);
//         /* Ouptut */
//         EXPECT_EQ(numMeas, 0);
//         EXPECT_EQ(measDataSize, 0);
//         /* State */
//         EXPECT_EQ(stateMachine.state, IDLE);
//         EXPECT_EQ(stateMachine.stringIndex, 0);
//         for(int i = 0; i < NUM_MEAS_STRING_LEN; ++i){
//             EXPECT_EQ(stateMachine.stringMeasData[i], 0);
//         }
//         for(int i = 0; i< MEAS_DATA_SIZE_STRING_LEN; ++i){
//             EXPECT_EQ(stateMachine.stringNumMeas[i], 0);
//         }
//     }
// }