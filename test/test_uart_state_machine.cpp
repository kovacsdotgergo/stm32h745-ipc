#include <gtest/gtest.h>

// eliminating the c restrict keyword
#ifdef __GNUC__
    #define restrict __restrict__
#else 
    #define restrict
#endif

extern "C" {
    #include "uart_commands.h"
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
    const char* echo = NULL;

    for (size_t i = 0; i < len; ++i) {
        auto ret = uart_addCharToBuffer(inString[i], &lineBuffer, &echo);
        EXPECT_EQ(BUFFER_OK, ret);
        EXPECT_EQ(i + 1, lineBuffer.len);
    }
    auto ret = uart_addCharToBuffer('\r', &lineBuffer, &echo);

    EXPECT_EQ(BUFFER_DONE, ret);
    EXPECT_EQ(len, lineBuffer.len);
    EXPECT_EQ(0, memcmp(inString, lineBuffer.buffer, len));
}

TEST(uartLineBufferTest, Overflow) {
    uart_LineBuffer lineBuffer = {
        .buffer = {0},
        .len = LINE_BUFFER_LEN,
    };
    const char* echo = NULL;
    // testing for a uniformly filled string
    auto ret = uart_addCharToBuffer('a', &lineBuffer, &echo);
    EXPECT_EQ(BUFFER_OVERFLOW, ret);
}

TEST(uartLineBufferTest, Backspace) {
    uart_LineBuffer lineBuffer = {
        .buffer = {[0] = 'a'},
        .len = 1,
    };
    const char* echo = NULL;

    // first deletion with char in the buffer
    auto ret = uart_addCharToBuffer('\b', &lineBuffer, &echo);
    EXPECT_EQ(BUFFER_OK, ret);
    EXPECT_EQ(0, lineBuffer.len);
    // second deletion with empty buffer
    ret = uart_addCharToBuffer('\b', &lineBuffer, &echo);
    EXPECT_EQ(BUFFER_OK, ret);
    EXPECT_EQ(0, lineBuffer.len);
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
    const char* msg = NULL;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams, &msg);
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
    const char* msg = NULL;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams, &msg);
    EXPECT_EQ(PARSE_ARG_NUM_ERR, ret);
}

TEST(uartLineParserTest, InvalidCmd) {
    const char cmd[] = "repea";
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = sizeof(cmd) - 1,
    };
    for (size_t i = 0; i < lineBuffer.len; ++i) {
        lineBuffer.buffer[i] = cmd[i];
    }
    uart_measParams measParams;
    const char* msg = NULL;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams, &msg);
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
    const char* msg = NULL;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams, &msg);
    EXPECT_EQ(PARSE_ARG_NUM_ERR, ret);
}

TEST(uartLineParserTest, InvalidArg) {
    const char cmd[] = " \t direction \t\tsen\0";
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = sizeof(cmd) - 1,
    };
    for (size_t i = 0; i < lineBuffer.len; ++i) {
        lineBuffer.buffer[i] = cmd[i];
    }
    uart_measParams measParams;
    const char* msg = NULL;

    auto ret = uart_parseBuffer(&lineBuffer, &measParams, &msg);
    EXPECT_EQ(PARSE_ARG_VAL_ERR, ret);
}

// uartLineParserCorrectCmdsTest ==========================================
class uartLineParserCorrectCmdsTest : public ::testing::Test {
public:
    uart_LineBuffer lineBuffer{};
    uart_measParams measParams{
        .repeat = 1U,
        .dataSize = 2U,
        .direction = SEND,
        .clk_div1 = 4U,
        .clk_div2 = 8U,
        .clk_div3 = 32U,
        .startMeas = false,
    };
    const uart_measParams defaultMeasParams{measParams};
    const char* msg{NULL};

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
    uint32_t testDivs[] = {0U, 150U, 929487320U};
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &measParams, &msg);
    
    // expecting saturated values
    for (size_t i = 0; i < 3; ++i) {
        if (testDivs[i] < CLK_DIV_LOW_LIMIT) {
            testDivs[i] = CLK_DIV_LOW_LIMIT;
        }
        else if (CLK_DIV_UP_LIMIT < testDivs[i]) {
            testDivs[i] = CLK_DIV_UP_LIMIT;
        }
    }
    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(testDivs[0], measParams.clk_div1);
    EXPECT_EQ(testDivs[1], measParams.clk_div2);
    EXPECT_EQ(testDivs[2], measParams.clk_div3);

    EXPECT_EQ(defaultMeasParams.repeat, measParams.repeat);
    EXPECT_EQ(defaultMeasParams.dataSize, measParams.dataSize);
    EXPECT_EQ(defaultMeasParams.direction, measParams.direction);
    EXPECT_EQ(defaultMeasParams.startMeas, measParams.startMeas);
}

