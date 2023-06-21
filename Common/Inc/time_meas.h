#ifndef TIME_MEAS_H
#define TIME_MEAS_H

#include <stdint.h>

/* Maximum size for the messages */
#define MAX_DATA_SIZE (32764)

/** Shared variables for time measurement
 *  Only for gcc compiler */
static uint32_t startTime __attribute__((section(".RAM_D3_Z3")));
static uint32_t endTime __attribute__((section(".RAM_D3_Z3")));

static const uint32_t runtimeOffset = 24;

#endif /* TIME_MEAS_H */