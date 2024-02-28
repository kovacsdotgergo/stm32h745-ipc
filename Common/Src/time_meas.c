#include "time_meas.h"

volatile uint32_t shCheckpoints[TIME_NUM_CHECKPOINT] 
    __attribute__((section(".shared")));
volatile uint32_t shOffset __attribute__((section(".shared")));

void time_initTimers(void) {
#ifdef CORE_CM4
    // IMPORTANT to be able to read the timer!
    TIMER_HTIM_CHANNEL.Instance = TIMER_TIM_TYPEDEF;
#endif
}

uint32_t time_getSharedOffset() {
    return shOffset;
}

uint32_t time_getTimeDiff(time_checkpoint begin, time_checkpoint end,
                                uint32_t localOffset) {
    assert(begin < TIME_NUM_CHECKPOINT && end < TIME_NUM_CHECKPOINT);
    assert(time_checkOrder(begin, end)); 
    // if this assert fails, it is still ok but should reconsider datatype,
    //  because if it is greater than the limit, then the information about
    //  the order of the checkpoints is lost (e.g. to check if blocking on
    //  the message is before the start of the measurement)
    return shCheckpoints[end] - shCheckpoints[begin] 
           - (localOffset + shOffset) / 2;
}

uint32_t time_getRuntime(uint32_t localOffset) {
    return time_getTimeDiff(TIME_START, TIME_END, localOffset);
}

bool time_checkOrder(time_checkpoint begin, time_checkpoint end) {
    return shCheckpoints[end] - shCheckpoints[begin] < UINT32_MAX/2;
}