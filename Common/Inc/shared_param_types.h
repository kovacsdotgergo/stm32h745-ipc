#ifndef SHARED_PARAM_TYPES_H
#define SHARED_PARAM_TYPES_H
/** @file Types for the shared parameters */

#include <stdbool.h>
#include <assert.h>

typedef enum {
    M7_SEND = 0,
    M7_RECEIVE,
    DIRECTION_NUM,
} params_direction;

typedef enum {
    MEM_D1 = 0,
    MEM_D2,
    MEM_D3,
    MEM_NUM,
} params_mem;

/** @returns The string equivalent for the params_mem enum*/
static inline const char* params_memToStr(params_mem mem) {
    switch (mem) {
    case MEM_D1: return "D1";
    case MEM_D2: return "D2";
    case MEM_D3: return "D3";
    default: assert(false);
    }
}

/** @returns The string equivalent for the params_direction enum */
static inline const char* params_measDirectionToStr(params_direction dir) {
    switch (dir) {
    case M7_SEND: return "M7 send";
    case M7_RECEIVE: return "M7 receive";
    default: assert(false);
    }
}

#endif // SHARED_PARAM_TYPES_H