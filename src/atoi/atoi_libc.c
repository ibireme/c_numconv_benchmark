/*
 Use libc's strtoll() and strtoull()
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "benchmark.h"

uint32_t atoi_u32_libc(const char *str, size_t len, char **endptr, atoi_result *res) {
    if (*str == '-') {
        *res = atoi_result_fail;
        return 0;
    }
    uint64_t val = strtoull(str, endptr, 10);
    if (val > (uint64_t)UINT32_MAX) {
        *res = atoi_result_overflow;
        return UINT32_MAX;
    }
    *res = (*endptr == str) ? atoi_result_fail : atoi_result_suc;
    return (int32_t)val;
}

int32_t atoi_i32_libc(const char *str, size_t len, char **endptr, atoi_result *res) {
    int64_t val = strtoll(str, endptr, 10);
    if (val > (int64_t)INT32_MAX) {
        *res = atoi_result_overflow;
        return INT32_MIN;
    } else if (val < (int64_t)INT32_MIN) {
        *res = atoi_result_overflow;
        return INT32_MIN;
    }
    *res = (*endptr == str) ? atoi_result_fail : atoi_result_suc;
    return (int32_t)val;
}

uint64_t atoi_u64_libc(const char *str, size_t len, char **endptr, atoi_result *res) {
    if (*str == '-') {
        *res = atoi_result_fail;
        return 0;
    }
    uint64_t val = (uint64_t)strtoull(str, endptr, 10);
    if (val == UINT64_MAX) {
        const char *max = "18446744073709551615";
        if (*endptr - str > (long)strlen(max) || strncmp(str, max, strlen(max)) != 0) {
            *res = atoi_result_overflow;
            return val;
        }
    }
    *res = (*endptr == str) ? atoi_result_fail : atoi_result_suc;
    return val;
}

int64_t atoi_i64_libc(const char *str, size_t len, char **endptr, atoi_result *res) {
    int64_t val = (int64_t)strtoll(str, endptr, 10);
    if (val == INT64_MAX) {
        const char *max = "9223372036854775807";
        if (*endptr - str > (long)strlen(max) || strncmp(str, max, strlen(max)) != 0) {
            *res = atoi_result_overflow;
            return val;
        }
    } else if (val == INT64_MIN) {
        const char *max = "-9223372036854775808";
        if (*endptr - str > (long)strlen(max) || strncmp(str, max, strlen(max)) != 0) {
            *res = atoi_result_overflow;
            return val;
        }
    }
    *res = (*endptr == str) ? atoi_result_fail : atoi_result_suc;
    return val;
}
