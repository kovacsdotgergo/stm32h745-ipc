#ifndef CSV_FORMATTER_H
#define CSV_FORMATTER_H

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "string_n.h"

#define CSV_LINE_END "\r\n" // crlf

// X macro for the configuration parameters
// X(type, name)
#define CSV_MEAS_CONFIG_TYPES(X) \
    X(uint32_t, timer) \
    X(uint32_t, repeat) \
    X(uint32_t, datasize) \
    X(uint32_t, clkM7) \
    X(uint32_t, clkM4) \
    X(const char*, direction) \
    X(const char*, mem) \
    X(const char*, cache)

// X macro for the measured results
// X(type, name)
#define CSV_MEAS_RESULT_TYPES(X) \
    X(uint32_t, time) \
    X(uint32_t, m7offset) \
    X(uint32_t, m4offset)

#define X_TO_CSV_STRUCT(type, name) type name;
typedef struct {
    CSV_MEAS_CONFIG_TYPES(X_TO_CSV_STRUCT)
} csv_meas_config;

typedef struct {
    CSV_MEAS_RESULT_TYPES(X_TO_CSV_STRUCT)
} csv_meas_result;

#define X_TO_STRUCT_NUM(type, name) +1
#define CSV_CONFIG_NUM (0 CSV_MEAS_CONFIG_TYPES(X_TO_STRUCT_NUM))
#define CSV_RESULT_NUM (0 CSV_MEAS_RESULT_TYPES(X_TO_STRUCT_NUM))

#define X_TO_NAME_ARR(type, name) #name,
#define CSV_CONFIG_NAME_ARR {CSV_MEAS_CONFIG_TYPES(X_TO_NAME_ARR)}
#define CSV_RESULT_NAME_ARR {CSV_MEAS_RESULT_TYPES(X_TO_NAME_ARR)}


/**
 * @brief Assemble one line of the output in csv format in the buffer
 * @param[inout] buf target buffer for the output msg
 * @param[in] len the size of buf
 * @param[in] line the index of the line starting from 0
 * @param[in] res the measured values
 * @param[in] conf configuration values of the measurement
*/
void csv_assembleResultLine(char* buf, size_t len, size_t line,
                            csv_meas_result res, csv_meas_config conf);

/**
 * @brief Assemble the output header in csv format in the buffer
 * @param[inout] buf target buffer for the ouptut msg
 * @param[in] len the size of buf
*/
void csv_assembleResultHeader(char* buf, size_t len);

/**
 * @brief Assemble the output footer in csv format in the buffer
 * @param[inout] buf target buffer for the ouptut msg
 * @param[in] len the size of buf
*/
void csv_assembleResultFooter(char* buf, size_t len);

#endif // CSV_FORMATTER_H
