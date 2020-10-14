#ifndef dtoa_benchmark_helper_h
#define dtoa_benchmark_helper_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 Prints unsigned integer number to string (with null-terminator).
 
 @param val An unsigned integer number.
 @param buf A string buffer, at least 21 bytes.
 @return The string length.
 */
int yy_uint_to_string(uint64_t val, char *buf);

/**
 Prints signed integer number to string (with null-terminator).
 
 @param val A signed integer number.
 @param buf A string buffer, at least 21 bytes.
 @return The string length.
 */
int yy_sint_to_string(int64_t val, char *buf);

/**
 Convert double number to shortest string (with null-terminator).
 The string format follows the ECMAScript specification.
 
 @param val A double value.
 @param buf A string buffer, at least 32 bytes.
 @return The string length.
 */
int google_double_to_string(double val, char *buf);

/**
 Convert double number to string with precision (with null-terminator).
 The string format follows the ECMAScript specification.
 
 @param val A double value.
 @param prec Max precision kept by string, should in range [1-17].
 @param buf A string buffer, at least 32 bytes.
 @return The string length.
 */
int google_double_to_string_prec(double val, int prec, char *buf);

/**
 Read double number from string, support infinity and nan literal.
 
 @param str A string with double number.
 @param len A pointer to receive processed length, 0 if failed.
 @return The double value, or 0.0 if failed.
 */
double google_string_to_double(const char *str, int *len);



#ifdef __cplusplus
}
#endif

#endif
