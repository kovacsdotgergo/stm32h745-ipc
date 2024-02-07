#include "meas_control_common.h"

static volatile uint32_t shDataSize __attribute__((section(".shared"))); 
static volatile params_direction shDirection __attribute__((section(".shared")));
static volatile params_mem shMem __attribute__((section(".shared")));
static uint32_t g_repeat = 1; // no need to be shared

void ctrl_initSharedVariables(void) {
#ifdef CORE_CM7
    shDataSize = 1;
    shDirection = M7_SEND;
    shMem = MEM_D1;
#endif
}

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
        g_repeat = REPETITION_UP_LIMIT;
    }
    else {
        g_repeat = repeat;
    }
    return true;
}

uint32_t ctrl_getRepeat(void) {
    return g_repeat;
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
