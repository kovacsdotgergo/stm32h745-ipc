#include "meas_control_common.h"
 
static volatile uint32_t shDataSize __attribute__((section(".shared"))); 
static volatile measDirection shDirection __attribute__((section(".shared")));

void ctrl_setDataSize(uint32_t dataSize) {
    shDataSize = dataSize;
}

uint32_t ctrl_getDataSize(void) {
    return shDataSize;
}

void ctrl_setDirection(measDirection direction) {
    shDirection = direction;
}

measDirection ctrl_getDirection(void) {
    return shDirection;
}
