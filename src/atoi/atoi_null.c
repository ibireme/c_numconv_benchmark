/*
 Do nothing...
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "benchmark.h"

uint32_t atoi_u32_null(const char *str, size_t len, char **endptr, atoi_result *res) {
    *endptr = (char *)str;
    *res = atoi_result_fail;
    return 0;
}

int32_t atoi_i32_null(const char *str, size_t len, char **endptr, atoi_result *res) {
    *endptr = (char *)str;
    *res = atoi_result_fail;
    return 0;
}

uint64_t atoi_u64_null(const char *str, size_t len, char **endptr, atoi_result *res) {
    *endptr = (char *)str;
    *res = atoi_result_fail;
    return 0;
}

int64_t atoi_i64_null(const char *str, size_t len, char **endptr, atoi_result *res) {
    *endptr = (char *)str;
    *res = atoi_result_fail;
    return 0;
}
