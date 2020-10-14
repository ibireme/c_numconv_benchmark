/*
 Core code from https://github.com/simdjson/simdjson
 Requires 8-byte padding
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "benchmark.h"

/* compiler builtin check (clang) */
#ifndef has_builtin
#   ifdef __has_builtin
#       define has_builtin(x) __has_builtin(x)
#   else
#       define has_builtin(x) 0
#   endif
#endif

/* compiler attribute check (gcc/clang) */
#ifndef has_attribute
#   ifdef __has_attribute
#       define has_attribute(x) __has_attribute(x)
#   else
#       define has_attribute(x) 0
#   endif
#endif

/* inline */
#ifndef really_inline
#   if _MSC_VER >= 1200
#       define really_inline __forceinline
#   elif defined(_MSC_VER)
#       define really_inline __inline
#   elif has_attribute(always_inline) || __GNUC__ >= 4
#       define really_inline __inline__ __attribute__((always_inline))
#   elif defined(__clang__) || defined(__GNUC__)
#       define really_inline __inline__
#   elif defined(__cplusplus) || (__STDC__ >= 1 && __STDC_VERSION__ >= 199901L)
#       define really_inline inline
#   else
#       define really_inline
#   endif
#endif

/* likely */
#ifndef likely
#   if has_builtin(__builtin_expect) || __GNUC__ >= 4
#       define likely(expr) __builtin_expect(!!(expr), 1)
#   else
#       define likely(expr) (expr)
#   endif
#endif

/* unlikely */
#ifndef unlikely
#   if has_builtin(__builtin_expect) || __GNUC__ >= 4
#       define unlikely(expr) __builtin_expect(!!(expr), 0)
#   else
#       define unlikely(expr) (expr)
#   endif
#endif




// check quickly whether the next 8 chars are made of digits
// at a glance, it looks better than Mula's
// http://0x80.pl/articles/swar-digits-validate.html
static really_inline bool is_made_of_eight_digits_fast(const char *chars) {
    uint64_t val;
    memcpy(&val, chars, 8);
    // a branchy method might be faster:
    // return (( val & 0xF0F0F0F0F0F0F0F0 ) == 0x3030303030303030)
    //  && (( (val + 0x0606060606060606) & 0xF0F0F0F0F0F0F0F0 ) ==
    //  0x3030303030303030);
    return (((val & 0xF0F0F0F0F0F0F0F0) |
             (((val + 0x0606060606060606) & 0xF0F0F0F0F0F0F0F0) >> 4)) ==
            0x3333333333333333);
}


#ifdef __AVX2__

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#include <emmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>

// Haswell (AVX2)
static really_inline uint32_t parse_eight_digits_unrolled(const char *chars) {
    // this actually computes *16* values so we are being wasteful.
    const __m128i ascii0 = _mm_set1_epi8('0');
    const __m128i mul_1_10 = _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
    const __m128i mul_1_100 = _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1);
    const __m128i mul_1_10000 = _mm_setr_epi16(10000, 1, 10000, 1, 10000, 1, 10000, 1);
    const __m128i input = _mm_sub_epi8(_mm_loadu_si128((const __m128i *)(chars)), ascii0);
    const __m128i t1 = _mm_maddubs_epi16(input, mul_1_10);
    const __m128i t2 = _mm_madd_epi16(t1, mul_1_100);
    const __m128i t3 = _mm_packus_epi32(t2, t2);
    const __m128i t4 = _mm_madd_epi16(t3, mul_1_10000);
    return _mm_cvtsi128_si32(t4); // only captures the sum of the first 8 digits, drop the rest
}

#else

// we don't have SSE, so let us use a scalar function
// credit: https://johnnylee-sde.github.io/Fast-numeric-string-to-int/
static really_inline uint32_t parse_eight_digits_unrolled(const char *chars) {
    uint64_t val;
    memcpy(&val, chars, sizeof(uint64_t));
    val = (val & 0x0F0F0F0F0F0F0F0FULL) * 2561 >> 8;
    val = (val & 0x00FF00FF00FF00FFULL) * 6553601 >> 16;
    return (uint32_t)((val & 0x0000FFFF0000FFFFULL) * 42949672960001ULL >> 32);
}

