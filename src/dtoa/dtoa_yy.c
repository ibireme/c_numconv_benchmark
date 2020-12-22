/*
 Code from https://github.com/ibireme/yyjson
 */

#include "yy_double.h"

char *dtoa_yy(double val, char *buf) {
    char *end = yy_double_to_string(val, buf);
    *end = '\0';
    return end;
}
