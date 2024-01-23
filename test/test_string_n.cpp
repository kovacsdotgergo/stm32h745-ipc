#include <gtest/gtest.h>

extern "C" {
    #include "string_n.h"
}

// strntouTest ============================================================
TEST(strntouTest, NotDigitError) {
    const char input[] = "10a98";
    size_t len = sizeof(input) - 1;
    uint32_t res;

    bool ret = strn_strntou(input, len, &res);

    EXPECT_EQ(false, ret);
}

TEST(strntouTest, Zero) {
    const char input[] = "0000";
    size_t len = sizeof(input) - 1;
    uint32_t res;

    bool ret = strn_strntou(input, len, &res);

    EXPECT_EQ(true, ret);
    EXPECT_EQ(0, res);
}

TEST(strntouTest, BigNum) {
    const char input[] = "4294967295"; // 2**32 - 1
    size_t len = sizeof(input) - 1;
    uint32_t res;

    bool ret = strn_strntou(input, len, &res);

    EXPECT_EQ(true, ret);
    EXPECT_EQ(4294967295U, res);
}
// utostrnTest ============================================================
class utostrnTest : public ::testing::Test {
public:
    static constexpr size_t strLen{16}; 
};

TEST_F(utostrnTest, Zero) {
    const uint32_t input = 0U;
    char str[strLen] = {0};

    auto ret = strn_utostrn(input, str, strLen);

    EXPECT_EQ(1, ret);
    EXPECT_EQ(0, strcmp(str, "0"));
}

TEST_F(utostrnTest, Mid) {
    const uint32_t input = 2863311530U;
    const char* res = "2863311530";
    char str[strLen] = {0};

    auto ret = strn_utostrn(input, str, strLen);

    EXPECT_EQ(strlen(res), ret);
    EXPECT_EQ(0, strcmp(str, res));
}

TEST_F(utostrnTest, End) {
    const uint32_t input = 4294967295U;
    const char* res = "4294967295"; // 2**32 - 1
    char str[strLen] = {0};

    auto ret = strn_utostrn(input, str, strLen);

    EXPECT_EQ(strlen(res), ret);
    EXPECT_EQ(0, strcmp(str, res));
}