TEST_F(uartLineParserCorrectCmdsTest, Direction) {
    // cmd at the beginning
    const char cmd[] = "direction \treceive ";
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &measParams, &msg);
    
    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(RECEIVE, measParams.direction);

    EXPECT_EQ(defaultMeasParams.repeat, measParams.repeat);
    EXPECT_EQ(defaultMeasParams.dataSize, measParams.dataSize);
    EXPECT_EQ(defaultMeasParams.clk_div1, measParams.clk_div1);
    EXPECT_EQ(defaultMeasParams.clk_div2, measParams.clk_div2);
    EXPECT_EQ(defaultMeasParams.clk_div3, measParams.clk_div3);
    EXPECT_EQ(defaultMeasParams.startMeas, measParams.startMeas);
}

TEST_F(uartLineParserCorrectCmdsTest, Start) {
    const char cmd[] = "start";
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &measParams, &msg);
    
    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(true, measParams.startMeas);

    EXPECT_EQ(defaultMeasParams.repeat, measParams.repeat);
    EXPECT_EQ(defaultMeasParams.dataSize, measParams.dataSize);
    EXPECT_EQ(defaultMeasParams.direction, measParams.direction);
    EXPECT_EQ(defaultMeasParams.clk_div1, measParams.clk_div1);
    EXPECT_EQ(defaultMeasParams.clk_div2, measParams.clk_div2);
    EXPECT_EQ(defaultMeasParams.clk_div3, measParams.clk_div3);
}

TEST_F(uartLineParserCorrectCmdsTest, Repeat) {
    const char cmd[] = " repeat 2980";
    uint32_t testRep = 2980U;
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &measParams, &msg);

    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(testRep < REPETITION_UP_LIMIT ? testRep : REPETITION_UP_LIMIT,
              measParams.repeat);

    EXPECT_EQ(defaultMeasParams.dataSize, measParams.dataSize);
    EXPECT_EQ(defaultMeasParams.direction, measParams.direction);
    EXPECT_EQ(defaultMeasParams.clk_div1, measParams.clk_div1);
    EXPECT_EQ(defaultMeasParams.clk_div2, measParams.clk_div2);
    EXPECT_EQ(defaultMeasParams.clk_div3, measParams.clk_div3);
    EXPECT_EQ(defaultMeasParams.startMeas, measParams.startMeas);
}

TEST_F(uartLineParserCorrectCmdsTest, Datasize) {
    const char cmd[] = "datasize \t 389";
    uint32_t testSize = 389U;
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &measParams, &msg);

    if (testSize < DATASIZE_LOW_LIMIT) {
        testSize = DATASIZE_LOW_LIMIT;
    }
    else if (DATASIZE_UP_LIMIT < testSize) {
        testSize = DATASIZE_UP_LIMIT;
    }
    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(testSize, measParams.dataSize);

    EXPECT_EQ(defaultMeasParams.repeat, measParams.repeat);
    EXPECT_EQ(defaultMeasParams.direction, measParams.direction);
    EXPECT_EQ(defaultMeasParams.clk_div1, measParams.clk_div1);
    EXPECT_EQ(defaultMeasParams.clk_div2, measParams.clk_div2);
    EXPECT_EQ(defaultMeasParams.clk_div3, measParams.clk_div3);
    EXPECT_EQ(defaultMeasParams.startMeas, measParams.startMeas);
}