#endif

static really_inline bool parse_digit_u32(const char c, uint32_t *i) {
    const uint8_t digit = c - '0';
    if (digit > 9) {
        return false;
    }
    // PERF NOTE: multiplication by 10 is cheaper than arbitrary integer multiplication
    *i = 10 * *i + digit; // might overflow, we will handle the overflow later
    return true;
}

static really_inline bool parse_digit_u64(const char c, uint64_t *i) {
    const uint8_t digit = c - '0';
    if (digit > 9) {
        return false;
    }
    // PERF NOTE: multiplication by 10 is cheaper than arbitrary integer multiplication
    *i = 10 * *i + digit; // might overflow, we will handle the overflow later
    return true;
}

static really_inline bool is_digit(const char c) {
    const uint8_t digit = c - '0';
    return digit <= 9;
}

uint32_t atoi_u32_lemire(const char *str, size_t len, char **endptr, atoi_result *res) {
    if (unlikely(*str <= '0' || *str > '9')) {
        if (*str == '0' && !is_digit(str[1])) {
            *endptr = (char *)str + 1;
            *res = atoi_result_suc;
        } else {
            *endptr = (char *)str;
            *res = atoi_result_fail;
        }
        return 0;
    }
    
    uint32_t i = 0;
    if (is_made_of_eight_digits_fast(str)) {
        i = i * 100000000 + parse_eight_digits_unrolled(str);
        str += 8;
        if (parse_digit_u32(*str, &i)) { // digit 9
            str++;
            const uint8_t num = *str - '0'; // digit 10
            if (num <= 9) {
                if (i > UINT32_MAX / 10 ||
                    ((i == UINT32_MAX / 10) && num > UINT32_MAX % 10) ||
                    (is_digit(str[1]))) {
                    while (is_digit(*str)) str++;
                    *endptr = (char *)str;
                    *res = atoi_result_overflow;
                    return UINT32_MAX;
                }
                str++;
                i = i * 10 + num;
            }
        }
    } else {
        // Less than 8 digits can't overflow, simpler logic here.
        while (parse_digit_u32(*str, &i)) str++;
    }
    *endptr = (char *)str;
    *res = atoi_result_suc;
    return i;
}

int32_t atoi_i32_lemire(const char *str, size_t len, char **endptr, atoi_result *res) {
    bool sign = (*str == '-');
    str += sign;
    if (unlikely(*str <= '0' || *str > '9')) {
        if (*str == '0' && !is_digit(str[1])) {
            *endptr = (char *)str + 1;
            *res = atoi_result_suc;
        } else {
            *endptr = (char *)str;
            *res = atoi_result_fail;
        }
        return 0;
    }
    
    uint32_t i = 0;
    if (is_made_of_eight_digits_fast(str)) {
        i = i * 100000000 + parse_eight_digits_unrolled(str);
        str += 8;
        if (parse_digit_u32(*str, &i)) { // digit 9
            str++;
            const uint8_t num = *str - '0'; // digit 10
            if (num <= 9) {
                if (i > (uint32_t)INT32_MAX / 10 ||
                    ((i == (uint32_t)INT32_MAX / 10) && (num > ((uint32_t)INT32_MAX % 10) + sign)) ||
                    (is_digit(str[1]))) {
                    while (is_digit(*str)) str++;
                    *endptr = (char *)str;
                    *res = atoi_result_overflow;
                    return sign ? INT32_MIN : INT32_MAX;
                }
                str++;
                i = i * 10 + num;
            }
        }
    } else {
        // Less than 8 digits can't overflow, simpler logic here.
        while (parse_digit_u32(*str, &i)) str++;
    }
    *endptr = (char *)str;
    *res = atoi_result_suc;
    return sign ? -(int32_t)i : (int32_t)i;
}

