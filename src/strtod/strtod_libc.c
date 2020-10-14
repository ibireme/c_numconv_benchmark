#include <stdlib.h>

double strtod_libc(const char *str, size_t len, char **endptr) {
    return strtod(str, endptr);
}
