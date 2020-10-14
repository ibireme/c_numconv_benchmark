/*
 Source: https://github.com/miloyip/itoa-benchmark/blob/master/src/unrolledlut.cpp
 License: MIT
 
 Code is modified for benchmark.
 */


#if defined(__clang__)
#   define clang_attribute __has_attribute
#else
#   define clang_attribute(x) 0
#endif

#if !defined(force_inline)
#   if clang_attribute(always_inline) || (defined(__GNUC__) && __GNUC__ >= 4)
#       define force_inline __inline__ __attribute__((always_inline))
#   elif defined(__clang__) || defined(__GNUC__)
#       define force_inline __inline__
#   elif defined(_MSC_VER) && _MSC_VER >= 1200
#       define force_inline __forceinline
#   elif defined(_MSC_VER)
#       define force_inline __inline
#   elif defined(__cplusplus) || (defined(__STDC__) && __STDC__ && \
        defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#       define force_inline inline
#   else
#       define force_inline
#   endif
#endif



// unrolledlut.cpp: Fast integer to string conversion by using per-digit-count unrolling and a lookuptable
//
// ===-------- DESCRIPTION --------===
//
// Very fast implementation of uint32_t to string:
// - Automatically takes advantage of two-byte load/store on
//   architectures that support it (memcpy will be optimized).
// - Avoids as many jumps as possible, by unrolling the whole thing for every digit count.
// - Con: Costs some memory for the duplicated instructions of all branches
//
// Further optimization possible:
// - You may reorder the digit-cases, so that the most
//   commonly used cases come first. Currently digit-counts
//   from 7 to 10 are processed first, as they cover ~99.7% of all uint32_t values.
//   By reordering these for your specific needs, you can save one or two extra instructions for these cases.
//
// ===-------- LICENSE --------===
//
// The MIT License (MIT)
//
// Copyright (c) 2017 nyronium (nyronium@genthree.io)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static const char TWO_DIGITS_TO_STR[201] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";


#define COPY_2_DIGITS(out, value) \
    memcpy(out, &((const uint16_t*)(TWO_DIGITS_TO_STR))[value], 2); \
    out += 2;

#define COPY_1_DIGIT(out, value) \
    *out++ = '0' + value;


#define UNROLL_EXACT_DIGITS_8(out, value) {                   \
    uint32_t digits;                                          \
    digits = value  / 1000000;    COPY_2_DIGITS(out, digits); \
    value -= digits * 1000000;                                \
    digits = value  / 10000;      COPY_2_DIGITS(out, digits); \
    value -= digits * 10000;                                  \
    digits = value  / 100;        COPY_2_DIGITS(out, digits); \
    value -= digits * 100;                                    \
                                  COPY_2_DIGITS(out, value);  \
    /* *out = '\0'; */                                              \
}

#define UNROLL_REMAINING_DIGITS_8(out, value, digits) {       \
    value -= digits * 100000000;                              \
    digits = value  / 1000000;    COPY_2_DIGITS(out, digits); \
    value -= digits * 1000000;                                \
    digits = value  / 10000;      COPY_2_DIGITS(out, digits); \
    value -= digits * 10000;                                  \
    digits = value  / 100;        COPY_2_DIGITS(out, digits); \
    value -= digits * 100;                                    \
                                  COPY_2_DIGITS(out, value);  \
    /* *out = '\0'; */ return out;                                  \
}

#define UNROLL_REMAINING_DIGITS_6(out, value, digits) {       \
    value -= digits * 1000000;                                \
    digits = value  / 10000;      COPY_2_DIGITS(out, digits); \
    value -= digits * 10000;                                  \
    digits = value  / 100;        COPY_2_DIGITS(out, digits); \
    value -= digits * 100;                                    \
                                  COPY_2_DIGITS(out, value);  \
    /* *out = '\0'; */ return out;                                  \
}

#define UNROLL_REMAINING_DIGITS_4(out, value, digits) {       \
    value -= digits * 10000;                                  \
    digits = value  / 100;        COPY_2_DIGITS(out, digits); \
    value -= digits * 100;                                    \
                                  COPY_2_DIGITS(out, value);  \
    /* *out = '\0'; */ return out;                                  \
}

#define UNROLL_REMAINING_DIGITS_2(out, value, digits) {       \
    value -= digits * 100;                                    \
                                  COPY_2_DIGITS(out, value);  \
    /* *out = '\0'; */ return out;                                  \
}

#define UNROLL_REMAINING_DIGITS_0(out, value) {               \
    /* *out = '\0'; */ return out;                                  \
}


#define UNROLL_DIGIT_PAIR_9_10(out, value) {                      \
    uint32_t digits;                                              \
    if (value >= 1000000000) {                                    \
        digits = value  / 100000000;  COPY_2_DIGITS(out, digits); \
        UNROLL_REMAINING_DIGITS_8(out, value, digits);            \
    } else {                                                      \
        digits = value  / 100000000;  COPY_1_DIGIT(out, digits);  \
        UNROLL_REMAINING_DIGITS_8(out, value, digits);            \
    }                                                             \
}

