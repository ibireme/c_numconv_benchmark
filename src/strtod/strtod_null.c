#include <stddef.h>

double strtod_null(const char *str, size_t len, char **endptr) {
    *endptr = (char *)str;
    return 0.0;
}
