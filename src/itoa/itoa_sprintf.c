/*
 C sprintf
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

char *itoa_u32_sprintf(uint32_t val, char *buf) {
    return buf + snprintf(buf, 11, "%" PRIu32, val);
}

char *itoa_i32_sprintf(int32_t val, char *buf) {
    return buf + snprintf(buf, 12, "%" PRIi32, val);
}

char *itoa_u64_sprintf(uint64_t val, char *buf) {
    return buf + snprintf(buf, 21, "%" PRIu64, val);
}

char *itoa_i64_sprintf(int64_t val, char *buf) {
    return buf + snprintf(buf, 21, "%" PRIi64, val);
}

/* benckmark config */
int itoa_sprintf_available_32 = 1;
int itoa_sprintf_available_64 = 1;
