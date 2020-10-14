#include <stdio.h>


char *dtoa_null_impl(double val, char *buf) {
    *buf = '\0';
    return buf;
}

char *dtoa_null(double val, char *buf) {
    return dtoa_null_impl(val, buf);
}
