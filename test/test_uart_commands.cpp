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

    const char bs = 127;
    // first deletion with char in the buffer
    auto ret = uart_addCharToBuffer(bs, &lineBuffer, &echo);
    EXPECT_EQ(BUFFER_OK, ret);
    EXPECT_EQ(0, lineBuffer.len);
    // second deletion with empty buffer
    ret = uart_addCharToBuffer(bs, &lineBuffer, &echo);
    EXPECT_EQ(BUFFER_OK, ret);
    EXPECT_EQ(0, lineBuffer.len);
}

// uartLineParserTest =====================================================
class uartLineParserTest : public ::testing::Test {
protected:
    static bool setClksStub(uint32_t, uint32_t, const char**)
        { return true; }
    static void getClksStub(uint32_t*, uint32_t*) {}
    static bool setDataSizeStub(uint32_t, const char**)
        { return true; }
    static uint32_t getDataSizeStub(void)
        { return 2; }
    static bool setRepeatStub(uint32_t, const char**)
        { return true; }
    static uint32_t getRepeatStub(void) 
        { return 1; }
    static bool setDirectionStub(params_direction, const char**)
        { return true; }
    static params_direction getDirectionStub(void) 
        { return M7_SEND; }
    static void setStartMeasStub(void) {}

    uart_controlIf controlFuns{
        .setClks = setClksStub,
        .getClks = getClksStub,
        .setDataSize = setDataSizeStub,
        .getDataSize = getDataSizeStub,
        .setRepeat = setRepeatStub,
        .getRepeat = getRepeatStub,
        .setDirection = setDirectionStub,
        .getDirection = getDirectionStub,
        .setStartMeas = setStartMeasStub,
    };
    const char* msg{NULL};
};

TEST_F(uartLineParserTest, EmptyLine) {
    const char cmd[] = " \t\t "; // only separators
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = sizeof(cmd) - 1,
    };
    for (size_t i = 0; i < lineBuffer.len; ++i) {
        lineBuffer.buffer[i] = cmd[i];
    }

    auto ret = uart_parseBuffer(&lineBuffer, &controlFuns, &msg);
    EXPECT_EQ(PARSE_COMMAND_ERR, ret);
}

TEST_F(uartLineParserTest, CmdAtTheEnd) {
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

    auto ret = uart_parseBuffer(&lineBuffer, &controlFuns, &msg);
    EXPECT_EQ(PARSE_ARG_NUM_ERR, ret);
}

TEST_F(uartLineParserTest, InvalidCmd) {
    const char cmd[] = "repea";
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = sizeof(cmd) - 1,
    };
    for (size_t i = 0; i < lineBuffer.len; ++i) {
        lineBuffer.buffer[i] = cmd[i];
    }

    auto ret = uart_parseBuffer(&lineBuffer, &controlFuns, &msg);
    EXPECT_EQ(PARSE_COMMAND_ERR, ret);
}

TEST_F(uartLineParserTest, WrongNumOfArgs) {
    // also whitespace before cmd
    const char cmd[] = " \t   \t clk 1   \t \t\t ";
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = sizeof(cmd) - 1,
    };
    for (size_t i = 0; i < lineBuffer.len; ++i) {
        lineBuffer.buffer[i] = cmd[i];
    }

    auto ret = uart_parseBuffer(&lineBuffer, &controlFuns, &msg);
    EXPECT_EQ(PARSE_ARG_NUM_ERR, ret);
}

TEST_F(uartLineParserTest, InvalidArg) {
    const char cmd[] = " \t direction \t\tsen\0";
    uart_LineBuffer lineBuffer = {
        .buffer = {},
        .len = sizeof(cmd) - 1,
    };
    for (size_t i = 0; i < lineBuffer.len; ++i) {
        lineBuffer.buffer[i] = cmd[i];
    }

    auto ret = uart_parseBuffer(&lineBuffer, &controlFuns, &msg);
    EXPECT_EQ(PARSE_ARG_VAL_ERR, ret);
}

// uartLineParserCorrectCmdsTest ==========================================
class uartLineParserCorrectCmdsTest : public ::testing::Test {
protected:
    static uint32_t repeat, datasize, clkM4, clkM7;
    static params_direction direction;
    static bool start;

