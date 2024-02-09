#include "csv_formatter.h"

/**
 * @brief Copies the null terminated string to the buffer
 * @returns the number of characters written
 * @note The input pointers can not overlap, no bound checking
*/
static size_t addStrToBuf(char* restrict buf, const char* restrict str){
    strcpy(buf, str);
    return strlen(str);
}

void csv_assembleResultLine(char* buf, size_t len, size_t line,
                            csv_meas_result res, csv_meas_config conf) {
    assert(buf != NULL);
    size_t cursor = 0;
    // measured values
    cursor += strn_utostrn(res.time, &buf[cursor], len - cursor);
    cursor += addStrToBuf(&buf[cursor], ", ");
    cursor += strn_utostrn(res.m7offset, &buf[cursor], len - cursor);
    cursor += addStrToBuf(&buf[cursor], ", ");
    cursor += strn_utostrn(res.m4offset, &buf[cursor], len - cursor);
    // config values
    if (line == 0) { // config values only in the first line
        cursor += addStrToBuf(&buf[cursor], ", ");
        cursor += strn_utostrn(conf.timer, &buf[cursor], len - cursor);
        cursor += addStrToBuf(&buf[cursor], ", ");
        cursor += strn_utostrn(conf.repeat, &buf[cursor], len - cursor);
        cursor += addStrToBuf(&buf[cursor], ", ");
        cursor += strn_utostrn(conf.datasize, &buf[cursor], len - cursor);
        cursor += addStrToBuf(&buf[cursor], ", ");
        cursor += strn_utostrn(conf.clkM7, &buf[cursor], len - cursor);
        cursor += addStrToBuf(&buf[cursor], ", ");
        cursor += strn_utostrn(conf.clkM4, &buf[cursor], len - cursor);
        
        cursor += addStrToBuf(&buf[cursor], ", ");
        cursor += addStrToBuf(&buf[cursor], conf.direction);
        cursor += addStrToBuf(&buf[cursor], ", ");
        cursor += addStrToBuf(&buf[cursor], conf.mem);
        cursor += addStrToBuf(&buf[cursor], ", ");
        cursor += addStrToBuf(&buf[cursor], conf.cache);
    }
    else { // rest of the lines have no config values
        for (size_t i = 0; i < CSV_CONFIG_NUM; ++i) {
            buf[cursor++] = ',';
        }
    }
    // line end
    cursor += addStrToBuf(&buf[cursor], CSV_LINE_END);
    buf[cursor++] = '\0';
    assert(cursor < len);
}

void csv_assembleResultHeader(char* buf, size_t len) {
    assert(buf != NULL);
    const char *const * resultNames 
        = (const char* const [CSV_RESULT_NUM])CSV_RESULT_NAME_ARR;
    size_t cursor = 0;
    // result name headers
    assert(0 < CSV_RESULT_NUM);
    cursor += addStrToBuf(&buf[cursor], resultNames[0]);
    for (size_t i = 1; i < CSV_RESULT_NUM; ++i) {
        cursor += addStrToBuf(&buf[cursor], ", ");
        cursor += addStrToBuf(&buf[cursor], resultNames[i]);
    }
    // config name headers
    const char *const * configNames 
        = (const char *const [CSV_CONFIG_NUM])CSV_CONFIG_NAME_ARR;
    for (size_t i = 0; i < CSV_CONFIG_NUM; ++i) {
        cursor += addStrToBuf(&buf[cursor], ", ");
        cursor += addStrToBuf(&buf[cursor], configNames[i]);
    }
    // line end
    cursor += addStrToBuf(&buf[cursor], CSV_LINE_END);
    buf[cursor++] = '\0';
    assert(cursor < len);
}

void csv_assembleResultFooter(char* buf, size_t len) {
    assert(0 < len);
    buf[0] = '\0';
}
