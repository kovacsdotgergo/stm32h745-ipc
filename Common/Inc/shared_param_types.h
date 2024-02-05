#ifndef SHARED_PARAM_TYPES_H
#define SHARED_PARAM_TYPES_H
/** @file Types for the shared parameters */

typedef enum {
    M7_SEND,
    M7_RECEIVE,
} params_direction;

/** @returns The string equivalent for the enum */
static inline const char* params_measDirectionToStr(params_direction dir) {
    assert(dir == M7_SEND|| dir == M7_RECEIVE);
    return dir == M7_SEND ? "M7 send" : "M7 receive";
}

#endif // SHARED_PARAM_TYPES_H