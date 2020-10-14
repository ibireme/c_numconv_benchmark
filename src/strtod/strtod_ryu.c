/*
 Code from https://github.com/ulfjack/ryu
 */
 
#ifndef _MSC_VER

#include "ryu/ryu_parse.h"
#include <string.h>

double strtod_ryu(const char *str, size_t len, char **endptr) {
    double val = 0.0;
    enum Status ret = s2d(str, &val);
    if (ret == SUCCESS) {
        *endptr = (char *)str + len;
        return val;
    } else {
        *endptr = (char *)str;
        return 0.0;
    }
}

#endif