#ifndef STRING_N
#define STRING_N
// Own implementation for functions working with fixed length strings

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

/**
 * @brief Converts an uint to string and writes it to the buffer
 * @returns the number of characters written
 * @note Gives assertion error if not enough space in the string
 * @note Uses 10 as radix
*/
size_t strn_utostrn(uint32_t num, char* str, size_t len);

/**
 * @brief Converts a not null-terminated string containing digits to uint 
 * @returns false if found not digit true otherwise
*/
bool strn_strntou(const char* str, size_t len, uint32_t* res);

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
                           const char* delimiters);

/**
 * @brief Compares the null-terminated and fixed length string
 * @param[in] cstr Null-terminated string
 * @param[in] nstr Fixed length string
 * @param[in] nlen Size of nstr
 * @returns True if the match is exact (length and content)
*/
bool exact_strn_match(const char* cstr, const char* nstr, size_t nlen);

#endif // STRING_N