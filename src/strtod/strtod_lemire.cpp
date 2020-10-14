/*
 Code from https://github.com/lemire/fast_double_parser
 */
#include "fast_double_parser.h"

extern "C"
double strtod_lemire(const char *str, size_t len, char **endptr) {
    double val;
    bool suc = fast_double_parser::parse_number(str, &val);
    if (suc) {
        *endptr = (char *)str + len;
    } else {
        *endptr = (char *)str;
        val = 0.0;
    }
    return val;
}
