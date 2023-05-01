#include <stdint.h>

/* Maximum size for the messages */
#define MAX_DATA_SIZE (16384)

/** Shared variables for time measurement
 *  Only for gcc compiler */
uint32_t startTime __attribute__((section(".RAM_D3_Z3")));
uint32_t endTime __attribute__((section(".RAM_D3_Z3")));

const uint32_t runtimeOffset = 24;