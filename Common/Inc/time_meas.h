#ifndef TIME_MEAS_H
#define TIME_MEAS_H

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_gcc.h"
#include "main.h"

#define TIMER_HTIM_CHANNEL htim5
#define TIMER_TIM_TYPEDEF TIM5

typedef enum {
    TIME_START = 0,
    TIME_AFTER_SEND,
    TIME_BEGIN_BLOCK,
    TIME_END,
    TIME_NUM_CHECKPOINT,
} time_checkpoint;

/* Shared variables for time measurement, not in c due to inline funs */
extern volatile uint32_t shOffset;
extern volatile uint32_t shCheckpoints[TIME_NUM_CHECKPOINT];

/** @brief Initializes the used timers */
void time_initTimers(void);

/** @brief Sets the checkpoint when measuring time */
static inline void time_setCheckpoint(time_checkpoint checkpoint) {
    __COMPILER_BARRIER();
    shCheckpoints[checkpoint] = __HAL_TIM_GET_COUNTER(&TIMER_HTIM_CHANNEL);
    __COMPILER_BARRIER();
}

/** @brief Returns the value of the chekcpoint */
static inline uint32_t time_getCheckpoint(time_checkpoint checkpoint) {
    return shCheckpoints[checkpoint];
}

/** @returns the offset of time measurement on the caller processor*/
static inline uint32_t time_measureOffset(void) {
    __COMPILER_BARRIER();
    volatile uint32_t st = __HAL_TIM_GET_COUNTER(&TIMER_HTIM_CHANNEL);
    __COMPILER_BARRIER();
    volatile uint32_t end = __HAL_TIM_GET_COUNTER(&TIMER_HTIM_CHANNEL);
    __COMPILER_BARRIER();
    return end - st;
}

/** @brief Measures the offset and sets a shared variable */
static inline void time_setSharedOffset(void) {
    shOffset = time_measureOffset();
}

/** @returns the offset from the shared variable */
uint32_t time_getSharedOffset(void);

/** 
 * @brief returns the runtime between the checkpoints compensated with
 *  the local offset on the calling core and with the shared offset
 *  that should be set on the other core
 * 
 * @param[in] begin starting checkpoint  
 * @param[in] end ending checkpoint
 * @param[in] localOffset the offset in the measurement on this core
 * 
 * @returns the runtime between the checkpoints
*/
uint32_t time_getTimeDifference(time_checkpoint begin, time_checkpoint end,
                                uint32_t localOffset);

/** @returns the runtime compensated with the local offset and the shared offset */
uint32_t time_getRuntime(uint32_t localOffset);

/** 
 * @brief Checks if the checkpoints are in the correct order 
 * @returns true on correct order, false otherwise
*/
bool time_checkOrder(time_checkpoint begin, time_checkpoint end);

#endif /* TIME_MEAS_H */