#ifndef SHARED_PARAM_TYPES_H
#define SHARED_PARAM_TYPES_H
/** @file Types for the shared parameters */

#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

typedef enum {
    M7_SEND = 0U,
    M7_RECEIVE,
    DIRECTION_NUM,
} params_direction;

typedef enum {
    MEM_D1 = 0U,
    MEM_D2,
    MEM_D3,
    MEM_NUM,
} params_mem;

typedef enum {
    NO_CACHE = 0U,
    I_CACHE = 1U, // 0b01
    D_CACHE = 2U, // 0b10
    ID_CACHE = 3U, // 0b11
    CACHE_NUM,
} params_cache;

/** @returns The string equivalent of the params_mem enum*/
static inline const char* params_memToStr(params_mem mem) {
    switch (mem) {
    case MEM_D1: return "D1";
    case MEM_D2: return "D2";
    case MEM_D3: return "D3";
    default: assert(false);
    }
    return NULL;
}

/** @returns The string equivalent of the params_direction enum */
static inline const char* params_measDirectionToStr(params_direction dir) {
    switch (dir) {
    case M7_SEND: return "M7 send";
    case M7_RECEIVE: return "M7 receive";
    default: assert(false);
    }
    return NULL;
}

/** @returns The string equivalent of the params_cache enum */
static inline const char* params_cacheToStr(params_cache cache) {
    switch (cache) {
    case NO_CACHE: return "no cache";
    case I_CACHE: return "I cache";
    case D_CACHE: return "D cache";
    case ID_CACHE: return "ID cache";    
    default: assert(false);
    }
    return NULL;
}

/** @brief Sets to output parameters if the I or D cache is enabled */
static inline void params_cacheToID(params_cache cache, 
                                    bool* icache, bool* dcache) {
    assert(cache < CACHE_NUM);
    if (icache != NULL) *icache = cache & I_CACHE;
    if (dcache != NULL) *dcache = cache & D_CACHE;
}

/** @returns the seperate bool values converted to the enum */
static inline params_cache params_IDtoCache(bool icache, bool dcache) {
    params_cache ret = NO_CACHE;
    if (icache) ret &= I_CACHE;
    if (dcache) ret &= D_CACHE;
    return ret; 
}

#endif // SHARED_PARAM_TYPES_H
