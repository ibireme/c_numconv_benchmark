/*
 Source: https://github.com/miloyip/itoa-benchmark/blob/master/src/tmueller.cpp
 License: (maybe) https://github.com/miloyip/itoa-benchmark/blob/master/license.txt
 
 Code is modified for benchmark.
 Require __builtin_clzll for 64-bit integers.
 */

#include <stdint.h>
#include <string.h>


#if defined(__GNUC__) || defined(__clang__)
#define t_likely(expr)   __builtin_expect(expr, 1)
#define t_unlikely(expr) __builtin_expect(expr, 0)
#else
#define t_likely(expr)   (expr)
#define t_unlikely(expr) (expr)
#endif

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


static const char DIGITS[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

static force_inline char *uint32toa_tmueller(uint64_t x, char* out) {
    char* str = out;
    if (t_likely(x < 100000)) {
        if (t_likely(x) < 1000) {
            if (t_likely(x) < 10) {
                *str++ = (char) ('0' + x);
                // *str = 0;
                return str;
            }
            uint32_t inc = 0;
            x = (x * ((0xffffUL / 100UL) + 1));
            uint32_t d;
            d = (uint32_t)(x >> 16); *str = (char) ('0' | d); inc |= (uint32_t)(-(int32_t)d); str += inc >> 31;
            x = (x & 0xffffUL) * 10;
            d = (uint32_t)(x >> 16); *str = (char) ('0' | d); inc |= (uint32_t)(-(int32_t)d); str += inc >> 31;
            x = (x & 0xffffUL) * 10;
            *str++ = (char) ('0' + (x >> 16));
            // *str = 0;
            return str;
        } else {
            uint32_t inc = 0;
            x = (x * ((0xffffffffUL / 10000UL) + 1));
            uint32_t d;
            d = (x >> 32); *str = (char) ('0' | d); inc |= (uint32_t)(-(int32_t)d); str += inc >> 31;
            x = (x & 0xffffffffUL) * 100;
            memcpy(str, DIGITS + (x >> 32) * 2, 2); str += 2;
            x = (x & 0xffffffffUL) * 100;
            memcpy(str, DIGITS + (x >> 32) * 2, 2); str += 2;
            // *str = 0;
            return str;
        }
    } else {
        if (t_likely(x < 10000000)) {
            uint32_t inc = 0;
            x = (x * ((0xfffffffffffUL / 1000000) + 1));
            uint32_t d;
            d = (x >> 44); *str = (char) ('0' | d); inc |= (uint32_t)(-(int32_t)d); str += inc >> 31;
            x = (x & 0xfffffffffffUL) * 100;
            memcpy(str, DIGITS + (x >> 44) * 2, 2); str += 2;
            x = (x & 0xfffffffffffUL) * 100;
            memcpy(str, DIGITS + (x >> 44) * 2, 2); str += 2;
            x = (x & 0xfffffffffffUL) * 100;
            memcpy(str, DIGITS + (x >> 44) * 2, 2); str += 2;
            // *str = 0;
            return str;
        } else {
            uint32_t inc = 0;
            x = (((x * 2305843009L) >> 29) + 4);
            uint32_t d;
            d = (x >> 32); *str = (char) ('0' | d); inc |= (uint32_t)(-(int32_t)d); str += inc >> 31;
            x = (x & 0xffffffffUL) * 10;
            d = (x >> 32); *str = (char) ('0' | d); inc |= (uint32_t)(-(int32_t)d); str += inc >> 31;
            x = (x & 0xffffffffUL) * 100;
            memcpy(str, DIGITS + (x >> 32) * 2, 2); str += 2;
            x = (x & 0xffffffffUL) * 100;
            memcpy(str, DIGITS + (x >> 32) * 2, 2); str += 2;
            x = (x & 0xffffffffUL) * 100;
            memcpy(str, DIGITS + (x >> 32) * 2, 2); str += 2;
            x = (x & 0xffffffffUL) * 100;
            memcpy(str, DIGITS + (x >> 32) * 2, 2); str += 2;
            // *str = 0;
            return str;
        }
    }
}

char *itoa_u32_tmueller(uint32_t v, char* out) {
    return uint32toa_tmueller(v, out);
}

char *itoa_i32_tmueller( int32_t v, char* out) {
    // branchless (from amartin)
    *out = '-';
    uint32_t mask = v < 0 ? ~(int32_t) 0 : 0;
    uint32_t u = ((2 * (uint32_t)(v)) & ~mask) - v;
    out += mask & 1;
    uint64_t x = u;
    return uint32toa_tmueller(x, out);
}

int itoa_tmueller_available_32 = 1;



#if (defined(__GNUC__) || defined(__clang__))

static const uint64_t POW_10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
    100000000, 1000000000, 10000000000ULL, 100000000000ULL, 1000000000000ULL, 10000000000000ULL,
    100000000000000ULL, 1000000000000000ULL, 10000000000000000ULL, 100000000000000000ULL,
    1000000000000000000ULL, 10000000000000000000ULL };

char *itoa_u64_tmueller(uint64_t v, char* out) {
    if(v < 10) {
        *out++ = '0' + v;
        // *out = 0;
        return out;
    }
    int zeros = 64 - __builtin_clzll(v);
    int len = (1233 * zeros) >> 12;
    uint64_t p10 = POW_10[len];
    if (v >= p10) {
        len++;
    }
    out += len;
    char *end = out;
    
    while (v >= 100) {
        uint64_t d100 = v / 100;
        uint64_t index = v - d100 * 100;
        v = d100;
        out -= 2;
        memcpy(out, DIGITS + index * 2, 2);
    }
    if (v < 10) {
        *--out = '0' + v;
        return end;
    }
    out -= 2;
    memcpy(out, DIGITS + v * 2, 2);
    return end;
}

char *itoa_i64_tmueller( int64_t v, char* out) {
    // branchless (from amartin)
    *out = '-';
    uint64_t mask = v < 0 ? ~(int64_t) 0 : 0;
    uint64_t u = ((2 * (uint64_t)(v)) & ~mask) - v;
    out += mask & 1;
    return itoa_u64_tmueller(u, out);
}

int itoa_tmueller_available_64 = 1;

#else
char *itoa_u64_tmueller(uint64_t v, char* out) { return out; }
char *itoa_i64_tmueller(int64_t v, char* out) { return out; }
int itoa_tmueller_available_64 = 0;

#endif
