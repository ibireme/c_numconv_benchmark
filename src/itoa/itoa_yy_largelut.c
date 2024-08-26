/*
 * Integer to ASCII conversion (ANSI C)
 *
 * This itoa implementation uses large lookup tables (50 KB).
 * It has fewer instructions and fewer branches, so it may get excellent result
 * in some benchmarks. However, large LUTs are unacceptable in many cases.
 *
 * Description
 *     The itoa function converts an integer value to a character string in
 *     decimal and stores the result in the buffer. If value is negative, the
 *     resulting string is preceded with a minus sign (-).
 *
 * Parameters
 *     val: Value to be converted.
 *     buf: Buffer that holds the result of the conversion.
 *
 * Return Value
 *     A pointer to the end of resulting string.
 *
 * Notice
 *     The resulting string is not null-terminated.
 *     The buffer should be large enough to hold any possible result:
 *         uint32_t: 10 bytes
 *         uint64_t: 20 bytes
 *         int32_t: 11 bytes
 *         int64_t: 20 bytes
 *
 * Copyright (c) 2018 YaoYuan <ibireme@gmail.com>.
 * Released under the MIT license (MIT).
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

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

#if !defined(table_align)
#   if clang_attribute(aligned) || defined(__GNUC__)
#       define table_align(x) __attribute__((aligned(x)))
#   elif defined(_MSC_VER)
#       define table_align(x) __declspec(align(x))
#   else
#       define table_align(x)
#   endif
#endif

#define C3(x, y, z) \
    x, y, z, '0', x, y, z, '1', x, y, z, '2', x, y, z, '3', x, y, z, '4', \
    x, y, z, '5', x, y, z, '6', x, y, z, '7', x, y, z, '8', x, y, z, '9'

#define C2(x, y) \
    C3(x, y, '0'), C3(x, y, '1'), C3(x, y, '2'), C3(x, y, '3'), C3(x, y, '4'), \
    C3(x, y, '5'), C3(x, y, '6'), C3(x, y, '7'), C3(x, y, '8'), C3(x, y, '9')

#define C1(x) \
    C2(x, '0'), C2(x, '1'), C2(x, '2'), C2(x, '3'), C2(x, '4'), \
    C2(x, '5'), C2(x, '6'), C2(x, '7'), C2(x, '8'), C2(x, '9')

#define R10(x) \
    x, x, x, x, x, x, x, x, x, x

#define R90(x) \
    R10(x), R10(x), R10(x), R10(x), R10(x), R10(x), R10(x), R10(x), R10(x)

#define R900(x) \
    R90(x), R90(x), R90(x), R90(x), R90(x), \
    R90(x), R90(x), R90(x), R90(x), R90(x)

#define R9000(x) \
    R900(x), R900(x), R900(x), R900(x), R900(x), \
    R900(x), R900(x), R900(x), R900(x), R900(x)

/* Lookup table for 4 digits (40000 bytes): */
/* { "0000" "0001" "0002" ... "9999" }      */
table_align(4)
static const char digit_table[] = {
    C1('0'), C1('1'), C1('2'), C1('3'), C1('4'),
    C1('5'), C1('6'), C1('7'), C1('8'), C1('9')
};

/* Lookup table for 4 digits leading zero count (10000 bytes):  */
/* { [0...9]=3, [10...99]=2, [100...999]=1, [1000...9999]=0 }   */
static const uint8_t lz_table[] = {
    R10(3), R90(2), R900(1), R9000(0)
};

static force_inline void byte_copy_4(void *dst, const void *src) {
    memcpy(dst, src, 4);
}

static force_inline char *itoa_u32_impl(uint32_t val, char *buf) {
    /* The maximum value of uint32_t is 4294967295 (10 digits). */
    /* These digits are named as 'aabbccddee' here.             */
    uint32_t aa, aabb, bbcc, ccdd, ddee, bbccddee;
    
    /* Leading zero count in the first quad.                    */
    uint32_t lz;
    
    /* Although most compilers may convert the "division by     */
    /* constant value" into "multiply and shift", manual        */
    /* conversion can still help some compilers generate        */
    /* fewer and better instructions.                           */
    
    if (val < 10000) { /* 1-4 digits: aabb */
        lz = lz_table[val];
        byte_copy_4(buf, digit_table + val * 4 + lz);
        buf += 4 - lz;
        return buf;
        
    } else if (val < 100000000) { /* 5-8 digits: aabbccdd */
        /* (val / 10000) */
        aabb = (uint32_t)(((uint64_t)val * 109951163) >> 40);
        ccdd = val - aabb * 10000; /* val % 10000 */
        lz = lz_table[aabb];
        byte_copy_4(buf, digit_table + aabb * 4 + lz);
        buf += 4 - lz;
        byte_copy_4(buf, digit_table + ccdd * 4);
        return buf + 4;
        
    } else { /* 9-10 digits: aabbccddee */
        /* (val / 100000000) */
        aa = (uint32_t)(((uint64_t)val * 1441151881) >> 57);
        bbccddee = val - aa * 100000000; /* (val % 100000000) */
        /* (bbccddee / 10000) */
        bbcc = (uint32_t)(((uint64_t)bbccddee * 109951163) >> 40);
        ddee = bbccddee - bbcc * 10000; /* (bbccddee % 10000) */
        lz = lz_table[aa];
        byte_copy_4(buf, digit_table + aa * 4 + lz);
        buf += 4 - lz;
        byte_copy_4(buf + 0, digit_table + bbcc * 4);
        byte_copy_4(buf + 4, digit_table + ddee * 4);
        return buf + 8;
    }
}

