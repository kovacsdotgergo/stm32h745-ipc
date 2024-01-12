#include <gtest/gtest.h>

extern "C" {
    #include "uart_state_machine.c"
}

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
    const char cmd[] = " \t\t "; // only separators
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = sizeof(cmd) - 1,
    };
    for (size_t i = 0; i < lineBuffer.len; ++i) {
        lineBuffer.buffer[i] = cmd[i];
    }
    uart_measParams measParams;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams);
    EXPECT_EQ(PARSE_COMMAND_ERR, ret);
}

TEST(uartLineParserTest, CmdAtTheEnd) {
    const char cmd[] = "clk";
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = LINE_BUFFER_LEN,
    };
    size_t i;
    // cmd at the end of the buffer
    for (i = 0; i < LINE_BUFFER_LEN - (sizeof(cmd) - 1); ++i) {
        lineBuffer.buffer[i] = ' ';
    }
    for (size_t j = 0; i < LINE_BUFFER_LEN; ++i, ++j) {
        lineBuffer.buffer[i] = cmd[j];
    }
    uart_measParams measParams;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams);
    EXPECT_EQ(PARSE_ARG_NUM_ERR, ret);
}

TEST(uartLineParserTest, InvalidCmd) {
    const char cmd[] = "abc";
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = sizeof(cmd) - 1,
    };
    for (size_t i = 0; i < lineBuffer.len; ++i) {
        lineBuffer.buffer[i] = cmd[i];
    }
    uart_measParams measParams;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams);
    EXPECT_EQ(PARSE_COMMAND_ERR, ret);
}

TEST(uartLineParserTest, WrongNumOfArgs) {
    // also whitespace before cmd
    const char cmd[] = " \t   \t clk 1 10   \t \t\t ";
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = sizeof(cmd) - 1,
    };
    for (size_t i = 0; i < lineBuffer.len; ++i) {
        lineBuffer.buffer[i] = cmd[i];
    }
    uart_measParams measParams;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams);
    EXPECT_EQ(PARSE_ARG_NUM_ERR, ret);
}

TEST(uartLineParserTest, InvalidArg) {
    const char cmd[] = " \t direction \t\tse\0nd";
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = sizeof(cmd) - 1,
    };
    for (size_t i = 0; i < lineBuffer.len; ++i) {
        lineBuffer.buffer[i] = cmd[i];
    }
    uart_measParams measParams;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams);
    EXPECT_EQ(PARSE_ARG_VAL_ERR, ret);
}

// uartLineParserCorrectCmdsTest ==========================================
class uartLineParserCorrectCmdsTest : public ::testing::Test {
public:
    uart_LineBuffer lineBuffer{};
    uart_measParams measParams{
        .numMeas = 1U,
        .dataSize = 2U,
        .direction = SEND,
        .clk_div1 = 4U,
        .clk_div2 = 8,
        .clk_div3 = 16,
        .startMeas = false,
    };
    const uart_measParams defaultMeasParams{measParams};

    void setupCmd(const char* cmd, size_t cmdlen) {
        assert(cmdlen < LINE_BUFFER_LEN);
        lineBuffer.len = cmdlen;
        for (size_t i = 0; i < lineBuffer.len; ++i) {
            lineBuffer.buffer[i] = cmd[i];
        }
    }
};

TEST_F(uartLineParserCorrectCmdsTest, Clk) {
    const char cmd[] = " \t clk \t\t 00 \t 150\t\t 929487320 \t";
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &measParams);
    
    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(0U, measParams.clk_div1);
    EXPECT_EQ(150U, measParams.clk_div2);
    EXPECT_EQ(929487320U, measParams.clk_div3);

    EXPECT_EQ(defaultMeasParams.numMeas, measParams.numMeas);
    EXPECT_EQ(defaultMeasParams.dataSize, measParams.dataSize);
    EXPECT_EQ(defaultMeasParams.direction, measParams.direction);
    EXPECT_EQ(defaultMeasParams.startMeas, measParams.startMeas);
}

TEST_F(uartLineParserCorrectCmdsTest, Direction) {
    // cmd at the beginning
    const char cmd[] = "direction \treceive ";
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &measParams);
    
    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(RECEIVE, measParams.direction);

    EXPECT_EQ(defaultMeasParams.clk_div1, measParams.clk_div1);
    EXPECT_EQ(defaultMeasParams.clk_div2, measParams.clk_div2);
    EXPECT_EQ(defaultMeasParams.clk_div3, measParams.clk_div3);
    EXPECT_EQ(defaultMeasParams.numMeas, measParams.numMeas);
    EXPECT_EQ(defaultMeasParams.dataSize, measParams.dataSize);
    EXPECT_EQ(defaultMeasParams.startMeas, measParams.startMeas);
}

TEST_F(uartLineParserCorrectCmdsTest, ValidArgStart) {
    const char cmd[] = "start";
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &measParams);
    
    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(RECEIVE, measParams.direction);

    EXPECT_EQ(defaultMeasParams.clk_div1, measParams.clk_div1);
    EXPECT_EQ(defaultMeasParams.clk_div2, measParams.clk_div2);
    EXPECT_EQ(defaultMeasParams.clk_div3, measParams.clk_div3);
    EXPECT_EQ(defaultMeasParams.numMeas, measParams.numMeas);
    EXPECT_EQ(defaultMeasParams.dataSize, measParams.dataSize);
    EXPECT_EQ(defaultMeasParams.startMeas, measParams.startMeas);
}

TEST_F(uartLineParserCorrectCmdsTest, ValidArgRepeat) {
    const char cmd[] = " repeat ";
}

TEST_F(uartLineParserCorrectCmdsTest, ValidArgDatasize) {
    // cmd at the beginning
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
