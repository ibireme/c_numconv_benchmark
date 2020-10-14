#include <stdio.h>

char *dtoa_printf(double val, char *buf) {
    int len = snprintf(buf, 32, "%.17g", val);
    buf += len;
    *buf = '\0';
    return buf;
}
