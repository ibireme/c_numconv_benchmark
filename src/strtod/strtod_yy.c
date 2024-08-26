/*
 Code from https://github.com/ibireme/yyjson
 */
#include "yy_double.h"
#include <stdio.h>

double strtod_yy(const char *str, size_t len, char **endptr) {
    return yy_string_to_double(str, endptr);
}