#define UNROLL_DIGIT_PAIR_7_8(out, value) {                       \
    uint32_t digits;                                              \
    if (value >= 10000000) {                                      \
        digits = value  / 1000000;    COPY_2_DIGITS(out, digits); \
        UNROLL_REMAINING_DIGITS_6(out, value, digits);            \
    } else {                                                      \
        digits = value  / 1000000;    COPY_1_DIGIT(out, digits);  \
        UNROLL_REMAINING_DIGITS_6(out, value, digits);            \
    }                                                             \
}

#define UNROLL_DIGIT_PAIR_5_6(out, value) {                       \
    uint32_t digits;                                              \
    if (value >= 100000) {                                        \
        digits = value  / 10000;      COPY_2_DIGITS(out, digits); \
        UNROLL_REMAINING_DIGITS_4(out, value, digits);            \
    } else {                                                      \
        digits = value  / 10000;      COPY_1_DIGIT(out, digits);  \
        UNROLL_REMAINING_DIGITS_4(out, value, digits);            \
    }                                                             \
}

#define UNROLL_DIGIT_PAIR_3_4(out, value) {                       \
    uint32_t digits;                                              \
    if (value >= 1000) {                                          \
        digits = value  / 100;        COPY_2_DIGITS(out, digits); \
        UNROLL_REMAINING_DIGITS_2(out, value, digits);            \
    } else {                                                      \
        digits = value  / 100;        COPY_1_DIGIT(out, digits);  \
        UNROLL_REMAINING_DIGITS_2(out, value, digits);            \
    }                                                             \
}

#define UNROLL_DIGIT_PAIR_1_2(out, value) {                       \
    if (value >= 10) {                                            \
                                      COPY_2_DIGITS(out, value);  \
        UNROLL_REMAINING_DIGITS_0(out, value);                    \
    } else {                                                      \
                                      COPY_1_DIGIT(out, value);   \
        UNROLL_REMAINING_DIGITS_0(out, value);                    \
    }                                                             \
}

static force_inline char *unrolledlut(uint32_t value, char* out) {
    if (value >= 100000000) {
        UNROLL_DIGIT_PAIR_9_10(out, value);
    } else if (value >= 1000000) {
        UNROLL_DIGIT_PAIR_7_8(out, value);
    } else if (value <  100) {
        UNROLL_DIGIT_PAIR_1_2(out, value);
    } else if (value <  10000) {
        UNROLL_DIGIT_PAIR_3_4(out, value);
    } else { /* (value <  1000000) */
        UNROLL_DIGIT_PAIR_5_6(out, value);
    }
}

static force_inline char* unrolledlut64(uint64_t value, char* buffer) {
    uint32_t least_significant = (uint32_t)(value);
    if (least_significant == value) {
        return unrolledlut(least_significant, buffer);
    }
    
    uint64_t high12 = value / 100000000;

    /* optimized unrolled recursion */
    least_significant = (uint32_t)(high12);
    if (least_significant == high12) {
        buffer = unrolledlut(least_significant, buffer);
    } else {
        uint64_t high4 = high12 / 100000000;
        buffer = unrolledlut((uint32_t)high4, buffer);

        uint32_t digits_15_8 = (uint32_t)(high12 - (high4 * 100000000));
        UNROLL_EXACT_DIGITS_8(buffer, digits_15_8);
    }

    uint32_t digits_7_0 = (uint32_t)(value - (high12 * 100000000));
    UNROLL_EXACT_DIGITS_8(buffer, digits_7_0);
    return buffer;
}

#undef UNROLL_DIGIT_PAIR_1_2
#undef UNROLL_DIGIT_PAIR_3_4
#undef UNROLL_DIGIT_PAIR_5_6
#undef UNROLL_DIGIT_PAIR_7_8
#undef UNROLL_DIGIT_PAIR_9_10

#undef UNROLL_REMAINING_DIGITS_0
#undef UNROLL_REMAINING_DIGITS_2
#undef UNROLL_REMAINING_DIGITS_4
#undef UNROLL_REMAINING_DIGITS_6
#undef UNROLL_REMAINING_DIGITS_8
#undef UNROLL_EXACT_DIGITS_8

#undef COPY_1_DIGIT
#undef COPY_2_DIGITS


char *itoa_u32_unrolledlut(uint32_t value, char* buffer) {
    return unrolledlut(value, buffer);
}

char *itoa_i32_unrolledlut(int32_t value, char* buffer) {
    uint32_t uvalue = (uint32_t)(value);
    if (value < 0) {
        *buffer++ = '-';
        uvalue = (uint32_t)-(int32_t)uvalue;
    }

    return unrolledlut(uvalue, buffer);
}

char *itoa_u64_unrolledlut(uint64_t value, char* buffer) {
    return unrolledlut64(value, buffer);
}

char *itoa_i64_unrolledlut(int64_t value, char* buffer) {
    uint64_t uvalue = (uint64_t)(value);
    if (value < 0) {
        *buffer++ = '-';
        uvalue = (uint64_t)-(int64_t)uvalue;
    }

    return unrolledlut64(uvalue, buffer);
}

/* benckmark config */
int itoa_unrolledlut_available_32 = 1;
int itoa_unrolledlut_available_64 = 1;