uint64_t atoi_u64_lemire(const char *str, size_t len, char **endptr, atoi_result *res) {
    if (unlikely(*str <= '0' || *str > '9')) {
        if (*str == '0' && !is_digit(str[1])) {
            *endptr = (char *)str + 1;
            *res = atoi_result_suc;
        } else {
            *endptr = (char *)str;
            *res = atoi_result_fail;
        }
        return 0;
    }
    
    uint64_t i = 0;
    if (is_made_of_eight_digits_fast(str)) {
        i = i * 100000000 + parse_eight_digits_unrolled(str);
        str += 8;
        if (is_made_of_eight_digits_fast(str)) {
            i = i * 100000000 + parse_eight_digits_unrolled(str);
            str += 8;
            if (parse_digit_u64(*str, &i)) { // digit 17
                str++;
                if (parse_digit_u64(*str, &i)) { // digit 18
                    str++;
                    if (parse_digit_u64(*str, &i)) { // digit 19
                        str++;
                        const uint8_t num = *str - '0'; // digit 20
                        if (num <= 9) {
                            if (i > UINT64_MAX / 10 ||
                                ((i == UINT64_MAX / 10) && (num > UINT64_MAX % 10)) ||
                                (is_digit(str[1]))) {
                                while (is_digit(*str)) str++;
                                *endptr = (char *)str;
                                *res = atoi_result_overflow;
                                return UINT64_MAX;
                            }
                            str++;
                            i = i * 10 + num;
                        }
                    }
                }
            }
        } else {
            // Less than 16 digit can't overflow, simpler logic here.
            while (parse_digit_u64(*str, &i)) str++;
        }
    } else {
        // Less than 8 digits can't overflow, simpler logic here.
        while (parse_digit_u64(*str, &i)) str++;
    }
    *endptr = (char *)str;
    *res = atoi_result_suc;
    return i;
}

int64_t atoi_i64_lemire(const char *str, size_t len, char **endptr, atoi_result *res) {
    bool sign = (*str == '-');
    str += sign;
    if (unlikely(*str <= '0' || *str > '9')) {
        if (*str == '0' && !is_digit(str[1])) {
            *endptr = (char *)str + 1;
            *res = atoi_result_suc;
        } else {
            *endptr = (char *)str;
            *res = atoi_result_fail;
        }
        return 0;
    }
    
    uint64_t i = 0;
    if (is_made_of_eight_digits_fast(str)) {
        i = i * 100000000 + parse_eight_digits_unrolled(str);
        str += 8;
        if (is_made_of_eight_digits_fast(str)) {
            i = i * 100000000 + parse_eight_digits_unrolled(str);
            str += 8;
            if (parse_digit_u64(*str, &i)) { // digit 17
                str++;
                if (parse_digit_u64(*str, &i)) { // digit 18
                    str++;
                    const uint8_t num = *str - '0'; // digit 19
                    if (num <= 9) {
                        if (i > (uint64_t)INT64_MAX / 10 ||
                            ((i == (uint64_t)INT64_MAX / 10) && (num > ((uint64_t)INT64_MAX % 10) + sign)) ||
                            (is_digit(str[1]))) {
                            while (is_digit(*str)) str++;
                            *endptr = (char *)str;
                            *res = atoi_result_overflow;
                            return sign ? INT64_MAX : INT64_MAX;
                        }
                        str++;
                        i = i * 10 + num;
                    }
                    
                }
            }
        } else {
            // Less than 16 digit can't overflow, simpler logic here.
            while (parse_digit_u64(*str, &i)) str++;
        }
    } else {
        // Less than 8 digits can't overflow, simpler logic here.
        while (parse_digit_u64(*str, &i)) str++;
    }
    *endptr = (char *)str;
    *res = atoi_result_suc;
    return sign ? -(int64_t)i : (int64_t)i;
}
