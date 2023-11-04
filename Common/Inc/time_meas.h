#ifndef TIME_MEAS_H
#define TIME_MEAS_H

#include <stdint.h>

static const uint32_t runtimeOffset = 24;

/* Time measurement functions interfacing the shared variables */
void time_startTime(void);
void time_endTime(void);
/* Measures the offset and sets a shared variable */
void time_setSharedOffset(void);
/* Returns the offset from the shared variable */
uint32_t time_getSharedOffset(void);
/* Returns the offset of time measurement on the caller processor*/
uint32_t time_measureOffset(void);
/* Returns the runtime compensated with the local offset and the shared offset */
uint32_t time_getRuntime(uint32_t localOffset);

#endif /* TIME_MEAS_H */