char *itoa_u32_yy_largelut(uint32_t val, char *buf) {
    return itoa_u32_impl(val, buf);
}

char *itoa_i32_yy_largelut(int32_t val, char *buf) {
    uint32_t pos = (uint32_t)val;
    uint32_t neg = ~pos + 1;
    size_t sign = val < 0;
    *buf = '-';
    return itoa_u32_impl(sign ? neg : pos, buf + sign);
}


static force_inline char *itoa_u64_impl_len_1_to_8(uint32_t val, char *buf) {
    uint32_t aabb, ccdd, lz;
    
    if (val < 10000) { /* 1-4 digits: aabb */
        lz = lz_table[val];
        byte_copy_4(buf, digit_table + val * 4 + lz);
        buf += 4 - lz;
        return buf;
        
    } else { /* 5-8 digits: aabbccdd */
        /* (val / 10000) */
        aabb = (uint32_t)(((uint64_t)val * 109951163) >> 40);
        ccdd = val - aabb * 10000; /* (val % 10000) */
        lz = lz_table[aabb];
        byte_copy_4(buf, digit_table + aabb * 4 + lz);
        buf += 4 - lz;
        byte_copy_4(buf, digit_table + ccdd * 4);
        return buf + 4;
    }
}

static force_inline char *itoa_u64_impl_len_5_to_8(uint32_t val, char *buf) {
    uint32_t aabb, ccdd, lz;
    aabb = (uint32_t)(((uint64_t)val * 109951163) >> 40); /* (val / 10000) */
    ccdd = val - aabb * 10000; /* (val % 10000) */
    lz = lz_table[aabb];
    byte_copy_4(buf, digit_table + aabb * 4 + lz);
    buf += 4 - lz;
    byte_copy_4(buf, digit_table + ccdd * 4);
    return buf + 4;
}

static force_inline char *itoa_u64_impl_len_4(uint32_t val, char *buf) {
    byte_copy_4(buf, digit_table + val * 4);
    return buf + 4;
}

static force_inline char *itoa_u64_impl_len_8(uint32_t val, char *buf) {
    uint32_t aabb, ccdd;
    aabb = (uint32_t)(((uint64_t)val * 109951163) >> 40); /* (val / 10000) */
    ccdd = val - aabb * 10000; /* (val % 10000) */
    byte_copy_4(buf + 0, digit_table + aabb * 4);
    byte_copy_4(buf + 4, digit_table + ccdd * 4);
    return buf + 8;
}

static force_inline char *itoa_u64_impl(uint64_t val, char *buf) {
    uint64_t tmp, hgh;
    uint32_t mid, low;
    
    if (val < 100000000) { /* 1-8 digits */
        buf = itoa_u64_impl_len_1_to_8((uint32_t)val, buf);
        return buf;
        
    } else if (val < (uint64_t)100000000 * 100000000) { /* 9-16 digits */
        hgh = val / 100000000;
        low = (uint32_t)(val - hgh * 100000000); /* (val % 100000000) */
        buf = itoa_u64_impl_len_1_to_8((uint32_t)hgh, buf);
        buf = itoa_u64_impl_len_8(low, buf);
        return buf;
        
    } else { /* 17-20 digits */
        tmp = val / 100000000;
        low = (uint32_t)(val - tmp * 100000000); /* (val % 100000000) */
        hgh = (uint32_t)(tmp / 10000);
        mid = (uint32_t)(tmp - hgh * 10000); /* (tmp % 10000) */
        buf = itoa_u64_impl_len_5_to_8((uint32_t)hgh, buf);
        buf = itoa_u64_impl_len_4(mid, buf);
        buf = itoa_u64_impl_len_8(low, buf);
        return buf;
    }
}

char *itoa_u64_yy_largelut(uint64_t val, char *buf) {
    return itoa_u64_impl(val, buf);
}

char *itoa_i64_yy_largelut(int64_t val, char *buf) {
    uint64_t pos = (uint64_t)val;
    uint64_t neg = ~pos + 1;
    size_t sign = val < 0;
    *buf = '-';
    return itoa_u64_impl(sign ? neg : pos, buf + sign);
}


/* benckmark config */
int itoa_yy_largelut_available_32 = 1;
int itoa_yy_largelut_available_64 = 1;
