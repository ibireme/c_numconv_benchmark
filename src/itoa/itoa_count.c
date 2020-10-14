/*
 Source: https://github.com/miloyip/itoa-benchmark/blob/master/src/count.cpp
 License: https://github.com/miloyip/itoa-benchmark/blob/master/license.txt
 
 Code is modified for benchmark.
 */


#include <stdint.h>

#ifdef _MSC_VER
#include "intrin.h"
#endif

static inline unsigned CountDecimalDigit32(uint32_t n) {
#if defined(_MSC_VER) || defined(__GNUC__)
    static const uint32_t powers_of_10[] = {
        0,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000
    };

#ifdef _MSC_VER
    unsigned long i = 0;
    _BitScanReverse(&i, n | 1);
    uint32_t t = (i + 1) * 1233 >> 12;
#elif __GNUC__
    uint32_t t = (32 - __builtin_clz(n | 1)) * 1233 >> 12;
#endif
    return t - (n < powers_of_10[t]) + 1;
#else
    // Simple pure C++ implementation
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    return 10;
#endif
}

static inline unsigned CountDecimalDigit64(uint64_t n) {
#if defined(_MSC_VER) || defined(__GNUC__)
    static const uint64_t powers_of_10[] = {
        0,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000,
        10000000000,
        100000000000,
        1000000000000,
        10000000000000,
        100000000000000,
        1000000000000000,
        10000000000000000,
        100000000000000000,
        1000000000000000000,
        10000000000000000000U
    };

#if __GNUC__
    uint32_t t = (64 - __builtin_clzll(n | 1)) * 1233 >> 12;
#elif _M_IX86
    unsigned long i = 0;
    uint64_t m = n | 1;
    if (_BitScanReverse(&i, m >> 32))
        i += 32;
    else
        _BitScanReverse(&i, m & 0xFFFFFFFF);
    uint32_t t = (i + 1) * 1233 >> 12;
#elif _M_X64
    unsigned long i = 0;
    _BitScanReverse64(&i, n | 1);
    uint32_t t = (i + 1) * 1233 >> 12;
#endif
    return t - (n < powers_of_10[t]) + 1;
#else
    // Simple pure C++ implementation
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    if (n < 10000000000) return 10;
    if (n < 100000000000) return 11;
    if (n < 1000000000000) return 12;
    if (n < 10000000000000) return 13;
    if (n < 100000000000000) return 14;
    if (n < 1000000000000000) return 15;
    if (n < 10000000000000000) return 16;
    if (n < 100000000000000000) return 17;
    if (n < 1000000000000000000) return 18;
    if (n < 10000000000000000000) return 19;
    return 20;
#endif
}






// Additional count number of digit pass

char *itoa_u32_count(uint32_t value, char* buffer) {
    char *end;
    unsigned digit = CountDecimalDigit32(value);
    buffer += digit;
    end = buffer;
    // *buffer = '\0';
    
    do {
        *--buffer = (char)(value % 10) + '0';
        value /= 10;
    } while (value > 0);
    return end;
}

char *itoa_i32_count(int32_t value, char* buffer) {
    uint32_t u = (uint32_t)(value);
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1;
    }
    return itoa_u32_count(u, buffer);
}

char *itoa_u64_count(uint64_t value, char* buffer) {
    char *end;
    unsigned digit = CountDecimalDigit64(value);
    buffer += digit;
    end = buffer;
    // *buffer = '\0';

    do {
        *--buffer = (char)(value % 10) + '0';
        value /= 10;
    } while (value > 0);
    return end;
}

char *itoa_i64_count(int64_t value, char* buffer) {
    uint64_t u = (uint64_t)(value);
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1;
    }
    return itoa_u64_count(u, buffer);
}

/* benckmark config */
int itoa_count_available_32 = 1;
int itoa_count_available_64 = 1;
