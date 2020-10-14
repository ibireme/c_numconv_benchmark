#include <stddef.h>
#include "david_gay_dtoa.h"

double strtod_david_gay(const char *str, size_t len, char **endptr) {
    return strtod_gay(str, endptr);
}