    static bool setClksStub(uint32_t clkM7_, uint32_t clkM4_, const char**)
        { clkM7 = clkM7_; clkM4 = clkM4_; return true; }
    static void getClksStub(uint32_t* clkM7_, uint32_t* clkM4_) 
        { *clkM7_ = clkM7; *clkM4_ = clkM4; }
    static bool setDataSizeStub(uint32_t datasize_, const char**)
        { datasize = datasize_; return true; }
    static uint32_t getDataSizeStub(void)
        { return datasize; }
    static bool setRepeatStub(uint32_t repeat_, const char**)
        { repeat = repeat_; return true; }
    static uint32_t getRepeatStub(void) 
        { return repeat; }
    static bool setDirectionStub(params_direction direction_, const char**)
        { direction = direction_; return true; }
    static params_direction getDirectionStub(void) 
        { return direction; }
    static void setStartMeasStub(void)
        { start = true; }

    uart_LineBuffer lineBuffer{};
    uart_controlIf controlFuns{
        .setClks = setClksStub,
        .getClks = getClksStub,
        .setDataSize = setDataSizeStub,
        .getDataSize = getDataSizeStub,
        .setRepeat = setRepeatStub,
        .getRepeat = getRepeatStub,
        .setDirection = setDirectionStub,
        .getDirection = getDirectionStub,
        .setStartMeas = setStartMeasStub,
    };
    const char* msg{NULL};

    void setupCmd(const char* cmd, size_t cmdlen) {
        assert(cmdlen < LINE_BUFFER_LEN);
        lineBuffer.len = cmdlen;
        for (size_t i = 0; i < lineBuffer.len; ++i) {
            lineBuffer.buffer[i] = cmd[i];
        }
    }

    void SetUp(void) override {
        repeat = datasize = clkM4 = clkM7 = 1;
        direction = M7_SEND;
        start = false;
    }

    void expectDefault(bool clkM7_, bool clkM4_, bool repeat_,
                       bool datasize_, bool direction_, bool start_) {
        if (clkM7_) { EXPECT_EQ(1, clkM7); }
        if (clkM4_) { EXPECT_EQ(1, clkM4); }
        if (repeat_) { EXPECT_EQ(1, repeat); }
        if (datasize_) { EXPECT_EQ(1, datasize); }
        if (direction_) { EXPECT_EQ(M7_SEND, direction); }
        if (start_) { EXPECT_EQ(false, start); }
    }
};

uint32_t uartLineParserCorrectCmdsTest::clkM7;
uint32_t uartLineParserCorrectCmdsTest::clkM4;
uint32_t uartLineParserCorrectCmdsTest::datasize;
uint32_t uartLineParserCorrectCmdsTest::repeat;
params_direction uartLineParserCorrectCmdsTest::direction;
bool uartLineParserCorrectCmdsTest::start;

TEST_F(uartLineParserCorrectCmdsTest, Clk) {
    const char cmd[] = " \t clk \t\t 00 \t\t929487320 \t";
    const uint32_t testClkM7 = 0U, testClkM4 = 929487320U;
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &controlFuns, &msg);
    
    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(testClkM7, clkM7);
    EXPECT_EQ(testClkM4, clkM4);

    expectDefault(false, false, true, true, true, true);
}

TEST_F(uartLineParserCorrectCmdsTest, Direction) {
    // cmd at the beginning
    const char cmd[] = "direction \treceive ";
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &controlFuns, &msg);
    
    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(M7_RECEIVE, direction);

    expectDefault(true, true, true, true, false, true);
}

TEST_F(uartLineParserCorrectCmdsTest, Start) {
    const char cmd[] = "start";
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &controlFuns, &msg);
    
    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(true, start);

    expectDefault(true, true, true, true, true, false);
}

TEST_F(uartLineParserCorrectCmdsTest, Repeat) {
    const char cmd[] = " repeat 2980";
    const uint32_t testRep = 2980U;
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &controlFuns, &msg);

    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(testRep, repeat);

    expectDefault(true, true, false, true, true, true);
}

TEST_F(uartLineParserCorrectCmdsTest, Datasize) {
    const char cmd[] = "datasize \t 389";
    const uint32_t testSize = 389U;
    setupCmd(cmd, sizeof(cmd) - 1);

    auto ret = uart_parseBuffer(&lineBuffer, &controlFuns, &msg);

    EXPECT_EQ(PARSE_OK, ret);
    EXPECT_EQ(testSize, datasize);

    expectDefault(true, true, true, false, true, true);
}
