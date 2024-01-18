#include "string_n.h"

size_t strn_utostrn(uint32_t num, char* str, size_t len) {
    assert(str != NULL && 0 < len);
    const uint32_t radix = 10;
    const size_t maxDigitsUint32InBase10 = 9;
    
    if (num == 0) {
        str[0] = '0';
        return 1;
    }
    // convert to string
    size_t reversedLen = 0;
    char reversedStr[maxDigitsUint32InBase10];
    for (;0 < num; num /= radix) {
        reversedStr[reversedLen++] = (num % radix) + '0';
    }
    // not enough space
    if (len < reversedLen) assert(false);
    // inverting the reversed string
    for (size_t i = 0; i < reversedLen; ++i) {
        str[i] = reversedStr[reversedLen - 1 - i];
    }
    return reversedLen;
}

bool strn_strntou(const char* str, size_t len, uint32_t* res) {
    // todo input greater than 2**32 e.g. assert len < 10, but return some err
    assert(len != 0 && str != NULL);
    assert(res != NULL);
    uint32_t n = 0;
    while (len--) {
        if (!isdigit((unsigned char)*str)) {
            return false;
        }
        n = 10 * n + *(str++) - '0'; 
    }
    *res = n;
    return true;
}

/**
 * @brief returns if the char is in the null-terminated string
*/
static bool charInStr(char c, const char* str) {
    assert(str != NULL);
    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (c == str[i]) return true;
    }
    return false;
}

/**
 * @brief Static local function that splits a not null terminated string by
 *  calculating the beginning of the first token
 * @param[in] str the string to split
 * @param[in] len the length of the input string
 * @param[out] tok_len the length of the output token
 * @param[in] delimiters the token delimiter characters in a null-terminated
 *  string
 * @returns NULL if no token found, or the pointer to the first element of
 *  the token
*/
const char* strn_strntok(const char* str, size_t len,
                           size_t* tok_len, 
                           const char* delimiters){
    assert(delimiters != NULL && tok_len != NULL);
    assert(len == 0 || str != NULL);
    size_t i = 0;
    // skip delimiters
    while(i < len && charInStr(str[i], delimiters)) { ++i; }
    const size_t begIdx = i;
    // find end of token
    while(i < len && !charInStr(str[i], delimiters)) { ++i; }
    const size_t endIdx = i;
    
    *tok_len = endIdx - begIdx;
    if (*tok_len == 0) {
        return NULL;
    }
    else {
        return str + begIdx;
    }
}

bool exact_strn_match(const char* cstr, const char* nstr, size_t nlen) {
    return strlen(cstr) == nlen && strncmp(cstr, nstr, nlen);
}
