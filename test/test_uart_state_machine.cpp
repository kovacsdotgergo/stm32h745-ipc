#include <gtest/gtest.h>

extern "C" {
    #include "uart_state_machine.h"
}

TEST(uartStateMachineSuite, uninitialized_then_notStartChar) {
    uint32_t pNumMeas[NUM_MEAS_STRING_LEN];
    uint32_t pMeasDataSize[MEAS_DATA_SIZE_STRING_LEN];
    EXPECT_EQ(uartStateMachineStep('a', pNumMeas, pMeasDataSize), );
}