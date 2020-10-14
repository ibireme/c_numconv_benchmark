/*
 The null function is used to meansure the benchmark overhead.
 */

#include <stdint.h>

char *itoa_u32_null(uint32_t val, char *buf) {
    return buf;
}

char *itoa_i32_null(int32_t val, char *buf) {
    return buf;
}

char *itoa_u64_null(uint64_t val, char *buf) {
    return buf;
}

char *itoa_i64_null(int64_t val, char *buf) {
    return buf;
}

/* benckmark config */
int itoa_null_available_32 = 1;
int itoa_null_available_64 = 1;
