#ifndef TIME_MEAS_H
#define TIME_MEAS_H

#include <stdint.h>

static const uint32_t runtimeOffset = 24;

/* Time measurement functions interfacing the shared variables */
void time_startTime(void);
void time_endTime(void);
uint32_t time_getRuntime(void);

#endif /* TIME_MEAS_H */