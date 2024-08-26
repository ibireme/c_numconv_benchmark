/*==============================================================================
 * Copyright (C) 2020 YaoYuan <ibireme@gmail.com>.
 * Released under the MIT license (MIT).
 *============================================================================*/

#ifndef yy_double_h
#define yy_double_h

#ifdef __cplusplus
extern "C" {
#endif

/**
 Read string as double.
 This method only accepts strings in JSON format: https://tools.ietf.org/html/rfc8259
 @param str C-string beginning with the representation of a floating-point number.
 @param endptr Ending pointer after the numerical value, or point to `str` if failed.
 @return The double number, 0.0 if failed, +/-HUGE_VAL if overflow.
 */
double yy_string_to_double(const char *str, char **endptr);

/**
 Write double to string (shortest decimal representation with null-terminator).
 @param val A double number.
 @param buf A string buffer, as least 40 bytes.
 @return The ending of this string.
 */
char *yy_double_to_string(double val, char *buf);

#ifdef __cplusplus
}
#endif
#endif
