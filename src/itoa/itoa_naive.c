/*
 Source: https://github.com/miloyip/itoa-benchmark/blob/master/src/naive.cpp
 License: https://github.com/miloyip/itoa-benchmark/blob/master/license.txt
 
 Code is modified for benchmark.
 */

#include <stdint.h>

char *itoa_u32_naive(uint32_t value, char *buffer) {
    char temp[10];
    char *p = temp;
    do {
        *p++ = (char)(value % 10) + '0';
        value /= 10;
    } while (value > 0);

    do {
        *buffer++ = *--p;
    } while (p != temp);

    return buffer;
}

char *itoa_i32_naive(int32_t value, char *buffer) {
    uint32_t u = (uint32_t)(value);
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1;
    }
    return itoa_u32_naive(u, buffer);
}

char *itoa_u64_naive(uint64_t value, char *buffer) {
    char temp[20];
    char *p = temp;
    do {
        *p++ = (char)(value % 10) + '0';
        value /= 10;
    } while (value > 0);

    do {
        *buffer++ = *--p;
    } while (p != temp);

    return buffer;
}

char *itoa_i64_naive(int64_t value, char *buffer) {
    uint64_t u = (uint64_t)value;
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1;
    }
    return itoa_u64_naive(u, buffer);
}

/* benckmark config */
int itoa_naive_available_32 = 1;
int itoa_naive_available_64 = 1;
