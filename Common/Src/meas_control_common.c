#include "meas_control_common.h"

static volatile uint32_t shDataSize __attribute__((section(".shared"))) = 1; 
static volatile params_direction shDirection __attribute__((section(".shared"))) = M7_SEND;
static volatile params_direction shMem __attribute__((section(".shared"))) = MEM_D1;
static uint32_t gRepeat = 1; // no need to be shared

bool ctrl_setDataSize(uint32_t dataSize, const char** msg) {
    if (dataSize < DATASIZE_LOW_LIMIT) {
        if (msg != NULL) *msg = "Datasize saturated to lower limit\r\n";
        shDataSize = DATASIZE_LOW_LIMIT;
    }
    else if (DATASIZE_UP_LIMIT < dataSize) {
        if (msg != NULL) *msg = "Datasize saturated to upper limit\r\n";
        shDataSize = DATASIZE_UP_LIMIT;
    }
    else {
        shDataSize = dataSize;
    }
    return true;
}

uint32_t ctrl_getDataSize(void) {
    return shDataSize;
}

bool ctrl_setDirection(params_direction direction, const char** msg) {
    (void)msg;
    shDirection = direction;
    return true;
}

params_direction ctrl_getDirection(void) {
    return shDirection;
}

bool ctrl_setRepeat(uint32_t repeat, const char** msg) {
    if (REPETITION_UP_LIMIT < repeat) {
        if (msg != NULL) *msg = "Repetition saturated to upper limit\r\n";
        gRepeat = REPETITION_UP_LIMIT;
    }
    else {
        gRepeat = repeat;
    }
    return true;
}

uint32_t ctrl_getRepeat(void) {
    return gRepeat;
}

bool ctrl_setMemory(params_mem mem, const char** msg) {
    (void)msg;
    mb_setUsedMemory(mem);
    shMem = mem;
    return true;
}

params_mem ctrl_getMemory(void) {
    return shMem;
}
