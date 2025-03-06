/*
 Code from https://gitee.com/xjb714/dtoa-benchmark/tree/master/src/d2sci
 License MIT: https://gitee.com/xjb714/dtoa-benchmark/blob/master/LICENSE
 Code is modified to return ending pointer.
*/

#ifndef _MSC_VER
#if HAVE_SSE2

#include <immintrin.h>

// ================================ itoa.cpp =================================

#include <stddef.h>
#include <stdint.h>
#include <string.h>
// #include "test.h"

#if defined(__has_attribute)
#define yy_attribute(x) __has_attribute(x)
#else
#define yy_attribute(x) 0
#endif

#if !defined(force_inline)
#if yy_attribute(always_inline) || (defined(__GNUC__) && __GNUC__ >= 4)
#define force_inline __inline__ __attribute__((always_inline))
#elif defined(__clang__) || defined(__GNUC__)
#define force_inline __inline__
#elif defined(_MSC_VER) && _MSC_VER >= 1200
#define force_inline __forceinline
#elif defined(_MSC_VER)
#define force_inline __inline
#elif defined(__cplusplus) || \
    (defined(__STDC__) && __STDC__ && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define force_inline inline
#else
#define force_inline
#endif
#endif

#if !defined(table_align)
#if yy_attribute(aligned) || defined(__GNUC__)
#define table_align(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#define table_align(x) __declspec(align(x))
#else
#define table_align(x)
#endif
#endif

table_align(2) static const char digit_table[200] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9', '1', '0', '1',
    '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2', '2',
    '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9', '3', '0', '3', '1', '3', '2', '3', '3', '3',
    '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5',
    '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5',
    '7', '5', '8', '5', '9', '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8',
    '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8',
    '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', '9', '0', '9', '1',
    '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'};

static force_inline void byte_copy_2(void *dst, const void *src) { memcpy(dst, src, 2); }

static force_inline char *itoa_u32_impl(uint32_t val, char *buf)
{
    /* The maximum value of uint32_t is 4294967295 (10 digits), */
    /* these digits are named as 'aabbccddee' here.             */
    uint32_t aa, bb, cc, dd, ee, aabb, bbcc, ccdd, ddee, aabbcc;

    /* Leading zero count in the first pair.                    */
    uint32_t lz;

    /* Although most compilers may convert the "division by     */
    /* constant value" into "multiply and shift", manual        */
    /* conversion can still help some compilers generate        */
    /* fewer and better instructions.                           */

    if (val < 100)
    { /* 1-2 digits: aa */
        lz = val < 10;
        byte_copy_2(buf + 0, digit_table + val * 2 + lz);
        buf -= lz;
        return buf + 2;
    }
    else if (val < 10000)
    {                            /* 3-4 digits: aabb */
        aa = (val * 5243) >> 19; /* (val / 100) */
        bb = val - aa * 100;     /* (val % 100) */
        lz = aa < 10;
        byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
        buf -= lz;
        byte_copy_2(buf + 2, digit_table + bb * 2);
        return buf + 4;
    }
    else if (val < 1000000)
    {                                                    /* 5-6 digits: aabbcc */
        aa = (uint32_t)(((uint64_t)val * 429497) >> 32); /* (val / 10000) */
        bbcc = val - aa * 10000;                         /* (val % 10000) */
        bb = (bbcc * 5243) >> 19;                        /* (bbcc / 100) */
        cc = bbcc - bb * 100;                            /* (bbcc % 100) */
        lz = aa < 10;
        byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
        buf -= lz;
        byte_copy_2(buf + 2, digit_table + bb * 2);
        byte_copy_2(buf + 4, digit_table + cc * 2);
        return buf + 6;
    }
    else if (val < 100000000)
    { /* 7~8 digits: aabbccdd */
        /* (val / 10000) */
        aabb = (uint32_t)(((uint64_t)val * 109951163) >> 40);
        ccdd = val - aabb * 10000; /* (val % 10000) */
        aa = (aabb * 5243) >> 19;  /* (aabb / 100) */
        cc = (ccdd * 5243) >> 19;  /* (ccdd / 100) */
        bb = aabb - aa * 100;      /* (aabb % 100) */
        dd = ccdd - cc * 100;      /* (ccdd % 100) */
        lz = aa < 10;
        byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
        buf -= lz;
        byte_copy_2(buf + 2, digit_table + bb * 2);
        byte_copy_2(buf + 4, digit_table + cc * 2);
        byte_copy_2(buf + 6, digit_table + dd * 2);
        return buf + 8;
    }
    else
    { /* 9~10 digits: aabbccddee */
        /* (val / 10000) */
        aabbcc = (uint32_t)(((uint64_t)val * 3518437209ul) >> 45);
        /* (aabbcc / 10000) */
        aa = (uint32_t)(((uint64_t)aabbcc * 429497) >> 32);
        ddee = val - aabbcc * 10000; /* (val % 10000) */
        bbcc = aabbcc - aa * 10000;  /* (aabbcc % 10000) */
        bb = (bbcc * 5243) >> 19;    /* (bbcc / 100) */
        dd = (ddee * 5243) >> 19;    /* (ddee / 100) */
        cc = bbcc - bb * 100;        /* (bbcc % 100) */
        ee = ddee - dd * 100;        /* (ddee % 100) */
        lz = aa < 10;
        byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
        buf -= lz;
        byte_copy_2(buf + 2, digit_table + bb * 2);
        byte_copy_2(buf + 4, digit_table + cc * 2);
        byte_copy_2(buf + 6, digit_table + dd * 2);
        byte_copy_2(buf + 8, digit_table + ee * 2);
        return buf + 10;
    }
}

static force_inline char *itoa_u64_impl_len_8(uint32_t val, char *buf)
{
    uint32_t aa, bb, cc, dd, aabb, ccdd;
    // aabb = (uint32_t)(((uint64_t)val * 109951163) >> 40); /* (val / 10000) */
    // ccdd = val - aabb * 10000;                            /* (val % 10000) */
    unsigned long long h4, l4, low4;
    l4 = _mulx_u64(val, 1844674407370956, &h4);
    aabb = h4;
    _mulx_u64(l4, 10000, &low4);
    ccdd = low4;

    aa = (aabb * 5243) >> 19; /* (aabb / 100) */
    cc = (ccdd * 5243) >> 19; /* (ccdd / 100) */
    bb = aabb - aa * 100;     /* (aabb % 100) */
    dd = ccdd - cc * 100;     /* (ccdd % 100) */
    byte_copy_2(buf + 0, digit_table + aa * 2);
    byte_copy_2(buf + 2, digit_table + bb * 2);
    byte_copy_2(buf + 4, digit_table + cc * 2);
    byte_copy_2(buf + 6, digit_table + dd * 2);
    return buf + 8;
}

static force_inline char *itoa_u64_impl_len_4(uint32_t val, char *buf)
{
    uint32_t aa, bb;
    // aa = (val * 5243) >> 19; /* (val / 100) */
    // bb = val - aa * 100;     /* (val % 100) */
    unsigned long long b, c, d1, d2;
    unsigned long long low64 = _mulx_u64(val, 184467440737095517, &d1); // val/100
    _mulx_u64(low64, 200, &d2);
    aa = d1;
    uint32_t bb2 = d2;

    byte_copy_2(buf + 0, digit_table + aa * 2);
    byte_copy_2(buf + 2, digit_table + bb2);
    return buf + 4;
}

static force_inline char *itoa_u64_impl_len_1_to_8(uint32_t val, char *buf)
{
    uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

    if (val < 100)
    { /* 1-2 digits: aa */
        lz = val < 10;
        byte_copy_2(buf + 0, digit_table + val * 2 + lz);
        buf -= lz;
        return buf + 2;
    }
    else if (val < 10000)
    { /* 3-4 digits: aabb */
        // aa = (val * 5243) >> 19; /* (val / 100) */
        // bb = val - aa * 100;     /* (val % 100) */

        unsigned long long b, c, d1, d2;
        unsigned long long low64 = _mulx_u64(val, 184467440737095517, &d1); // val/100
        _mulx_u64(low64, 200, &d2);
        aa = d1;
        uint32_t bb2 = d2;

        lz = aa < 10;
        byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
        buf -= lz;
        byte_copy_2(buf + 2, digit_table + bb2);
        return buf + 4;
    }
    else if (val < 1000000)
    { /* 5-6 digits: aabbcc */
        // aa = (uint32_t)(((uint64_t)val * 429497) >> 32); /* (val / 10000) */
        // bbcc = val - aa * 10000;                         /* (val % 10000) */
        // bb = (bbcc * 5243) >> 19;                        /* (bbcc / 100) */
        // cc = bbcc - bb * 100;                            /* (bbcc % 100) */
        unsigned long long t1, t2, t3, low23, low3;
        low23 = _mulx_u64(val, 1844674407370956, &t1);
        low3 = _mulx_u64(low23, 100, &t2);
        _mulx_u64(low3, 200, &t3);
        aa = t1;
        bb = t2;
        uint32_t cc2 = t3;

        lz = aa < 10;
        byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
        buf -= lz;
        byte_copy_2(buf + 2, digit_table + bb * 2);
        byte_copy_2(buf + 4, digit_table + cc2);
        return buf + 6;
    }
    else
    { /* 7-8 digits: aabbccdd */
        unsigned long long h4, l4, low4;
        /* (val / 10000) */
        // aabb = (uint32_t)(((uint64_t)val * 109951163) >> 40);
        l4 = _mulx_u64(val, 1844674407370956, &h4);
        aabb = h4;
        // ccdd = val - aabb * 10000; /* (val % 10000) */
        _mulx_u64(l4, 10000, &low4);
        ccdd = low4;
        aa = (aabb * 5243) >> 19; /* (aabb / 100) */
        cc = (ccdd * 5243) >> 19; /* (ccdd / 100) */
        bb = aabb - aa * 100;     /* (aabb % 100) */
        dd = ccdd - cc * 100;     /* (ccdd % 100) */
        lz = aa < 10;
        byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
        buf -= lz;
        byte_copy_2(buf + 2, digit_table + bb * 2);
        byte_copy_2(buf + 4, digit_table + cc * 2);
        byte_copy_2(buf + 6, digit_table + dd * 2);
        return buf + 8;
    }
}

static force_inline char *itoa_u64_impl_len_5_to_8(uint32_t val, char *buf)
{
    uint32_t aa, bb, cc, dd, aabb, bbcc, ccdd, lz;

    if (val < 1000000)
    {                                                    /* 5-6 digits: aabbcc */
        //aa = (uint32_t)(((uint64_t)val * 429497) >> 32); /* (val / 10000) */
        //bbcc = val - aa * 10000;                         /* (val % 10000) */
        unsigned long long h4, l4, t1,t2,low2,t3;
        /* (val / 10000) */
        l4 = _mulx_u64(val, 1844674407370956, &t1);
        aa = t1;
        low2=_mulx_u64(l4, 100, &t2);
        bb = t2;
        _mulx_u64(low2, 200, &t3);
        uint32_t cc2=t3;
        //bb = (bbcc * 5243) >> 19;                        /* (bbcc / 100) */
        //cc = bbcc - bb * 100;                            /* (bbcc % 100) */
        lz = aa < 10;
        byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
        buf -= lz;
        byte_copy_2(buf + 2, digit_table + bb * 2);
        byte_copy_2(buf + 4, digit_table + cc2);
        return buf + 6;
    }
    else
    { /* 7-8 digits: aabbccdd */
        /* (val / 10000) */
        aabb = (uint32_t)(((uint64_t)val * 109951163) >> 40);
        ccdd = val - aabb * 10000; /* (val % 10000) */
        aa = (aabb * 5243) >> 19;  /* (aabb / 100) */
        cc = (ccdd * 5243) >> 19;  /* (ccdd / 100) */
        bb = aabb - aa * 100;      /* (aabb % 100) */
        dd = ccdd - cc * 100;      /* (ccdd % 100) */
        lz = aa < 10;
        byte_copy_2(buf + 0, digit_table + aa * 2 + lz);
        buf -= lz;
        byte_copy_2(buf + 2, digit_table + bb * 2);
        byte_copy_2(buf + 4, digit_table + cc * 2);
        byte_copy_2(buf + 6, digit_table + dd * 2);
        return buf + 8;
    }
}

static force_inline char *itoa_u64_impl(uint64_t val, char *buf)
{
    uint64_t tmp, hgh;
    uint32_t mid, low;

    if (val < 100000000)
    { /* 1-8 digits */
        buf = itoa_u64_impl_len_1_to_8((uint32_t)val, buf);
        return buf;
    }
    else if (val < (uint64_t)100000000 * 100000000)
    { /* 9-16 digits */
        hgh = val / 100000000;
        low = (uint32_t)(val - hgh * 100000000); /* (val % 100000000) */
        buf = itoa_u64_impl_len_1_to_8((uint32_t)hgh, buf);
        buf = itoa_u64_impl_len_8(low, buf);
        return buf;
    }
    else
    { /* 17-20 digits */
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



// ================================ itoa_sse2.cpp =================================

#include <cassert>
#include <emmintrin.h>
#include <stdint.h>
//#include "digitslut.h"
//#include "test.h"

#ifdef _MSC_VER
#include "intrin.h"
#endif

#ifdef _MSC_VER
#define ALIGN_PRE __declspec(align(16))
#define ALIGN_SUF
#else
#define ALIGN_PRE
#define ALIGN_SUF  __attribute__ ((aligned(16)))
#endif

static const uint32_t kDiv10000 = 0xd1b71759;
ALIGN_PRE static const uint32_t kDiv10000Vector[4] ALIGN_SUF = { kDiv10000, kDiv10000, kDiv10000, kDiv10000 };
ALIGN_PRE static const uint32_t k10000Vector[4] ALIGN_SUF = { 10000, 10000, 10000, 10000 };
ALIGN_PRE static const uint16_t kDivPowersVector[8] ALIGN_SUF = { 8389, 5243, 13108, 32768, 8389, 5243, 13108, 32768 }; // 10^3, 10^2, 10^1, 10^0
ALIGN_PRE static const uint16_t kShiftPowersVector[8] ALIGN_SUF = {
    1 << (16 - (23 + 2 - 16)),
    1 << (16 - (19 + 2 - 16)),
    1 << (16 - 1 - 2),
    1 << (15),
    1 << (16 - (23 + 2 - 16)),
    1 << (16 - (19 + 2 - 16)),
    1 << (16 - 1 - 2),
    1 << (15)
};
ALIGN_PRE static const uint16_t k10Vector[8] ALIGN_SUF = { 10, 10, 10, 10, 10, 10, 10, 10 };
ALIGN_PRE static const char kAsciiZero[16] ALIGN_SUF = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };
static const char gDigitsLut[200] = {
    '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
    '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
    '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
    '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
    '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
    '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
    '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
    '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
    '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
    '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
};
static inline __m128i Convert8DigitsSSE2(uint32_t value) {
    //assert(value <= 99999999);

    // abcd, efgh = abcdefgh divmod 10000 
    const __m128i abcdefgh = _mm_cvtsi32_si128(value);
    const __m128i abcd = _mm_srli_epi64(_mm_mul_epu32(abcdefgh, reinterpret_cast<const __m128i*>(kDiv10000Vector)[0]), 45);
    const __m128i efgh = _mm_sub_epi32(abcdefgh, _mm_mul_epu32(abcd, reinterpret_cast<const __m128i*>(k10000Vector)[0]));

    // v1 = [ abcd, efgh, 0, 0, 0, 0, 0, 0 ]
    const __m128i v1 = _mm_unpacklo_epi16(abcd, efgh);

    // v1a = v1 * 4 = [ abcd * 4, efgh * 4, 0, 0, 0, 0, 0, 0 ]
    const __m128i v1a = _mm_slli_epi64(v1, 2);

    // v2 = [ abcd * 4, abcd * 4, abcd * 4, abcd * 4, efgh * 4, efgh * 4, efgh * 4, efgh * 4 ]
    const __m128i v2a = _mm_unpacklo_epi16(v1a, v1a);
    const __m128i v2 = _mm_unpacklo_epi32(v2a, v2a);

    // v4 = v2 div 10^3, 10^2, 10^1, 10^0 = [ a, ab, abc, abcd, e, ef, efg, efgh ]
    const __m128i v3 = _mm_mulhi_epu16(v2, reinterpret_cast<const __m128i*>(kDivPowersVector)[0]);
    const __m128i v4 = _mm_mulhi_epu16(v3, reinterpret_cast<const __m128i*>(kShiftPowersVector)[0]);

    // v5 = v4 * 10 = [ a0, ab0, abc0, abcd0, e0, ef0, efg0, efgh0 ]
    const __m128i v5 = _mm_mullo_epi16(v4, reinterpret_cast<const __m128i*>(k10Vector)[0]);

    // v6 = v5 << 16 = [ 0, a0, ab0, abc0, 0, e0, ef0, efg0 ]
    const __m128i v6 = _mm_slli_epi64(v5, 16);

    // v7 = v4 - v6 = { a, b, c, d, e, f, g, h }
    const __m128i v7 = _mm_sub_epi16(v4, v6);

    return v7;
}

 


// ================================ d2sci2.cpp =================================

static const long long exp_result3[324 + 308 + 1] = {0x3432332d65, 0x3332332d65, 0x3232332d65, 0x3132332d65, 0x3032332d65, 0x3931332d65, 0x3831332d65, 0x3731332d65, 0x3631332d65, 0x3531332d65, 0x3431332d65, 0x3331332d65, 0x3231332d65, 0x3131332d65, 0x3031332d65, 0x3930332d65, 0x3830332d65, 0x3730332d65, 0x3630332d65, 0x3530332d65, 0x3430332d65, 0x3330332d65, 0x3230332d65, 0x3130332d65, 0x3030332d65, 0x3939322d65, 0x3839322d65, 0x3739322d65, 0x3639322d65, 0x3539322d65, 0x3439322d65, 0x3339322d65, 0x3239322d65, 0x3139322d65, 0x3039322d65, 0x3938322d65, 0x3838322d65, 0x3738322d65, 0x3638322d65, 0x3538322d65, 0x3438322d65, 0x3338322d65, 0x3238322d65, 0x3138322d65, 0x3038322d65, 0x3937322d65, 0x3837322d65, 0x3737322d65, 0x3637322d65, 0x3537322d65, 0x3437322d65, 0x3337322d65, 0x3237322d65, 0x3137322d65, 0x3037322d65, 0x3936322d65, 0x3836322d65, 0x3736322d65, 0x3636322d65, 0x3536322d65, 0x3436322d65, 0x3336322d65, 0x3236322d65, 0x3136322d65, 0x3036322d65, 0x3935322d65, 0x3835322d65, 0x3735322d65, 0x3635322d65, 0x3535322d65, 0x3435322d65, 0x3335322d65, 0x3235322d65, 0x3135322d65, 0x3035322d65, 0x3934322d65, 0x3834322d65, 0x3734322d65, 0x3634322d65, 0x3534322d65, 0x3434322d65, 0x3334322d65, 0x3234322d65, 0x3134322d65, 0x3034322d65, 0x3933322d65, 0x3833322d65, 0x3733322d65, 0x3633322d65, 0x3533322d65, 0x3433322d65, 0x3333322d65, 0x3233322d65, 0x3133322d65, 0x3033322d65, 0x3932322d65, 0x3832322d65, 0x3732322d65, 0x3632322d65, 0x3532322d65, 0x3432322d65, 0x3332322d65, 0x3232322d65, 0x3132322d65, 0x3032322d65, 0x3931322d65, 0x3831322d65, 0x3731322d65, 0x3631322d65, 0x3531322d65, 0x3431322d65, 0x3331322d65, 0x3231322d65, 0x3131322d65, 0x3031322d65, 0x3930322d65, 0x3830322d65, 0x3730322d65, 0x3630322d65, 0x3530322d65, 0x3430322d65, 0x3330322d65, 0x3230322d65, 0x3130322d65, 0x3030322d65, 0x3939312d65, 0x3839312d65, 0x3739312d65, 0x3639312d65, 0x3539312d65, 0x3439312d65, 0x3339312d65, 0x3239312d65, 0x3139312d65, 0x3039312d65, 0x3938312d65, 0x3838312d65, 0x3738312d65, 0x3638312d65, 0x3538312d65, 0x3438312d65, 0x3338312d65, 0x3238312d65, 0x3138312d65, 0x3038312d65, 0x3937312d65, 0x3837312d65, 0x3737312d65, 0x3637312d65, 0x3537312d65, 0x3437312d65, 0x3337312d65, 0x3237312d65, 0x3137312d65, 0x3037312d65, 0x3936312d65, 0x3836312d65, 0x3736312d65, 0x3636312d65, 0x3536312d65, 0x3436312d65, 0x3336312d65, 0x3236312d65, 0x3136312d65, 0x3036312d65, 0x3935312d65, 0x3835312d65, 0x3735312d65, 0x3635312d65, 0x3535312d65, 0x3435312d65, 0x3335312d65, 0x3235312d65, 0x3135312d65, 0x3035312d65, 0x3934312d65, 0x3834312d65, 0x3734312d65, 0x3634312d65, 0x3534312d65, 0x3434312d65, 0x3334312d65, 0x3234312d65, 0x3134312d65, 0x3034312d65, 0x3933312d65, 0x3833312d65, 0x3733312d65, 0x3633312d65, 0x3533312d65, 0x3433312d65, 0x3333312d65, 0x3233312d65, 0x3133312d65, 0x3033312d65, 0x3932312d65, 0x3832312d65, 0x3732312d65, 0x3632312d65, 0x3532312d65, 0x3432312d65, 0x3332312d65, 0x3232312d65, 0x3132312d65, 0x3032312d65, 0x3931312d65, 0x3831312d65, 0x3731312d65, 0x3631312d65, 0x3531312d65, 0x3431312d65, 0x3331312d65, 0x3231312d65, 0x3131312d65, 0x3031312d65, 0x3930312d65, 0x3830312d65, 0x3730312d65, 0x3630312d65, 0x3530312d65, 0x3430312d65, 0x3330312d65, 0x3230312d65, 0x3130312d65, 0x3030312d65, 0x39392d65, 0x38392d65, 0x37392d65, 0x36392d65, 0x35392d65, 0x34392d65, 0x33392d65, 0x32392d65, 0x31392d65, 0x30392d65, 0x39382d65, 0x38382d65, 0x37382d65, 0x36382d65, 0x35382d65, 0x34382d65, 0x33382d65, 0x32382d65, 0x31382d65, 0x30382d65, 0x39372d65, 0x38372d65, 0x37372d65, 0x36372d65, 0x35372d65, 0x34372d65, 0x33372d65, 0x32372d65, 0x31372d65, 0x30372d65, 0x39362d65, 0x38362d65, 0x37362d65, 0x36362d65, 0x35362d65, 0x34362d65, 0x33362d65, 0x32362d65, 0x31362d65, 0x30362d65, 0x39352d65, 0x38352d65, 0x37352d65, 0x36352d65, 0x35352d65, 0x34352d65, 0x33352d65, 0x32352d65, 0x31352d65, 0x30352d65, 0x39342d65, 0x38342d65, 0x37342d65, 0x36342d65, 0x35342d65, 0x34342d65, 0x33342d65, 0x32342d65, 0x31342d65, 0x30342d65, 0x39332d65, 0x38332d65, 0x37332d65, 0x36332d65, 0x35332d65, 0x34332d65, 0x33332d65, 0x32332d65, 0x31332d65, 0x30332d65, 0x39322d65, 0x38322d65, 0x37322d65, 0x36322d65, 0x35322d65, 0x34322d65, 0x33322d65, 0x32322d65, 0x31322d65, 0x30322d65, 0x39312d65, 0x38312d65, 0x37312d65, 0x36312d65, 0x35312d65, 0x34312d65, 0x33312d65, 0x32312d65, 0x31312d65, 0x30312d65, 0x39302d65, 0x38302d65, 0x37302d65, 0x36302d65, 0x35302d65, 0x34302d65, 0x33302d65, 0x32302d65, 0x31302d65, 0x30302b65, 0x31302b65, 0x32302b65, 0x33302b65, 0x34302b65, 0x35302b65, 0x36302b65, 0x37302b65, 0x38302b65, 0x39302b65, 0x30312b65, 0x31312b65, 0x32312b65, 0x33312b65, 0x34312b65, 0x35312b65, 0x36312b65, 0x37312b65, 0x38312b65, 0x39312b65, 0x30322b65, 0x31322b65, 0x32322b65, 0x33322b65, 0x34322b65, 0x35322b65, 0x36322b65, 0x37322b65, 0x38322b65, 0x39322b65, 0x30332b65, 0x31332b65, 0x32332b65, 0x33332b65, 0x34332b65, 0x35332b65, 0x36332b65, 0x37332b65, 0x38332b65, 0x39332b65, 0x30342b65, 0x31342b65, 0x32342b65, 0x33342b65, 0x34342b65, 0x35342b65, 0x36342b65, 0x37342b65, 0x38342b65, 0x39342b65, 0x30352b65, 0x31352b65, 0x32352b65, 0x33352b65, 0x34352b65, 0x35352b65, 0x36352b65, 0x37352b65, 0x38352b65, 0x39352b65, 0x30362b65, 0x31362b65, 0x32362b65, 0x33362b65, 0x34362b65, 0x35362b65, 0x36362b65, 0x37362b65, 0x38362b65, 0x39362b65, 0x30372b65, 0x31372b65, 0x32372b65, 0x33372b65, 0x34372b65, 0x35372b65, 0x36372b65, 0x37372b65, 0x38372b65, 0x39372b65, 0x30382b65, 0x31382b65, 0x32382b65, 0x33382b65, 0x34382b65, 0x35382b65, 0x36382b65, 0x37382b65, 0x38382b65, 0x39382b65, 0x30392b65, 0x31392b65, 0x32392b65, 0x33392b65, 0x34392b65, 0x35392b65, 0x36392b65, 0x37392b65, 0x38392b65, 0x39392b65, 0x3030312b65, 0x3130312b65, 0x3230312b65, 0x3330312b65, 0x3430312b65, 0x3530312b65, 0x3630312b65, 0x3730312b65, 0x3830312b65, 0x3930312b65, 0x3031312b65, 0x3131312b65, 0x3231312b65, 0x3331312b65, 0x3431312b65, 0x3531312b65, 0x3631312b65, 0x3731312b65, 0x3831312b65, 0x3931312b65, 0x3032312b65, 0x3132312b65, 0x3232312b65, 0x3332312b65, 0x3432312b65, 0x3532312b65, 0x3632312b65, 0x3732312b65, 0x3832312b65, 0x3932312b65, 0x3033312b65, 0x3133312b65, 0x3233312b65, 0x3333312b65, 0x3433312b65, 0x3533312b65, 0x3633312b65, 0x3733312b65, 0x3833312b65, 0x3933312b65, 0x3034312b65, 0x3134312b65, 0x3234312b65, 0x3334312b65, 0x3434312b65, 0x3534312b65, 0x3634312b65, 0x3734312b65, 0x3834312b65, 0x3934312b65, 0x3035312b65, 0x3135312b65, 0x3235312b65, 0x3335312b65, 0x3435312b65, 0x3535312b65, 0x3635312b65, 0x3735312b65, 0x3835312b65, 0x3935312b65, 0x3036312b65, 0x3136312b65, 0x3236312b65, 0x3336312b65, 0x3436312b65, 0x3536312b65, 0x3636312b65, 0x3736312b65, 0x3836312b65, 0x3936312b65, 0x3037312b65, 0x3137312b65, 0x3237312b65, 0x3337312b65, 0x3437312b65, 0x3537312b65, 0x3637312b65, 0x3737312b65, 0x3837312b65, 0x3937312b65, 0x3038312b65, 0x3138312b65, 0x3238312b65, 0x3338312b65, 0x3438312b65, 0x3538312b65, 0x3638312b65, 0x3738312b65, 0x3838312b65, 0x3938312b65, 0x3039312b65, 0x3139312b65, 0x3239312b65, 0x3339312b65, 0x3439312b65, 0x3539312b65, 0x3639312b65, 0x3739312b65, 0x3839312b65, 0x3939312b65, 0x3030322b65, 0x3130322b65, 0x3230322b65, 0x3330322b65, 0x3430322b65, 0x3530322b65, 0x3630322b65, 0x3730322b65, 0x3830322b65, 0x3930322b65, 0x3031322b65, 0x3131322b65, 0x3231322b65, 0x3331322b65, 0x3431322b65, 0x3531322b65, 0x3631322b65, 0x3731322b65, 0x3831322b65, 0x3931322b65, 0x3032322b65, 0x3132322b65, 0x3232322b65, 0x3332322b65, 0x3432322b65, 0x3532322b65, 0x3632322b65, 0x3732322b65, 0x3832322b65, 0x3932322b65, 0x3033322b65, 0x3133322b65, 0x3233322b65, 0x3333322b65, 0x3433322b65, 0x3533322b65, 0x3633322b65, 0x3733322b65, 0x3833322b65, 0x3933322b65, 0x3034322b65, 0x3134322b65, 0x3234322b65, 0x3334322b65, 0x3434322b65, 0x3534322b65, 0x3634322b65, 0x3734322b65, 0x3834322b65, 0x3934322b65, 0x3035322b65, 0x3135322b65, 0x3235322b65, 0x3335322b65, 0x3435322b65, 0x3535322b65, 0x3635322b65, 0x3735322b65, 0x3835322b65, 0x3935322b65, 0x3036322b65, 0x3136322b65, 0x3236322b65, 0x3336322b65, 0x3436322b65, 0x3536322b65, 0x3636322b65, 0x3736322b65, 0x3836322b65, 0x3936322b65, 0x3037322b65, 0x3137322b65, 0x3237322b65, 0x3337322b65, 0x3437322b65, 0x3537322b65, 0x3637322b65, 0x3737322b65, 0x3837322b65, 0x3937322b65, 0x3038322b65, 0x3138322b65, 0x3238322b65, 0x3338322b65, 0x3438322b65, 0x3538322b65, 0x3638322b65, 0x3738322b65, 0x3838322b65, 0x3938322b65, 0x3039322b65, 0x3139322b65, 0x3239322b65, 0x3339322b65, 0x3439322b65, 0x3539322b65, 0x3639322b65, 0x3739322b65, 0x3839322b65, 0x3939322b65, 0x3030332b65, 0x3130332b65, 0x3230332b65, 0x3330332b65, 0x3430332b65, 0x3530332b65, 0x3630332b65, 0x3730332b65, 0x3830332b65};





// ================================ d2s.cpp =================================

using u64 = unsigned long long;
using i64 = long long;
using ull = unsigned long long;
using ll = long long;

struct u64x2
{
    u64 hi;
    u64 lo;
};

static inline unsigned long long dec_length(const unsigned long long v)
{
    // return value [1,16]
    return (v >= 0) +              // 0
           (v >= 10) +             // 1
           (v >= 100) +            // 2
           (v >= 1000) +           // 3
           (v >= 10000) +          // 4
           (v >= 100000) +         // 5
           (v >= 1000000) +        // 6
           (v >= 10000000) +       // 7
           (v >= 100000000) +      // 8
           (v >= 1000000000) +     // 9
           (v >= 10000000000) +    // 10
           (v >= 100000000000) +   // 11
           (v >= 1000000000000) +  // 12
           (v >= 10000000000000) + // 13
           (v >= 100000000000000); // 14
                                   //+ (v >= 1000000000000000);  // 15
}

static inline unsigned long long RoundToOdd(u64x2 g, unsigned long long cp)
{
    unsigned long long x1 = 0;
    unsigned long long x0 = _mulx_u64(g.lo, cp, &x1);
    unsigned long long y1 = 0;
    unsigned long long y0 = _mulx_u64(g.hi, cp, &y1);
    static_cast<void>(x0);

    // y0 += x1;
    // y1 += y0 < x1;
    _addcarry_u64(_addcarry_u64(0, y0, x1, &y0), y1, 0, &y1);

    return y1 | (y0 > 1);
}
static inline __m512i merge_h32(__m512i a, __m512i b)
{
    const __m512i idx = _mm512_set_epi32(31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11, 9, 7, 5, 3, 1);
    __m512i res = _mm512_permutex2var_epi32(b, idx, a); // 1:b,0:a;
    return res;
}

static inline __m512i merge_l32(__m512i a, __m512i b)
{
    const __m512i idx = _mm512_set_epi32(30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0);
    __m512i res = _mm512_permutex2var_epi32(b, idx, a); // 1:b,0:a;
    return res;
}

static inline int count_tz(int high9, int low8)
{
    // count tail zero number ; num = high9*10^8+low8
    // num < 1e17 ;
    // such as if num = 1 1230 0000 0000 0000 ; return 13
    // but if num =  1 0000 0000 0000 0000 ; return 17
    __m512i h9 = _mm512_set1_epi64(high9);
    __m512i l8 = _mm512_set1_epi64(low8);
    __m512i all = merge_l32(h9, l8);
    const int e57_e8 = 1441151881; // ceil(2**57/1e8)
    const int e54_e7 = 1801439851;
    const int e50_e6 = 1125899907;
    const int e47_e5 = 1407374884;
    const int e44_e4 = 1759218605;
    const int e40_e3 = 1099511628;
    const int e37_e2 = 1374389535;
    const int e34_e1 = 1717986919;
    const __m512i n8 = _mm512_set_epi64(e57_e8, e54_e7, e50_e6, e47_e5, e44_e4, e40_e3, e37_e2, e34_e1);
    // const __m512i r8 = _mm512_set_epi64(57,54,50,47,44,40,37,34);
    const __m512i r16 = _mm512_set_epi32(25, 22, 18, 15, 12, 8, 5, 2, 25, 22, 18, 15, 12, 8, 5, 2);
    // const __m512i m8 = _mm512_set_epi64(1e8,1e7,1e6,1e5,1e4,1e3,1e2,1e1);
    const __m512i m16 = _mm512_set_epi32(1e8, 1e7, 1e6, 1e5, 1e4, 1e3, 1e2, 1e1, 1e8, 1e7, 1e6, 1e5, 1e4, 1e3, 1e2, 1e1);
    __m512i h9_n8 = _mm512_mullo_epi64(h9, n8);
    __m512i l8_n8 = _mm512_mullo_epi64(l8, n8);
    __m512i h9_l8 = merge_h32(h9_n8, l8_n8);
    __m512i res17 = _mm512_mullo_epi32(_mm512_srlv_epi32(h9_l8, r16), m16);
    __mmask16 mask17 = _mm512_cmpneq_epi32_mask(res17, all);

    // __m512i res9 =_mm512_mullo_epi64(_mm512_srlv_epi64(_mm512_mullo_epi64(h9,n8) , r8) , m8);
    // __m512i res8 =_mm512_mullo_epi64(_mm512_srlv_epi64(_mm512_mullo_epi64(l8,n8) , r8) , m8);
    // __mmask8 mask9 = _mm512_cmpneq_epi64_mask(res9,h9);
    // __mmask8 mask8 = _mm512_cmpneq_epi64_mask(res8,l8);

    // int ctz9 = _tzcnt_u64(mask9|0x100);// max = 8;
    // int ctz8 = _tzcnt_u64(mask8|0x100);// max = 8;
    int ctz17 = _tzcnt_u64(mask17 | 0x20000);
    // printf("h9=%d,l8=%d,mask9=%x,ctz9=%d,mask8 = %x,ctz8=%d,mask17=%x,ctz17=%d\n",high9,low8,mask9,ctz9,mask8,ctz8,mask17,ctz17);
    return ctz17; //[0,15] and 17
}
#if 0
u64 generalized_granlund_montgomery_branchless(u64 n)
{
    u64 s = 0;

    auto r = u64(n * u64(28999941890838049));
    auto b = r < u64(184467440969);
    s = s * 2 + b;
    n = b ? u64(r >> 8) : n;

    r = u64(n * u64(182622766329724561));
    b = r < u64(1844674407370971);
    s = s * 2 + b;
    n = b ? u64(r >> 4) : n;

    r = u64(n * u64(14941862699704736809));
    b = r < u64(184467440737095517);
    s = s * 2 + b;
    n = b ? u64(r >> 2) : n;

    r = u64(n * u64(5534023222112865485));
    b = r < u64(1844674407370955163);
    s = s * 2 + b;
    n = b ? u64(r >> 1) : n;

    return s;
}
#endif



#ifdef HAVE_AVX512

static inline int d2s_avx512(double v, char *buffer)
{
    //============ README start===========

    // this code base on Schubafach algorithm
    // avx512 implementation
    // I have limited time, so the performance may not be optimal.

    // performance test result
    // when input double is random
    // CPU : AMD R7 7840H ; use icpx 2025.0.4 -O3 compile this func
    // performance: this func cost 12ns per double , 1.83x faster than dragonbox , 2.08x faster than schubafach
    // dragonbox  cost 22ns per double
    // schubafach cost 25ns per double
    // ryu        cost 27ns per double

    // return write buffer length
    // recommend set buffer length is 32 byte;

    // this func may has any bug , if you find bug please report
    // if you can improve this code performance,you can also report

    // input double v range and print result ;
    // range       : double v ->   print result
    // 1. v<1e-7   : 1.23e-8  ->   1.23e-08  ,  1.23e-11 -> 1.23e-11  , 1.23e-103  -> 1.23e-103
    // 2. [1e-7,1) : 1.23e-3  ->   0.00123   ,  1.23e-1  -> 0.123     , 0.0123     -> 0.0123
    // 3. [1,1e9)  : 1.23e2   ->   123       ,  1.23e5   -> 12300       123.45     -> 123.45
    // 4. >=1e9    : 1.23e10  ->   1.23e+10  ,  1.23e9   -> 1.23e+09  , 1.23e103   -> 1.23e+103

    // special value : 0 -> 0 , nan -> nan , inf -> inf

    // The code may not be the final version
    //============ README end===========

    // caller-saved registers
    // rax, rcx, rdx , r8-r11 , xmm0-xmm5
    // 7 + 6 = 13 registers
    // how to reduce register use , how to use xmm registers to improve performance

    const int kMin = -292;
    const int kMax = 324;
    static const u64x2 g[kMax - kMin + 1] = {
        {0xFF77B1FCBEBCDC4F, 0x25E8E89C13BB0F7B}, // -292
        {0x9FAACF3DF73609B1, 0x77B191618C54E9AD}, // -291
        {0xC795830D75038C1D, 0xD59DF5B9EF6A2418}, // -290
        {0xF97AE3D0D2446F25, 0x4B0573286B44AD1E}, // -289
        {0x9BECCE62836AC577, 0x4EE367F9430AEC33}, // -288
        {0xC2E801FB244576D5, 0x229C41F793CDA740}, // -287
        {0xF3A20279ED56D48A, 0x6B43527578C11110}, // -286
        {0x9845418C345644D6, 0x830A13896B78AAAA}, // -285
        {0xBE5691EF416BD60C, 0x23CC986BC656D554}, // -284
        {0xEDEC366B11C6CB8F, 0x2CBFBE86B7EC8AA9}, // -283
        {0x94B3A202EB1C3F39, 0x7BF7D71432F3D6AA}, // -282
        {0xB9E08A83A5E34F07, 0xDAF5CCD93FB0CC54}, // -281
        {0xE858AD248F5C22C9, 0xD1B3400F8F9CFF69}, // -280
        {0x91376C36D99995BE, 0x23100809B9C21FA2}, // -279
        {0xB58547448FFFFB2D, 0xABD40A0C2832A78B}, // -278
        {0xE2E69915B3FFF9F9, 0x16C90C8F323F516D}, // -277
        {0x8DD01FAD907FFC3B, 0xAE3DA7D97F6792E4}, // -276
        {0xB1442798F49FFB4A, 0x99CD11CFDF41779D}, // -275
        {0xDD95317F31C7FA1D, 0x40405643D711D584}, // -274
        {0x8A7D3EEF7F1CFC52, 0x482835EA666B2573}, // -273
        {0xAD1C8EAB5EE43B66, 0xDA3243650005EED0}, // -272
        {0xD863B256369D4A40, 0x90BED43E40076A83}, // -271
        {0x873E4F75E2224E68, 0x5A7744A6E804A292}, // -270
        {0xA90DE3535AAAE202, 0x711515D0A205CB37}, // -269
        {0xD3515C2831559A83, 0x0D5A5B44CA873E04}, // -268
        {0x8412D9991ED58091, 0xE858790AFE9486C3}, // -267
        {0xA5178FFF668AE0B6, 0x626E974DBE39A873}, // -266
        {0xCE5D73FF402D98E3, 0xFB0A3D212DC81290}, // -265
        {0x80FA687F881C7F8E, 0x7CE66634BC9D0B9A}, // -264
        {0xA139029F6A239F72, 0x1C1FFFC1EBC44E81}, // -263
        {0xC987434744AC874E, 0xA327FFB266B56221}, // -262
        {0xFBE9141915D7A922, 0x4BF1FF9F0062BAA9}, // -261
        {0x9D71AC8FADA6C9B5, 0x6F773FC3603DB4AA}, // -260
        {0xC4CE17B399107C22, 0xCB550FB4384D21D4}, // -259
        {0xF6019DA07F549B2B, 0x7E2A53A146606A49}, // -258
        {0x99C102844F94E0FB, 0x2EDA7444CBFC426E}, // -257
        {0xC0314325637A1939, 0xFA911155FEFB5309}, // -256
        {0xF03D93EEBC589F88, 0x793555AB7EBA27CB}, // -255
        {0x96267C7535B763B5, 0x4BC1558B2F3458DF}, // -254
        {0xBBB01B9283253CA2, 0x9EB1AAEDFB016F17}, // -253
        {0xEA9C227723EE8BCB, 0x465E15A979C1CADD}, // -252
        {0x92A1958A7675175F, 0x0BFACD89EC191ECA}, // -251
        {0xB749FAED14125D36, 0xCEF980EC671F667C}, // -250
        {0xE51C79A85916F484, 0x82B7E12780E7401B}, // -249
        {0x8F31CC0937AE58D2, 0xD1B2ECB8B0908811}, // -248
        {0xB2FE3F0B8599EF07, 0x861FA7E6DCB4AA16}, // -247
        {0xDFBDCECE67006AC9, 0x67A791E093E1D49B}, // -246
        {0x8BD6A141006042BD, 0xE0C8BB2C5C6D24E1}, // -245
        {0xAECC49914078536D, 0x58FAE9F773886E19}, // -244
        {0xDA7F5BF590966848, 0xAF39A475506A899F}, // -243
        {0x888F99797A5E012D, 0x6D8406C952429604}, // -242
        {0xAAB37FD7D8F58178, 0xC8E5087BA6D33B84}, // -241
        {0xD5605FCDCF32E1D6, 0xFB1E4A9A90880A65}, // -240
        {0x855C3BE0A17FCD26, 0x5CF2EEA09A550680}, // -239
        {0xA6B34AD8C9DFC06F, 0xF42FAA48C0EA481F}, // -238
        {0xD0601D8EFC57B08B, 0xF13B94DAF124DA27}, // -237
        {0x823C12795DB6CE57, 0x76C53D08D6B70859}, // -236
        {0xA2CB1717B52481ED, 0x54768C4B0C64CA6F}, // -235
        {0xCB7DDCDDA26DA268, 0xA9942F5DCF7DFD0A}, // -234
        {0xFE5D54150B090B02, 0xD3F93B35435D7C4D}, // -233
        {0x9EFA548D26E5A6E1, 0xC47BC5014A1A6DB0}, // -232
        {0xC6B8E9B0709F109A, 0x359AB6419CA1091C}, // -231
        {0xF867241C8CC6D4C0, 0xC30163D203C94B63}, // -230
        {0x9B407691D7FC44F8, 0x79E0DE63425DCF1E}, // -229
        {0xC21094364DFB5636, 0x985915FC12F542E5}, // -228
        {0xF294B943E17A2BC4, 0x3E6F5B7B17B2939E}, // -227
        {0x979CF3CA6CEC5B5A, 0xA705992CEECF9C43}, // -226
        {0xBD8430BD08277231, 0x50C6FF782A838354}, // -225
        {0xECE53CEC4A314EBD, 0xA4F8BF5635246429}, // -224
        {0x940F4613AE5ED136, 0x871B7795E136BE9A}, // -223
        {0xB913179899F68584, 0x28E2557B59846E40}, // -222
        {0xE757DD7EC07426E5, 0x331AEADA2FE589D0}, // -221
        {0x9096EA6F3848984F, 0x3FF0D2C85DEF7622}, // -220
        {0xB4BCA50B065ABE63, 0x0FED077A756B53AA}, // -219
        {0xE1EBCE4DC7F16DFB, 0xD3E8495912C62895}, // -218
        {0x8D3360F09CF6E4BD, 0x64712DD7ABBBD95D}, // -217
        {0xB080392CC4349DEC, 0xBD8D794D96AACFB4}, // -216
        {0xDCA04777F541C567, 0xECF0D7A0FC5583A1}, // -215
        {0x89E42CAAF9491B60, 0xF41686C49DB57245}, // -214
        {0xAC5D37D5B79B6239, 0x311C2875C522CED6}, // -213
        {0xD77485CB25823AC7, 0x7D633293366B828C}, // -212
        {0x86A8D39EF77164BC, 0xAE5DFF9C02033198}, // -211
        {0xA8530886B54DBDEB, 0xD9F57F830283FDFD}, // -210
        {0xD267CAA862A12D66, 0xD072DF63C324FD7C}, // -209
        {0x8380DEA93DA4BC60, 0x4247CB9E59F71E6E}, // -208
        {0xA46116538D0DEB78, 0x52D9BE85F074E609}, // -207
        {0xCD795BE870516656, 0x67902E276C921F8C}, // -206
        {0x806BD9714632DFF6, 0x00BA1CD8A3DB53B7}, // -205
        {0xA086CFCD97BF97F3, 0x80E8A40ECCD228A5}, // -204
        {0xC8A883C0FDAF7DF0, 0x6122CD128006B2CE}, // -203
        {0xFAD2A4B13D1B5D6C, 0x796B805720085F82}, // -202
        {0x9CC3A6EEC6311A63, 0xCBE3303674053BB1}, // -201
        {0xC3F490AA77BD60FC, 0xBEDBFC4411068A9D}, // -200
        {0xF4F1B4D515ACB93B, 0xEE92FB5515482D45}, // -199
        {0x991711052D8BF3C5, 0x751BDD152D4D1C4B}, // -198
        {0xBF5CD54678EEF0B6, 0xD262D45A78A0635E}, // -197
        {0xEF340A98172AACE4, 0x86FB897116C87C35}, // -196
        {0x9580869F0E7AAC0E, 0xD45D35E6AE3D4DA1}, // -195
        {0xBAE0A846D2195712, 0x8974836059CCA10A}, // -194
        {0xE998D258869FACD7, 0x2BD1A438703FC94C}, // -193
        {0x91FF83775423CC06, 0x7B6306A34627DDD0}, // -192
        {0xB67F6455292CBF08, 0x1A3BC84C17B1D543}, // -191
        {0xE41F3D6A7377EECA, 0x20CABA5F1D9E4A94}, // -190
        {0x8E938662882AF53E, 0x547EB47B7282EE9D}, // -189
        {0xB23867FB2A35B28D, 0xE99E619A4F23AA44}, // -188
        {0xDEC681F9F4C31F31, 0x6405FA00E2EC94D5}, // -187
        {0x8B3C113C38F9F37E, 0xDE83BC408DD3DD05}, // -186
        {0xAE0B158B4738705E, 0x9624AB50B148D446}, // -185
        {0xD98DDAEE19068C76, 0x3BADD624DD9B0958}, // -184
        {0x87F8A8D4CFA417C9, 0xE54CA5D70A80E5D7}, // -183
        {0xA9F6D30A038D1DBC, 0x5E9FCF4CCD211F4D}, // -182
        {0xD47487CC8470652B, 0x7647C32000696720}, // -181
        {0x84C8D4DFD2C63F3B, 0x29ECD9F40041E074}, // -180
        {0xA5FB0A17C777CF09, 0xF468107100525891}, // -179
        {0xCF79CC9DB955C2CC, 0x7182148D4066EEB5}, // -178
        {0x81AC1FE293D599BF, 0xC6F14CD848405531}, // -177
        {0xA21727DB38CB002F, 0xB8ADA00E5A506A7D}, // -176
        {0xCA9CF1D206FDC03B, 0xA6D90811F0E4851D}, // -175
        {0xFD442E4688BD304A, 0x908F4A166D1DA664}, // -174
        {0x9E4A9CEC15763E2E, 0x9A598E4E043287FF}, // -173
        {0xC5DD44271AD3CDBA, 0x40EFF1E1853F29FE}, // -172
        {0xF7549530E188C128, 0xD12BEE59E68EF47D}, // -171
        {0x9A94DD3E8CF578B9, 0x82BB74F8301958CF}, // -170
        {0xC13A148E3032D6E7, 0xE36A52363C1FAF02}, // -169
        {0xF18899B1BC3F8CA1, 0xDC44E6C3CB279AC2}, // -168
        {0x96F5600F15A7B7E5, 0x29AB103A5EF8C0BA}, // -167
        {0xBCB2B812DB11A5DE, 0x7415D448F6B6F0E8}, // -166
        {0xEBDF661791D60F56, 0x111B495B3464AD22}, // -165
        {0x936B9FCEBB25C995, 0xCAB10DD900BEEC35}, // -164
        {0xB84687C269EF3BFB, 0x3D5D514F40EEA743}, // -163
        {0xE65829B3046B0AFA, 0x0CB4A5A3112A5113}, // -162
        {0x8FF71A0FE2C2E6DC, 0x47F0E785EABA72AC}, // -161
        {0xB3F4E093DB73A093, 0x59ED216765690F57}, // -160
        {0xE0F218B8D25088B8, 0x306869C13EC3532D}, // -159
        {0x8C974F7383725573, 0x1E414218C73A13FC}, // -158
        {0xAFBD2350644EEACF, 0xE5D1929EF90898FB}, // -157
        {0xDBAC6C247D62A583, 0xDF45F746B74ABF3A}, // -156
        {0x894BC396CE5DA772, 0x6B8BBA8C328EB784}, // -155
        {0xAB9EB47C81F5114F, 0x066EA92F3F326565}, // -154
        {0xD686619BA27255A2, 0xC80A537B0EFEFEBE}, // -153
        {0x8613FD0145877585, 0xBD06742CE95F5F37}, // -152
        {0xA798FC4196E952E7, 0x2C48113823B73705}, // -151
        {0xD17F3B51FCA3A7A0, 0xF75A15862CA504C6}, // -150
        {0x82EF85133DE648C4, 0x9A984D73DBE722FC}, // -149
        {0xA3AB66580D5FDAF5, 0xC13E60D0D2E0EBBB}, // -148
        {0xCC963FEE10B7D1B3, 0x318DF905079926A9}, // -147
        {0xFFBBCFE994E5C61F, 0xFDF17746497F7053}, // -146
        {0x9FD561F1FD0F9BD3, 0xFEB6EA8BEDEFA634}, // -145
        {0xC7CABA6E7C5382C8, 0xFE64A52EE96B8FC1}, // -144
        {0xF9BD690A1B68637B, 0x3DFDCE7AA3C673B1}, // -143
        {0x9C1661A651213E2D, 0x06BEA10CA65C084F}, // -142
        {0xC31BFA0FE5698DB8, 0x486E494FCFF30A63}, // -141
        {0xF3E2F893DEC3F126, 0x5A89DBA3C3EFCCFB}, // -140
        {0x986DDB5C6B3A76B7, 0xF89629465A75E01D}, // -139
        {0xBE89523386091465, 0xF6BBB397F1135824}, // -138
        {0xEE2BA6C0678B597F, 0x746AA07DED582E2D}, // -137
        {0x94DB483840B717EF, 0xA8C2A44EB4571CDD}, // -136
        {0xBA121A4650E4DDEB, 0x92F34D62616CE414}, // -135
        {0xE896A0D7E51E1566, 0x77B020BAF9C81D18}, // -134
        {0x915E2486EF32CD60, 0x0ACE1474DC1D122F}, // -133
        {0xB5B5ADA8AAFF80B8, 0x0D819992132456BB}, // -132
        {0xE3231912D5BF60E6, 0x10E1FFF697ED6C6A}, // -131
        {0x8DF5EFABC5979C8F, 0xCA8D3FFA1EF463C2}, // -130
        {0xB1736B96B6FD83B3, 0xBD308FF8A6B17CB3}, // -129
        {0xDDD0467C64BCE4A0, 0xAC7CB3F6D05DDBDF}, // -128
        {0x8AA22C0DBEF60EE4, 0x6BCDF07A423AA96C}, // -127
        {0xAD4AB7112EB3929D, 0x86C16C98D2C953C7}, // -126
        {0xD89D64D57A607744, 0xE871C7BF077BA8B8}, // -125
        {0x87625F056C7C4A8B, 0x11471CD764AD4973}, // -124
        {0xA93AF6C6C79B5D2D, 0xD598E40D3DD89BD0}, // -123
        {0xD389B47879823479, 0x4AFF1D108D4EC2C4}, // -122
        {0x843610CB4BF160CB, 0xCEDF722A585139BB}, // -121
        {0xA54394FE1EEDB8FE, 0xC2974EB4EE658829}, // -120
        {0xCE947A3DA6A9273E, 0x733D226229FEEA33}, // -119
        {0x811CCC668829B887, 0x0806357D5A3F5260}, // -118
        {0xA163FF802A3426A8, 0xCA07C2DCB0CF26F8}, // -117
        {0xC9BCFF6034C13052, 0xFC89B393DD02F0B6}, // -116
        {0xFC2C3F3841F17C67, 0xBBAC2078D443ACE3}, // -115
        {0x9D9BA7832936EDC0, 0xD54B944B84AA4C0E}, // -114
        {0xC5029163F384A931, 0x0A9E795E65D4DF12}, // -113
        {0xF64335BCF065D37D, 0x4D4617B5FF4A16D6}, // -112
        {0x99EA0196163FA42E, 0x504BCED1BF8E4E46}, // -111
        {0xC06481FB9BCF8D39, 0xE45EC2862F71E1D7}, // -110
        {0xF07DA27A82C37088, 0x5D767327BB4E5A4D}, // -109
        {0x964E858C91BA2655, 0x3A6A07F8D510F870}, // -108
        {0xBBE226EFB628AFEA, 0x890489F70A55368C}, // -107
        {0xEADAB0ABA3B2DBE5, 0x2B45AC74CCEA842F}, // -106
        {0x92C8AE6B464FC96F, 0x3B0B8BC90012929E}, // -105
        {0xB77ADA0617E3BBCB, 0x09CE6EBB40173745}, // -104
        {0xE55990879DDCAABD, 0xCC420A6A101D0516}, // -103
        {0x8F57FA54C2A9EAB6, 0x9FA946824A12232E}, // -102
        {0xB32DF8E9F3546564, 0x47939822DC96ABFA}, // -101
        {0xDFF9772470297EBD, 0x59787E2B93BC56F8}, // -100
        {0x8BFBEA76C619EF36, 0x57EB4EDB3C55B65B}, //  -99
        {0xAEFAE51477A06B03, 0xEDE622920B6B23F2}, //  -98
        {0xDAB99E59958885C4, 0xE95FAB368E45ECEE}, //  -97
        {0x88B402F7FD75539B, 0x11DBCB0218EBB415}, //  -96
        {0xAAE103B5FCD2A881, 0xD652BDC29F26A11A}, //  -95
        {0xD59944A37C0752A2, 0x4BE76D3346F04960}, //  -94
        {0x857FCAE62D8493A5, 0x6F70A4400C562DDC}, //  -93
        {0xA6DFBD9FB8E5B88E, 0xCB4CCD500F6BB953}, //  -92
        {0xD097AD07A71F26B2, 0x7E2000A41346A7A8}, //  -91
        {0x825ECC24C873782F, 0x8ED400668C0C28C9}, //  -90
        {0xA2F67F2DFA90563B, 0x728900802F0F32FB}, //  -89
        {0xCBB41EF979346BCA, 0x4F2B40A03AD2FFBA}, //  -88
        {0xFEA126B7D78186BC, 0xE2F610C84987BFA9}, //  -87
        {0x9F24B832E6B0F436, 0x0DD9CA7D2DF4D7CA}, //  -86
        {0xC6EDE63FA05D3143, 0x91503D1C79720DBC}, //  -85
        {0xF8A95FCF88747D94, 0x75A44C6397CE912B}, //  -84
        {0x9B69DBE1B548CE7C, 0xC986AFBE3EE11ABB}, //  -83
        {0xC24452DA229B021B, 0xFBE85BADCE996169}, //  -82
        {0xF2D56790AB41C2A2, 0xFAE27299423FB9C4}, //  -81
        {0x97C560BA6B0919A5, 0xDCCD879FC967D41B}, //  -80
        {0xBDB6B8E905CB600F, 0x5400E987BBC1C921}, //  -79
        {0xED246723473E3813, 0x290123E9AAB23B69}, //  -78
        {0x9436C0760C86E30B, 0xF9A0B6720AAF6522}, //  -77
        {0xB94470938FA89BCE, 0xF808E40E8D5B3E6A}, //  -76
        {0xE7958CB87392C2C2, 0xB60B1D1230B20E05}, //  -75
        {0x90BD77F3483BB9B9, 0xB1C6F22B5E6F48C3}, //  -74
        {0xB4ECD5F01A4AA828, 0x1E38AEB6360B1AF4}, //  -73
        {0xE2280B6C20DD5232, 0x25C6DA63C38DE1B1}, //  -72
        {0x8D590723948A535F, 0x579C487E5A38AD0F}, //  -71
        {0xB0AF48EC79ACE837, 0x2D835A9DF0C6D852}, //  -70
        {0xDCDB1B2798182244, 0xF8E431456CF88E66}, //  -69
        {0x8A08F0F8BF0F156B, 0x1B8E9ECB641B5900}, //  -68
        {0xAC8B2D36EED2DAC5, 0xE272467E3D222F40}, //  -67
        {0xD7ADF884AA879177, 0x5B0ED81DCC6ABB10}, //  -66
        {0x86CCBB52EA94BAEA, 0x98E947129FC2B4EA}, //  -65
        {0xA87FEA27A539E9A5, 0x3F2398D747B36225}, //  -64
        {0xD29FE4B18E88640E, 0x8EEC7F0D19A03AAE}, //  -63
        {0x83A3EEEEF9153E89, 0x1953CF68300424AD}, //  -62
        {0xA48CEAAAB75A8E2B, 0x5FA8C3423C052DD8}, //  -61
        {0xCDB02555653131B6, 0x3792F412CB06794E}, //  -60
        {0x808E17555F3EBF11, 0xE2BBD88BBEE40BD1}, //  -59
        {0xA0B19D2AB70E6ED6, 0x5B6ACEAEAE9D0EC5}, //  -58
        {0xC8DE047564D20A8B, 0xF245825A5A445276}, //  -57
        {0xFB158592BE068D2E, 0xEED6E2F0F0D56713}, //  -56
        {0x9CED737BB6C4183D, 0x55464DD69685606C}, //  -55
        {0xC428D05AA4751E4C, 0xAA97E14C3C26B887}, //  -54
        {0xF53304714D9265DF, 0xD53DD99F4B3066A9}, //  -53
        {0x993FE2C6D07B7FAB, 0xE546A8038EFE402A}, //  -52
        {0xBF8FDB78849A5F96, 0xDE98520472BDD034}, //  -51
        {0xEF73D256A5C0F77C, 0x963E66858F6D4441}, //  -50
        {0x95A8637627989AAD, 0xDDE7001379A44AA9}, //  -49
        {0xBB127C53B17EC159, 0x5560C018580D5D53}, //  -48
        {0xE9D71B689DDE71AF, 0xAAB8F01E6E10B4A7}, //  -47
        {0x9226712162AB070D, 0xCAB3961304CA70E9}, //  -46
        {0xB6B00D69BB55C8D1, 0x3D607B97C5FD0D23}, //  -45
        {0xE45C10C42A2B3B05, 0x8CB89A7DB77C506B}, //  -44
        {0x8EB98A7A9A5B04E3, 0x77F3608E92ADB243}, //  -43
        {0xB267ED1940F1C61C, 0x55F038B237591ED4}, //  -42
        {0xDF01E85F912E37A3, 0x6B6C46DEC52F6689}, //  -41
        {0x8B61313BBABCE2C6, 0x2323AC4B3B3DA016}, //  -40
        {0xAE397D8AA96C1B77, 0xABEC975E0A0D081B}, //  -39
        {0xD9C7DCED53C72255, 0x96E7BD358C904A22}, //  -38
        {0x881CEA14545C7575, 0x7E50D64177DA2E55}, //  -37
        {0xAA242499697392D2, 0xDDE50BD1D5D0B9EA}, //  -36
        {0xD4AD2DBFC3D07787, 0x955E4EC64B44E865}, //  -35
        {0x84EC3C97DA624AB4, 0xBD5AF13BEF0B113F}, //  -34
        {0xA6274BBDD0FADD61, 0xECB1AD8AEACDD58F}, //  -33
        {0xCFB11EAD453994BA, 0x67DE18EDA5814AF3}, //  -32
        {0x81CEB32C4B43FCF4, 0x80EACF948770CED8}, //  -31
        {0xA2425FF75E14FC31, 0xA1258379A94D028E}, //  -30
        {0xCAD2F7F5359A3B3E, 0x096EE45813A04331}, //  -29
        {0xFD87B5F28300CA0D, 0x8BCA9D6E188853FD}, //  -28
        {0x9E74D1B791E07E48, 0x775EA264CF55347E}, //  -27
        {0xC612062576589DDA, 0x95364AFE032A819E}, //  -26
        {0xF79687AED3EEC551, 0x3A83DDBD83F52205}, //  -25
        {0x9ABE14CD44753B52, 0xC4926A9672793543}, //  -24
        {0xC16D9A0095928A27, 0x75B7053C0F178294}, //  -23
        {0xF1C90080BAF72CB1, 0x5324C68B12DD6339}, //  -22
        {0x971DA05074DA7BEE, 0xD3F6FC16EBCA5E04}, //  -21
        {0xBCE5086492111AEA, 0x88F4BB1CA6BCF585}, //  -20
        {0xEC1E4A7DB69561A5, 0x2B31E9E3D06C32E6}, //  -19
        {0x9392EE8E921D5D07, 0x3AFF322E62439FD0}, //  -18
        {0xB877AA3236A4B449, 0x09BEFEB9FAD487C3}, //  -17
        {0xE69594BEC44DE15B, 0x4C2EBE687989A9B4}, //  -16
        {0x901D7CF73AB0ACD9, 0x0F9D37014BF60A11}, //  -15
        {0xB424DC35095CD80F, 0x538484C19EF38C95}, //  -14
        {0xE12E13424BB40E13, 0x2865A5F206B06FBA}, //  -13
        {0x8CBCCC096F5088CB, 0xF93F87B7442E45D4}, //  -12
        {0xAFEBFF0BCB24AAFE, 0xF78F69A51539D749}, //  -11
        {0xDBE6FECEBDEDD5BE, 0xB573440E5A884D1C}, //  -10
        {0x89705F4136B4A597, 0x31680A88F8953031}, //   -9
        {0xABCC77118461CEFC, 0xFDC20D2B36BA7C3E}, //   -8
        {0xD6BF94D5E57A42BC, 0x3D32907604691B4D}, //   -7
        {0x8637BD05AF6C69B5, 0xA63F9A49C2C1B110}, //   -6
        {0xA7C5AC471B478423, 0x0FCF80DC33721D54}, //   -5
        {0xD1B71758E219652B, 0xD3C36113404EA4A9}, //   -4
        {0x83126E978D4FDF3B, 0x645A1CAC083126EA}, //   -3
        {0xA3D70A3D70A3D70A, 0x3D70A3D70A3D70A4}, //   -2
        {0xCCCCCCCCCCCCCCCC, 0xCCCCCCCCCCCCCCCD}, //   -1
        {0x8000000000000000, 0x0000000000000000}, //    0
        {0xA000000000000000, 0x0000000000000000}, //    1
        {0xC800000000000000, 0x0000000000000000}, //    2
        {0xFA00000000000000, 0x0000000000000000}, //    3
        {0x9C40000000000000, 0x0000000000000000}, //    4
        {0xC350000000000000, 0x0000000000000000}, //    5
        {0xF424000000000000, 0x0000000000000000}, //    6
        {0x9896800000000000, 0x0000000000000000}, //    7
        {0xBEBC200000000000, 0x0000000000000000}, //    8
        {0xEE6B280000000000, 0x0000000000000000}, //    9
        {0x9502F90000000000, 0x0000000000000000}, //   10
        {0xBA43B74000000000, 0x0000000000000000}, //   11
        {0xE8D4A51000000000, 0x0000000000000000}, //   12
        {0x9184E72A00000000, 0x0000000000000000}, //   13
        {0xB5E620F480000000, 0x0000000000000000}, //   14
        {0xE35FA931A0000000, 0x0000000000000000}, //   15
        {0x8E1BC9BF04000000, 0x0000000000000000}, //   16
        {0xB1A2BC2EC5000000, 0x0000000000000000}, //   17
        {0xDE0B6B3A76400000, 0x0000000000000000}, //   18
        {0x8AC7230489E80000, 0x0000000000000000}, //   19
        {0xAD78EBC5AC620000, 0x0000000000000000}, //   20
        {0xD8D726B7177A8000, 0x0000000000000000}, //   21
        {0x878678326EAC9000, 0x0000000000000000}, //   22
        {0xA968163F0A57B400, 0x0000000000000000}, //   23
        {0xD3C21BCECCEDA100, 0x0000000000000000}, //   24
        {0x84595161401484A0, 0x0000000000000000}, //   25
        {0xA56FA5B99019A5C8, 0x0000000000000000}, //   26
        {0xCECB8F27F4200F3A, 0x0000000000000000}, //   27
        {0x813F3978F8940984, 0x4000000000000000}, //   28
        {0xA18F07D736B90BE5, 0x5000000000000000}, //   29
        {0xC9F2C9CD04674EDE, 0xA400000000000000}, //   30
        {0xFC6F7C4045812296, 0x4D00000000000000}, //   31
        {0x9DC5ADA82B70B59D, 0xF020000000000000}, //   32
        {0xC5371912364CE305, 0x6C28000000000000}, //   33
        {0xF684DF56C3E01BC6, 0xC732000000000000}, //   34
        {0x9A130B963A6C115C, 0x3C7F400000000000}, //   35
        {0xC097CE7BC90715B3, 0x4B9F100000000000}, //   36
        {0xF0BDC21ABB48DB20, 0x1E86D40000000000}, //   37
        {0x96769950B50D88F4, 0x1314448000000000}, //   38
        {0xBC143FA4E250EB31, 0x17D955A000000000}, //   39
        {0xEB194F8E1AE525FD, 0x5DCFAB0800000000}, //   40
        {0x92EFD1B8D0CF37BE, 0x5AA1CAE500000000}, //   41
        {0xB7ABC627050305AD, 0xF14A3D9E40000000}, //   42
        {0xE596B7B0C643C719, 0x6D9CCD05D0000000}, //   43
        {0x8F7E32CE7BEA5C6F, 0xE4820023A2000000}, //   44
        {0xB35DBF821AE4F38B, 0xDDA2802C8A800000}, //   45
        {0xE0352F62A19E306E, 0xD50B2037AD200000}, //   46
        {0x8C213D9DA502DE45, 0x4526F422CC340000}, //   47
        {0xAF298D050E4395D6, 0x9670B12B7F410000}, //   48
        {0xDAF3F04651D47B4C, 0x3C0CDD765F114000}, //   49
        {0x88D8762BF324CD0F, 0xA5880A69FB6AC800}, //   50
        {0xAB0E93B6EFEE0053, 0x8EEA0D047A457A00}, //   51
        {0xD5D238A4ABE98068, 0x72A4904598D6D880}, //   52
        {0x85A36366EB71F041, 0x47A6DA2B7F864750}, //   53
        {0xA70C3C40A64E6C51, 0x999090B65F67D924}, //   54
        {0xD0CF4B50CFE20765, 0xFFF4B4E3F741CF6D}, //   55
        {0x82818F1281ED449F, 0xBFF8F10E7A8921A5}, //   56
        {0xA321F2D7226895C7, 0xAFF72D52192B6A0E}, //   57
        {0xCBEA6F8CEB02BB39, 0x9BF4F8A69F764491}, //   58
        {0xFEE50B7025C36A08, 0x02F236D04753D5B5}, //   59
        {0x9F4F2726179A2245, 0x01D762422C946591}, //   60
        {0xC722F0EF9D80AAD6, 0x424D3AD2B7B97EF6}, //   61
        {0xF8EBAD2B84E0D58B, 0xD2E0898765A7DEB3}, //   62
        {0x9B934C3B330C8577, 0x63CC55F49F88EB30}, //   63
        {0xC2781F49FFCFA6D5, 0x3CBF6B71C76B25FC}, //   64
        {0xF316271C7FC3908A, 0x8BEF464E3945EF7B}, //   65
        {0x97EDD871CFDA3A56, 0x97758BF0E3CBB5AD}, //   66
        {0xBDE94E8E43D0C8EC, 0x3D52EEED1CBEA318}, //   67
        {0xED63A231D4C4FB27, 0x4CA7AAA863EE4BDE}, //   68
        {0x945E455F24FB1CF8, 0x8FE8CAA93E74EF6B}, //   69
        {0xB975D6B6EE39E436, 0xB3E2FD538E122B45}, //   70
        {0xE7D34C64A9C85D44, 0x60DBBCA87196B617}, //   71
        {0x90E40FBEEA1D3A4A, 0xBC8955E946FE31CE}, //   72
        {0xB51D13AEA4A488DD, 0x6BABAB6398BDBE42}, //   73
        {0xE264589A4DCDAB14, 0xC696963C7EED2DD2}, //   74
        {0x8D7EB76070A08AEC, 0xFC1E1DE5CF543CA3}, //   75
        {0xB0DE65388CC8ADA8, 0x3B25A55F43294BCC}, //   76
        {0xDD15FE86AFFAD912, 0x49EF0EB713F39EBF}, //   77
        {0x8A2DBF142DFCC7AB, 0x6E3569326C784338}, //   78
        {0xACB92ED9397BF996, 0x49C2C37F07965405}, //   79
        {0xD7E77A8F87DAF7FB, 0xDC33745EC97BE907}, //   80
        {0x86F0AC99B4E8DAFD, 0x69A028BB3DED71A4}, //   81
        {0xA8ACD7C0222311BC, 0xC40832EA0D68CE0D}, //   82
        {0xD2D80DB02AABD62B, 0xF50A3FA490C30191}, //   83
        {0x83C7088E1AAB65DB, 0x792667C6DA79E0FB}, //   84
        {0xA4B8CAB1A1563F52, 0x577001B891185939}, //   85
        {0xCDE6FD5E09ABCF26, 0xED4C0226B55E6F87}, //   86
        {0x80B05E5AC60B6178, 0x544F8158315B05B5}, //   87
        {0xA0DC75F1778E39D6, 0x696361AE3DB1C722}, //   88
        {0xC913936DD571C84C, 0x03BC3A19CD1E38EA}, //   89
        {0xFB5878494ACE3A5F, 0x04AB48A04065C724}, //   90
        {0x9D174B2DCEC0E47B, 0x62EB0D64283F9C77}, //   91
        {0xC45D1DF942711D9A, 0x3BA5D0BD324F8395}, //   92
        {0xF5746577930D6500, 0xCA8F44EC7EE3647A}, //   93
        {0x9968BF6ABBE85F20, 0x7E998B13CF4E1ECC}, //   94
        {0xBFC2EF456AE276E8, 0x9E3FEDD8C321A67F}, //   95
        {0xEFB3AB16C59B14A2, 0xC5CFE94EF3EA101F}, //   96
        {0x95D04AEE3B80ECE5, 0xBBA1F1D158724A13}, //   97
        {0xBB445DA9CA61281F, 0x2A8A6E45AE8EDC98}, //   98
        {0xEA1575143CF97226, 0xF52D09D71A3293BE}, //   99
        {0x924D692CA61BE758, 0x593C2626705F9C57}, //  100
        {0xB6E0C377CFA2E12E, 0x6F8B2FB00C77836D}, //  101
        {0xE498F455C38B997A, 0x0B6DFB9C0F956448}, //  102
        {0x8EDF98B59A373FEC, 0x4724BD4189BD5EAD}, //  103
        {0xB2977EE300C50FE7, 0x58EDEC91EC2CB658}, //  104
        {0xDF3D5E9BC0F653E1, 0x2F2967B66737E3EE}, //  105
        {0x8B865B215899F46C, 0xBD79E0D20082EE75}, //  106
        {0xAE67F1E9AEC07187, 0xECD8590680A3AA12}, //  107
        {0xDA01EE641A708DE9, 0xE80E6F4820CC9496}, //  108
        {0x884134FE908658B2, 0x3109058D147FDCDE}, //  109
        {0xAA51823E34A7EEDE, 0xBD4B46F0599FD416}, //  110
        {0xD4E5E2CDC1D1EA96, 0x6C9E18AC7007C91B}, //  111
        {0x850FADC09923329E, 0x03E2CF6BC604DDB1}, //  112
        {0xA6539930BF6BFF45, 0x84DB8346B786151D}, //  113
        {0xCFE87F7CEF46FF16, 0xE612641865679A64}, //  114
        {0x81F14FAE158C5F6E, 0x4FCB7E8F3F60C07F}, //  115
        {0xA26DA3999AEF7749, 0xE3BE5E330F38F09E}, //  116
        {0xCB090C8001AB551C, 0x5CADF5BFD3072CC6}, //  117
        {0xFDCB4FA002162A63, 0x73D9732FC7C8F7F7}, //  118
        {0x9E9F11C4014DDA7E, 0x2867E7FDDCDD9AFB}, //  119
        {0xC646D63501A1511D, 0xB281E1FD541501B9}, //  120
        {0xF7D88BC24209A565, 0x1F225A7CA91A4227}, //  121
        {0x9AE757596946075F, 0x3375788DE9B06959}, //  122
        {0xC1A12D2FC3978937, 0x0052D6B1641C83AF}, //  123
        {0xF209787BB47D6B84, 0xC0678C5DBD23A49B}, //  124
        {0x9745EB4D50CE6332, 0xF840B7BA963646E1}, //  125
        {0xBD176620A501FBFF, 0xB650E5A93BC3D899}, //  126
        {0xEC5D3FA8CE427AFF, 0xA3E51F138AB4CEBF}, //  127
        {0x93BA47C980E98CDF, 0xC66F336C36B10138}, //  128
        {0xB8A8D9BBE123F017, 0xB80B0047445D4185}, //  129
        {0xE6D3102AD96CEC1D, 0xA60DC059157491E6}, //  130
        {0x9043EA1AC7E41392, 0x87C89837AD68DB30}, //  131
        {0xB454E4A179DD1877, 0x29BABE4598C311FC}, //  132
        {0xE16A1DC9D8545E94, 0xF4296DD6FEF3D67B}, //  133
        {0x8CE2529E2734BB1D, 0x1899E4A65F58660D}, //  134
        {0xB01AE745B101E9E4, 0x5EC05DCFF72E7F90}, //  135
        {0xDC21A1171D42645D, 0x76707543F4FA1F74}, //  136
        {0x899504AE72497EBA, 0x6A06494A791C53A9}, //  137
        {0xABFA45DA0EDBDE69, 0x0487DB9D17636893}, //  138
        {0xD6F8D7509292D603, 0x45A9D2845D3C42B7}, //  139
        {0x865B86925B9BC5C2, 0x0B8A2392BA45A9B3}, //  140
        {0xA7F26836F282B732, 0x8E6CAC7768D7141F}, //  141
        {0xD1EF0244AF2364FF, 0x3207D795430CD927}, //  142
        {0x8335616AED761F1F, 0x7F44E6BD49E807B9}, //  143
        {0xA402B9C5A8D3A6E7, 0x5F16206C9C6209A7}, //  144
        {0xCD036837130890A1, 0x36DBA887C37A8C10}, //  145
        {0x802221226BE55A64, 0xC2494954DA2C978A}, //  146
        {0xA02AA96B06DEB0FD, 0xF2DB9BAA10B7BD6D}, //  147
        {0xC83553C5C8965D3D, 0x6F92829494E5ACC8}, //  148
        {0xFA42A8B73ABBF48C, 0xCB772339BA1F17FA}, //  149
        {0x9C69A97284B578D7, 0xFF2A760414536EFC}, //  150
        {0xC38413CF25E2D70D, 0xFEF5138519684ABB}, //  151
        {0xF46518C2EF5B8CD1, 0x7EB258665FC25D6A}, //  152
        {0x98BF2F79D5993802, 0xEF2F773FFBD97A62}, //  153
        {0xBEEEFB584AFF8603, 0xAAFB550FFACFD8FB}, //  154
        {0xEEAABA2E5DBF6784, 0x95BA2A53F983CF39}, //  155
        {0x952AB45CFA97A0B2, 0xDD945A747BF26184}, //  156
        {0xBA756174393D88DF, 0x94F971119AEEF9E5}, //  157
        {0xE912B9D1478CEB17, 0x7A37CD5601AAB85E}, //  158
        {0x91ABB422CCB812EE, 0xAC62E055C10AB33B}, //  159
        {0xB616A12B7FE617AA, 0x577B986B314D600A}, //  160
        {0xE39C49765FDF9D94, 0xED5A7E85FDA0B80C}, //  161
        {0x8E41ADE9FBEBC27D, 0x14588F13BE847308}, //  162
        {0xB1D219647AE6B31C, 0x596EB2D8AE258FC9}, //  163
        {0xDE469FBD99A05FE3, 0x6FCA5F8ED9AEF3BC}, //  164
        {0x8AEC23D680043BEE, 0x25DE7BB9480D5855}, //  165
        {0xADA72CCC20054AE9, 0xAF561AA79A10AE6B}, //  166
        {0xD910F7FF28069DA4, 0x1B2BA1518094DA05}, //  167
        {0x87AA9AFF79042286, 0x90FB44D2F05D0843}, //  168
        {0xA99541BF57452B28, 0x353A1607AC744A54}, //  169
        {0xD3FA922F2D1675F2, 0x42889B8997915CE9}, //  170
        {0x847C9B5D7C2E09B7, 0x69956135FEBADA12}, //  171
        {0xA59BC234DB398C25, 0x43FAB9837E699096}, //  172
        {0xCF02B2C21207EF2E, 0x94F967E45E03F4BC}, //  173
        {0x8161AFB94B44F57D, 0x1D1BE0EEBAC278F6}, //  174
        {0xA1BA1BA79E1632DC, 0x6462D92A69731733}, //  175
        {0xCA28A291859BBF93, 0x7D7B8F7503CFDCFF}, //  176
        {0xFCB2CB35E702AF78, 0x5CDA735244C3D43F}, //  177
        {0x9DEFBF01B061ADAB, 0x3A0888136AFA64A8}, //  178
        {0xC56BAEC21C7A1916, 0x088AAA1845B8FDD1}, //  179
        {0xF6C69A72A3989F5B, 0x8AAD549E57273D46}, //  180
        {0x9A3C2087A63F6399, 0x36AC54E2F678864C}, //  181
        {0xC0CB28A98FCF3C7F, 0x84576A1BB416A7DE}, //  182
        {0xF0FDF2D3F3C30B9F, 0x656D44A2A11C51D6}, //  183
        {0x969EB7C47859E743, 0x9F644AE5A4B1B326}, //  184
        {0xBC4665B596706114, 0x873D5D9F0DDE1FEF}, //  185
        {0xEB57FF22FC0C7959, 0xA90CB506D155A7EB}, //  186
        {0x9316FF75DD87CBD8, 0x09A7F12442D588F3}, //  187
        {0xB7DCBF5354E9BECE, 0x0C11ED6D538AEB30}, //  188
        {0xE5D3EF282A242E81, 0x8F1668C8A86DA5FB}, //  189
        {0x8FA475791A569D10, 0xF96E017D694487BD}, //  190
        {0xB38D92D760EC4455, 0x37C981DCC395A9AD}, //  191
        {0xE070F78D3927556A, 0x85BBE253F47B1418}, //  192
        {0x8C469AB843B89562, 0x93956D7478CCEC8F}, //  193
        {0xAF58416654A6BABB, 0x387AC8D1970027B3}, //  194
        {0xDB2E51BFE9D0696A, 0x06997B05FCC0319F}, //  195
        {0x88FCF317F22241E2, 0x441FECE3BDF81F04}, //  196
        {0xAB3C2FDDEEAAD25A, 0xD527E81CAD7626C4}, //  197
        {0xD60B3BD56A5586F1, 0x8A71E223D8D3B075}, //  198
        {0x85C7056562757456, 0xF6872D5667844E4A}, //  199
        {0xA738C6BEBB12D16C, 0xB428F8AC016561DC}, //  200
        {0xD106F86E69D785C7, 0xE13336D701BEBA53}, //  201
        {0x82A45B450226B39C, 0xECC0024661173474}, //  202
        {0xA34D721642B06084, 0x27F002D7F95D0191}, //  203
        {0xCC20CE9BD35C78A5, 0x31EC038DF7B441F5}, //  204
        {0xFF290242C83396CE, 0x7E67047175A15272}, //  205
        {0x9F79A169BD203E41, 0x0F0062C6E984D387}, //  206
        {0xC75809C42C684DD1, 0x52C07B78A3E60869}, //  207
        {0xF92E0C3537826145, 0xA7709A56CCDF8A83}, //  208
        {0x9BBCC7A142B17CCB, 0x88A66076400BB692}, //  209
        {0xC2ABF989935DDBFE, 0x6ACFF893D00EA436}, //  210
        {0xF356F7EBF83552FE, 0x0583F6B8C4124D44}, //  211
        {0x98165AF37B2153DE, 0xC3727A337A8B704B}, //  212
        {0xBE1BF1B059E9A8D6, 0x744F18C0592E4C5D}, //  213
        {0xEDA2EE1C7064130C, 0x1162DEF06F79DF74}, //  214
        {0x9485D4D1C63E8BE7, 0x8ADDCB5645AC2BA9}, //  215
        {0xB9A74A0637CE2EE1, 0x6D953E2BD7173693}, //  216
        {0xE8111C87C5C1BA99, 0xC8FA8DB6CCDD0438}, //  217
        {0x910AB1D4DB9914A0, 0x1D9C9892400A22A3}, //  218
        {0xB54D5E4A127F59C8, 0x2503BEB6D00CAB4C}, //  219
        {0xE2A0B5DC971F303A, 0x2E44AE64840FD61E}, //  220
        {0x8DA471A9DE737E24, 0x5CEAECFED289E5D3}, //  221
        {0xB10D8E1456105DAD, 0x7425A83E872C5F48}, //  222
        {0xDD50F1996B947518, 0xD12F124E28F7771A}, //  223
        {0x8A5296FFE33CC92F, 0x82BD6B70D99AAA70}, //  224
        {0xACE73CBFDC0BFB7B, 0x636CC64D1001550C}, //  225
        {0xD8210BEFD30EFA5A, 0x3C47F7E05401AA4F}, //  226
        {0x8714A775E3E95C78, 0x65ACFAEC34810A72}, //  227
        {0xA8D9D1535CE3B396, 0x7F1839A741A14D0E}, //  228
        {0xD31045A8341CA07C, 0x1EDE48111209A051}, //  229
        {0x83EA2B892091E44D, 0x934AED0AAB460433}, //  230
        {0xA4E4B66B68B65D60, 0xF81DA84D56178540}, //  231
        {0xCE1DE40642E3F4B9, 0x36251260AB9D668F}, //  232
        {0x80D2AE83E9CE78F3, 0xC1D72B7C6B42601A}, //  233
        {0xA1075A24E4421730, 0xB24CF65B8612F820}, //  234
        {0xC94930AE1D529CFC, 0xDEE033F26797B628}, //  235
        {0xFB9B7CD9A4A7443C, 0x169840EF017DA3B2}, //  236
        {0x9D412E0806E88AA5, 0x8E1F289560EE864F}, //  237
        {0xC491798A08A2AD4E, 0xF1A6F2BAB92A27E3}, //  238
        {0xF5B5D7EC8ACB58A2, 0xAE10AF696774B1DC}, //  239
        {0x9991A6F3D6BF1765, 0xACCA6DA1E0A8EF2A}, //  240
        {0xBFF610B0CC6EDD3F, 0x17FD090A58D32AF4}, //  241
        {0xEFF394DCFF8A948E, 0xDDFC4B4CEF07F5B1}, //  242
        {0x95F83D0A1FB69CD9, 0x4ABDAF101564F98F}, //  243
        {0xBB764C4CA7A4440F, 0x9D6D1AD41ABE37F2}, //  244
        {0xEA53DF5FD18D5513, 0x84C86189216DC5EE}, //  245
        {0x92746B9BE2F8552C, 0x32FD3CF5B4E49BB5}, //  246
        {0xB7118682DBB66A77, 0x3FBC8C33221DC2A2}, //  247
        {0xE4D5E82392A40515, 0x0FABAF3FEAA5334B}, //  248
        {0x8F05B1163BA6832D, 0x29CB4D87F2A7400F}, //  249
        {0xB2C71D5BCA9023F8, 0x743E20E9EF511013}, //  250
        {0xDF78E4B2BD342CF6, 0x914DA9246B255417}, //  251
        {0x8BAB8EEFB6409C1A, 0x1AD089B6C2F7548F}, //  252
        {0xAE9672ABA3D0C320, 0xA184AC2473B529B2}, //  253
        {0xDA3C0F568CC4F3E8, 0xC9E5D72D90A2741F}, //  254
        {0x8865899617FB1871, 0x7E2FA67C7A658893}, //  255
        {0xAA7EEBFB9DF9DE8D, 0xDDBB901B98FEEAB8}, //  256
        {0xD51EA6FA85785631, 0x552A74227F3EA566}, //  257
        {0x8533285C936B35DE, 0xD53A88958F872760}, //  258
        {0xA67FF273B8460356, 0x8A892ABAF368F138}, //  259
        {0xD01FEF10A657842C, 0x2D2B7569B0432D86}, //  260
        {0x8213F56A67F6B29B, 0x9C3B29620E29FC74}, //  261
        {0xA298F2C501F45F42, 0x8349F3BA91B47B90}, //  262
        {0xCB3F2F7642717713, 0x241C70A936219A74}, //  263
        {0xFE0EFB53D30DD4D7, 0xED238CD383AA0111}, //  264
        {0x9EC95D1463E8A506, 0xF4363804324A40AB}, //  265
        {0xC67BB4597CE2CE48, 0xB143C6053EDCD0D6}, //  266
        {0xF81AA16FDC1B81DA, 0xDD94B7868E94050B}, //  267
        {0x9B10A4E5E9913128, 0xCA7CF2B4191C8327}, //  268
        {0xC1D4CE1F63F57D72, 0xFD1C2F611F63A3F1}, //  269
        {0xF24A01A73CF2DCCF, 0xBC633B39673C8CED}, //  270
        {0x976E41088617CA01, 0xD5BE0503E085D814}, //  271
        {0xBD49D14AA79DBC82, 0x4B2D8644D8A74E19}, //  272
        {0xEC9C459D51852BA2, 0xDDF8E7D60ED1219F}, //  273
        {0x93E1AB8252F33B45, 0xCABB90E5C942B504}, //  274
        {0xB8DA1662E7B00A17, 0x3D6A751F3B936244}, //  275
        {0xE7109BFBA19C0C9D, 0x0CC512670A783AD5}, //  276
        {0x906A617D450187E2, 0x27FB2B80668B24C6}, //  277
        {0xB484F9DC9641E9DA, 0xB1F9F660802DEDF7}, //  278
        {0xE1A63853BBD26451, 0x5E7873F8A0396974}, //  279
        {0x8D07E33455637EB2, 0xDB0B487B6423E1E9}, //  280
        {0xB049DC016ABC5E5F, 0x91CE1A9A3D2CDA63}, //  281
        {0xDC5C5301C56B75F7, 0x7641A140CC7810FC}, //  282
        {0x89B9B3E11B6329BA, 0xA9E904C87FCB0A9E}, //  283
        {0xAC2820D9623BF429, 0x546345FA9FBDCD45}, //  284
        {0xD732290FBACAF133, 0xA97C177947AD4096}, //  285
        {0x867F59A9D4BED6C0, 0x49ED8EABCCCC485E}, //  286
        {0xA81F301449EE8C70, 0x5C68F256BFFF5A75}, //  287
        {0xD226FC195C6A2F8C, 0x73832EEC6FFF3112}, //  288
        {0x83585D8FD9C25DB7, 0xC831FD53C5FF7EAC}, //  289
        {0xA42E74F3D032F525, 0xBA3E7CA8B77F5E56}, //  290
        {0xCD3A1230C43FB26F, 0x28CE1BD2E55F35EC}, //  291
        {0x80444B5E7AA7CF85, 0x7980D163CF5B81B4}, //  292
        {0xA0555E361951C366, 0xD7E105BCC3326220}, //  293
        {0xC86AB5C39FA63440, 0x8DD9472BF3FEFAA8}, //  294
        {0xFA856334878FC150, 0xB14F98F6F0FEB952}, //  295
        {0x9C935E00D4B9D8D2, 0x6ED1BF9A569F33D4}, //  296
        {0xC3B8358109E84F07, 0x0A862F80EC4700C9}, //  297
        {0xF4A642E14C6262C8, 0xCD27BB612758C0FB}, //  298
        {0x98E7E9CCCFBD7DBD, 0x8038D51CB897789D}, //  299
        {0xBF21E44003ACDD2C, 0xE0470A63E6BD56C4}, //  300
        {0xEEEA5D5004981478, 0x1858CCFCE06CAC75}, //  301
        {0x95527A5202DF0CCB, 0x0F37801E0C43EBC9}, //  302
        {0xBAA718E68396CFFD, 0xD30560258F54E6BB}, //  303
        {0xE950DF20247C83FD, 0x47C6B82EF32A206A}, //  304
        {0x91D28B7416CDD27E, 0x4CDC331D57FA5442}, //  305
        {0xB6472E511C81471D, 0xE0133FE4ADF8E953}, //  306
        {0xE3D8F9E563A198E5, 0x58180FDDD97723A7}, //  307
        {0x8E679C2F5E44FF8F, 0x570F09EAA7EA7649}, //  308
        {0xB201833B35D63F73, 0x2CD2CC6551E513DB}, //  309
        {0xDE81E40A034BCF4F, 0xF8077F7EA65E58D2}, //  310
        {0x8B112E86420F6191, 0xFB04AFAF27FAF783}, //  311
        {0xADD57A27D29339F6, 0x79C5DB9AF1F9B564}, //  312
        {0xD94AD8B1C7380874, 0x18375281AE7822BD}, //  313
        {0x87CEC76F1C830548, 0x8F2293910D0B15B6}, //  314
        {0xA9C2794AE3A3C69A, 0xB2EB3875504DDB23}, //  315
        {0xD433179D9C8CB841, 0x5FA60692A46151EC}, //  316
        {0x849FEEC281D7F328, 0xDBC7C41BA6BCD334}, //  317
        {0xA5C7EA73224DEFF3, 0x12B9B522906C0801}, //  318
        {0xCF39E50FEAE16BEF, 0xD768226B34870A01}, //  319
        {0x81842F29F2CCE375, 0xE6A1158300D46641}, //  320
        {0xA1E53AF46F801C53, 0x60495AE3C1097FD1}, //  321
        {0xCA5E89B18B602368, 0x385BB19CB14BDFC5}, //  322
        {0xFCF62C1DEE382C42, 0x46729E03DD9ED7B6}, //  323
        {0x9E19DB92B4E31BA9, 0x6C07A2C26A8346D2}, //  324
    };

    u64 M52 = (1ull << 52) - 1;
    u64 M63 = (1ull << 63) - 1;
    u64 vi = *(u64 *)&v;
    i64 vi64 = *(i64 *)&v;
    u64 vi_abs = vi & M63;
    double v_abs = *(double *)&vi_abs; // abs(v)
    u64 v_to_u64 = v_abs;              // cvt to unsigned long long
    double u64_to_v = v_to_u64;        // cvt to double
    i64 v_to_i64 = v;                  // cvt to long long
    double i64_to_v = v_to_i64;        // cvt to double
    u64 sign = (vi >> 63);
    i64 exp = vi_abs >> 52;
    u64 frac = vi & M52;
#if 1 // print integer
    // if( (vi64 == *(i64*)&i64_to_v) & ( abs(v_to_i64) <= (i64)( ((1ll<<53) - 1) ) )  ) // branch miss may high in random data
    if ((vi_abs == *(u64 *)&u64_to_v) & (v_to_u64 <= (u64)(((1ull << 53) - 1))))
    {
        // small_int = 1;
        // printf("small integer v_to_i64 = %lld \n", v_to_u64);
#if 1
        buffer[0] = '-';
        // int len = static_cast<int>(itoa_i64_yy((int64_t)v_to_i64,buffer) - buffer);
        int len = static_cast<int>(itoa_u64_impl((uint64_t)v_to_u64, buffer + sign) - buffer);
        buffer[len] = '\0';
        return len;
#else
        // return i64toa_sse2(v_to_i64,buffer);
        buffer[0] = '-';
        return sign + u64toa_sse2(v_to_u64, buffer + sign);
#endif
    }
#endif
    buffer[0] = '-';
    buffer += sign;
    if (exp == 0x7ff)
    {
        *(int *)buffer = frac ? *(int *)"nan" : *(int *)"inf";
        return sign + 3;
    }
    // if (vi_abs == 0)
    // {
    //     *(short *)buffer = *(short *)"0";
    //     return sign + 1;
    // }
    u64 normal = (exp > 0);
    u64 c;
    ll q;
    if (normal) // [[likely]]
    {
        c = frac | (1ull << 52);
        q = exp - 1075;
    }
    else
    {
        c = frac;
        q = -1074;
    }
    *(u64 *)buffer = *(u64 *)"0.000000"; // 8byte
    u64 cbl;
    u64 cb = c << 2;
    u64 cbr = cb | 2;
    ll k;
    bool lower_boundary_is_closer = (frac == 0);
    if (lower_boundary_is_closer) [[unlikely]]
    {
        cbl = cb - 1;
        k = (q * 1262611 - 524031) >> 22;
    }
    else
    {
        cbl = cb - 2;
        k = (q * 1262611) >> 22;
    }
    ll h = q + (((k) * (-1741647)) >> 19) + 1; //[1,4]
    const u64x2 *pow10_ptr = &g[-kMin];
    u64x2 pow10 = pow10_ptr[-k];
    u64 lo = pow10.lo;
    u64 hi = pow10.hi;
    u64 cp = cbl << h;
    u64 x1;
    u64 y1;
    _mulx_u64(lo, cp, &x1);
    u64 y0 = _mulx_u64(hi, cp, &y1);
    // y0 += x1;
    // y1 += y0 < x1;
    _addcarry_u64(_addcarry_u64(0, y0, x1, &y0), y1, 0, &y1);
    // y1 y0 x0
    u64 vbl = y1 | (y0 > 1);
    // u128 y1y0 = _mm_set_epi64x(y1,y0);
    // u128 y12 = _mm_set1_epi64x(y1);
    // u128 y02 = _mm_set1_epi64x(y0);

    // pow10_hi>>(64-(h+1))
    // pow10_hi<<(h+1) | pow10_lo>>(64-(h+1))
    // pow10_lo<<(h+1)
    //u64 r1 = hi >> (63 - h);
    //u64 r2 = ((hi << h) << 1) | (lo >> (63 - h));

    // u128 r12 = hi2 >> (63 - h2);
    // u128 r22 = (hi2 << (h2 + 1)) | (lo2 >> (63 - h2));
    // u64 r3 = lo << (h + 1);

    // u64 r3_vb = r3 + x0;
    //  u64 r2_vb = r2 + y0;
    //  u64 r1_vb = r1 + y1 + (r2_vb < r2);
    //  //u128 r22_vb = r22 + y02;
    //  //u128 r12_vb = r12 + y12 + (r22_vb < r22);
    //  u64 vb = r1_vb | (r2_vb > 1);
    // u128 vb2 = r12_vb | (r22_vb > 1);
    //  u64 vbl;
    // u64 r3_vbr = r3 + r3_vb;
    //u64 r2_vbr = r2 + y0;
    //u64 r1_vbr = r1 + y1 + (r2_vbr < r2);
    // u128 r22_vbr = r22 + r22_vb;
    // u128 r12_vbr = r12 + r12_vb + (r22_vbr < r22);
    //u64 vbr = r1_vbr | (r2_vbr > 1);
    // u128 vbr2 = r12_vbr | (r22_vbr > 1);
    // u64 vbr;

    //u64 vbl = RoundToOdd(pow10, cbl << h);
    u64 vb = RoundToOdd(pow10, cb << h);
    u64 vbr = RoundToOdd(pow10, cbr << h); // (4c + 2) << h; // vbr - vb = 2 << h
    u64 lower = vbl + (c & 1);
    u64 upper = vbr - (c & 1);
    // u64 lower2 = lower>>2;
    // u64 upper2 = upper>>2;;
    u64 s = vb >> 2;
    u64 sp;
    _mulx_u64(1844674407370955162ull, s, &sp); // sp = s/10
    u64 sp10 = sp * 10;
    u64 digit_out = s;
    // s += ((((vb & -4) < lower) | ((vb | 3) < (upper))) | (0b11001000 >> (vb & 7)) & 1); // s or s + 1
    digit_out += (((vb & -4) < lower) |
                  ((vb | 3) < upper) |
                  ((vb & 3) == 3) |
                  ((vb & 7) == 6)); // s or s + 1
    if (lower <= sp10 * 4)
        digit_out = sp10;
    if ((sp10 + 10) * 4 <= upper)
        digit_out = sp10 + 10;
    // compute digit_out and k end; then print to buffer
    // result = digit_out * 10^k
    ll e10 = k;
    if (((u64)(1e15) <= digit_out) & (digit_out < (u64)(1e16))) // 16digit
    // if (  (digit_out - (u64)(1e15)) <  ( (u64)(9e15)  )  )
    {
        digit_out *= 10; // format to 17 digit
        e10 += 15;
    }
    else
    {
        e10 += 16;
    }
    int flag = ((-7 <= e10) & (e10 <= -1));
    int flag2 = ((-7 <= e10) & (e10 <= 0));
    ll high1 = digit_out / (ll)(1e16);      // [1,9]
    ll high9 = digit_out / (ll)(1e8);       // 1e8 <= high9 < 1e9
    ll high2_9 = high9 - high1 * (ll)(1e8); // 0<= high2_9 < 1e8
    __m512i h9_4r = _mm512_castsi256_si512(_mm256_set1_epi64x(high2_9));
    ll low8 = digit_out - high9 * (ll)(1e8); // 0<= low8 < 1e8
    int ctz;
    // ctz= count_tz(high9, low8);

    // count tail zero , avx512 instructions
    {
        __m512i h9 = _mm512_set1_epi64(high9); // set high2_9 also right
        __m512i l8 = _mm512_set1_epi64(low8);
        __m512i all = merge_l32(h9, l8); // high 32 bit is 0
        const int e57_e8 = 1441151881;   // ceil(2**57/1e8)
        const int e54_e7 = 1801439851;   // ceil(2**54/1e7)
        const int e50_e6 = 1125899907;
        const int e47_e5 = 1407374884;
        const int e44_e4 = 1759218605;
        const int e40_e3 = 1099511628;
        const int e37_e2 = 1374389535;
        const int e34_e1 = 1717986919;
        const __m512i n8 = _mm512_set_epi64(e57_e8, e54_e7, e50_e6, e47_e5, e44_e4, e40_e3, e37_e2, e34_e1);
        const __m512i r16 = _mm512_set_epi32(25, 22, 18, 15, 12, 8, 5, 2, 25, 22, 18, 15, 12, 8, 5, 2);
        const __m512i m16 = _mm512_set_epi32(1e8, 1e7, 1e6, 1e5, 1e4, 1e3, 1e2, 1e1, 1e8, 1e7, 1e6, 1e5, 1e4, 1e3, 1e2, 1e1);
        // __m512i h9_n8 = _mm512_mullo_epi64(h9, n8);
        // __m512i l8_n8 = _mm512_mullo_epi64(l8, n8);
        __m512i h9_n8 = _mm512_mul_epu32(h9, n8);
        __m512i l8_n8 = _mm512_mul_epu32(l8, n8);
        __m512i h9_l8 = merge_h32(h9_n8, l8_n8); // only need high 32 bit
        __m512i res17 = _mm512_mullo_epi32(_mm512_srlv_epi32(h9_l8, r16), m16);
        __mmask16 mask17 = _mm512_cmpneq_epi32_mask(res17, all);
        ctz = _tzcnt_u64(mask17 | (0x20000 >> flag));
    }
    int exp10_length = 4 | ((e10 >= 100) | (e10 <= -100)); // 4 + abs(e10)>=100
    char *ptr = buffer;
    // use avx512 instruction to calc low 16 digit ascii
    __m256i l8_4r = _mm256_set1_epi64x(low8);
    __m512i n8r = _mm512_inserti64x4(h9_4r, l8_4r, 1);
    const ull m8 = 180143986;       //>>> 2**54/1e8
    const ull m6 = 18014398510;     //>>> 2**54/1e6
    const ull m4 = 1801439850949;   //>>> 2**54/1e4
    const ull m2 = 180143985094820; //>>> 2**54/1e2
    const __m512i mr = _mm512_set_epi64(m8, m6, m4, m2, m8, m6, m4, m2);
    const ull M54 = (1ull << 54) - 1;
    const ull M8 = 0xff00;
    const __m512i M54_8_all = _mm512_set1_epi64(M54);
    const __m512i M8_8_2 = _mm512_set1_epi64(M8);
    const __m512i t10r = _mm512_set1_epi64(10);
    __m512i tmp_8_0 = _mm512_mullo_epi64(n8r, mr);
    __m512i tmp_8_1 = _mm512_and_epi64(tmp_8_0, M54_8_all);
    __m512i tmp_8_2 = _mm512_mullo_epi64(tmp_8_1, t10r);
    __m512i tmp_8_3_t = _mm512_and_epi64(tmp_8_2, M54_8_all);
    __m512i tmp_8_3 = _mm512_mullo_epi64(tmp_8_3_t, t10r);
    __m512i tmp_8_1_print = _mm512_srli_epi64(tmp_8_2, 54);
    __m512i tmp_8_2_print = _mm512_and_epi64(_mm512_srli_epi64(tmp_8_3, (54 - 8)), M8_8_2);
    __m512i tmp_8_3_print = _mm512_set1_epi64(0x3030) | tmp_8_1_print | tmp_8_2_print; // 0x30 is '0'
    const short idx[8] = {12, 8, 4, 0, 28, 24, 20, 16};                                // 16byte
    const __m512i idxr_epi16 = _mm512_castsi128_si512(_mm_loadu_epi64(idx));
    __m512i num_8_print_finalr = _mm512_permutexvar_epi16(idxr_epi16, tmp_8_3_print); // avx512bw

    ll start_write_pos = flag ? (1 - e10) : 0; // when e10 range in [-7,-1] , start_pos = 1-e10
    *(short *)(ptr + start_write_pos) = high1 | ('.' * 256 + '0');
    _mm_storeu_si128((__m128i *)(ptr + start_write_pos + 2 - flag), _mm512_extracti32x4_epi32(num_8_print_finalr, 0));
    //_mm_storeu_si128((__m128i *)(ptr + start_write_pos + 2 - flag), num_low16_print);
    //_mm_storeu_si128((__m128i *)(ptr + 2), num_final | DIGIT_ZERO);
    // if(0)
    if (((1 <= e10) & (e10 <= 8))) // not exec this code also right
    {
        // v range in [1e1 ,1e9)
        // 23 -> 23
        // 100 -> 100
        // 12.34 -> 12.34
        ll float_point_pos = e10 + 1;
        ll dot_right_value = *(ll *)(ptr + float_point_pos + 1);
        ll dot_left_value = _mm_extract_epi64(_mm512_extracti32x4_epi32(num_8_print_finalr, 0), 0); // high2_9 print result; sse4.1
        *(ll *)(ptr + 1) = dot_left_value;

        // origin code ; when print integer not open
        // *(ptr + float_point_pos) = (ctz + e10 < 16) ? '.' : 0;                     // xx.00 ; if dot right value = all 0 , not print dot
        // *(ll *)(ptr + float_point_pos + 1) = dot_right_value;                      //
        // int end_pos = (18 - ctz > float_point_pos) ? (18 - ctz) : float_point_pos; // max(18-ctz,float_point_pos)
        // *(ptr + end_pos) = '\0';                                                   // remove tail zero
        // return sign + ((ctz + e10 < 16) ? end_pos : float_point_pos);              // length = if(ctz + e10 < 16) end_pos else float_point_pos

        // when print integer open,use this code
        *(ptr + float_point_pos) = '.';
        *(ll *)(ptr + float_point_pos + 1) = dot_right_value;
        *(ptr + 18 - ctz) = '\0';
        return sign + 18 - ctz;
    }
    const ll *exp_ptr = &exp_result3[324];
    *(ll *)(ptr + 18 - flag + start_write_pos - ctz) = ((-7 <= e10) & (e10 <= 0)) ? 0 : exp_ptr[e10]; // remove tail zero
    // when double value < 1e-309 ; equal digit_out < 1e15
    // also 5e-324 <= v < 1e-309 ;
    if (digit_out < (ull)(1e15)) // [[unlikely]]
    {
        char *buf_ptr = ptr;
        u64 len = dec_length(digit_out); // 1<= len <= 15
        u64 tz_num = ctz;
        u64 lz_num = 17 - len; // left zero num: lz_num >= 2
        // such as 00000001234500000 ; significant = 5,tz = 5,lz= 7
        // u64 signficant = 17 - tz_num - lz_num;// also = len - ctz
        u64 signficant = len - tz_num;
        u64 start_pos = lz_num + 1; // 0.00xxx ; first x pos is 3 + 1
        if (signficant > 1ull)      // [[likely]]
        {
            buf_ptr[0] = buf_ptr[start_pos];
            if (signficant <= 9) // move 8 byte
                *(i64 *)&buf_ptr[2] = *(i64 *)&buf_ptr[start_pos + 1];
            else
            { // move 16 byte
                *(i64 *)&buf_ptr[2] = *(i64 *)&buf_ptr[start_pos + 1];
                *(i64 *)&buf_ptr[2 + 8] = *(i64 *)&buf_ptr[start_pos + 1 + 8];
            }
            *(i64 *)&buf_ptr[signficant + 1] = exp_ptr[e10 - lz_num]; // write exp10 ASCII
            return sign + 1 + signficant + 5;                         // 1.2e-310 , [1.2] -> 1+signficant , [e-310] -> 5  ; sign is 1, negative double
        }
        else
        {
            *(char *)&buf_ptr[0] = buf_ptr[start_pos];
            *(i64 *)&buf_ptr[1] = exp_ptr[e10 - lz_num];
            return sign + 1 + 5; // example : 5e-324 , 1e-310 , 1e-323 ; sign is 1, negative double
        }
    }
    //int exp10_length = 4 | ((e10 >= 100) | (e10 <= -100)); // 4 + abs(e10)>=100
    return sign + start_write_pos + 18 - ctz - flag + (((-7 <= e10) & (e10 <= 0)) ? 0 : exp10_length);
}

#endif


static inline int d2s_sse(double v, char *buffer)
{
    //============ README start===========

    // this code base on Schubafach algorithm
    // sse implementation
    // I have limited time, so the performance may not be optimal.

    // performance test result
    // when input double is random
    // CPU : AMD R7 7840H ; use icpx 2025.0.4 -O3 compile this func
    // performance: this func cost 12.1ns per double , 1.82x faster than dragonbox , 2x faster than schubafach
    // dragonbox  cost 22ns per double
    // schubafach cost 25ns per double
    // ryu        cost 27ns per double

    // return write buffer length
    // recommend set buffer length is 32 byte;

    // input double v range and print result ;
    // range       : double v ->   print result
    // 1. v<1e-7   : 1.23e-8  ->   1.23e-08  ,  1.23e-11 -> 1.23e-11  , 1.23e-103  -> 1.23e-103
    // 2. [1e-7,1) : 1.23e-3  ->   0.00123   ,  1.23e-1  -> 0.123
    // 3. [1,1e9)  : 1.23e2   ->   123       ,  1.23e5   -> 12300
    // 4. >=1e9    : 1.23e10  ->   1.23e+10  ,  1.23e9   -> 1.23e+09  , 1.23e103   -> 1.23e+103

    // special value : 0 -> 0 , nan -> nan , inf -> inf

    // The code may not be the final version , may optimize performance
    //============ README end===========

    // caller-saved registers
    // rax, rcx, rdx , r8-r11 , xmm0-xmm5
    // 7 + 6 = 13 registers
    // how to reduce register use , how to use xmm registers to improve performance

    const int kMin = -292;
    const int kMax = 324;
    static const u64x2 g[kMax - kMin + 1] = {
        {0xFF77B1FCBEBCDC4F, 0x25E8E89C13BB0F7B}, // -292
        {0x9FAACF3DF73609B1, 0x77B191618C54E9AD}, // -291
        {0xC795830D75038C1D, 0xD59DF5B9EF6A2418}, // -290
        {0xF97AE3D0D2446F25, 0x4B0573286B44AD1E}, // -289
        {0x9BECCE62836AC577, 0x4EE367F9430AEC33}, // -288
        {0xC2E801FB244576D5, 0x229C41F793CDA740}, // -287
        {0xF3A20279ED56D48A, 0x6B43527578C11110}, // -286
        {0x9845418C345644D6, 0x830A13896B78AAAA}, // -285
        {0xBE5691EF416BD60C, 0x23CC986BC656D554}, // -284
        {0xEDEC366B11C6CB8F, 0x2CBFBE86B7EC8AA9}, // -283
        {0x94B3A202EB1C3F39, 0x7BF7D71432F3D6AA}, // -282
        {0xB9E08A83A5E34F07, 0xDAF5CCD93FB0CC54}, // -281
        {0xE858AD248F5C22C9, 0xD1B3400F8F9CFF69}, // -280
        {0x91376C36D99995BE, 0x23100809B9C21FA2}, // -279
        {0xB58547448FFFFB2D, 0xABD40A0C2832A78B}, // -278
        {0xE2E69915B3FFF9F9, 0x16C90C8F323F516D}, // -277
        {0x8DD01FAD907FFC3B, 0xAE3DA7D97F6792E4}, // -276
        {0xB1442798F49FFB4A, 0x99CD11CFDF41779D}, // -275
        {0xDD95317F31C7FA1D, 0x40405643D711D584}, // -274
        {0x8A7D3EEF7F1CFC52, 0x482835EA666B2573}, // -273
        {0xAD1C8EAB5EE43B66, 0xDA3243650005EED0}, // -272
        {0xD863B256369D4A40, 0x90BED43E40076A83}, // -271
        {0x873E4F75E2224E68, 0x5A7744A6E804A292}, // -270
        {0xA90DE3535AAAE202, 0x711515D0A205CB37}, // -269
        {0xD3515C2831559A83, 0x0D5A5B44CA873E04}, // -268
        {0x8412D9991ED58091, 0xE858790AFE9486C3}, // -267
        {0xA5178FFF668AE0B6, 0x626E974DBE39A873}, // -266
        {0xCE5D73FF402D98E3, 0xFB0A3D212DC81290}, // -265
        {0x80FA687F881C7F8E, 0x7CE66634BC9D0B9A}, // -264
        {0xA139029F6A239F72, 0x1C1FFFC1EBC44E81}, // -263
        {0xC987434744AC874E, 0xA327FFB266B56221}, // -262
        {0xFBE9141915D7A922, 0x4BF1FF9F0062BAA9}, // -261
        {0x9D71AC8FADA6C9B5, 0x6F773FC3603DB4AA}, // -260
        {0xC4CE17B399107C22, 0xCB550FB4384D21D4}, // -259
        {0xF6019DA07F549B2B, 0x7E2A53A146606A49}, // -258
        {0x99C102844F94E0FB, 0x2EDA7444CBFC426E}, // -257
        {0xC0314325637A1939, 0xFA911155FEFB5309}, // -256
        {0xF03D93EEBC589F88, 0x793555AB7EBA27CB}, // -255
        {0x96267C7535B763B5, 0x4BC1558B2F3458DF}, // -254
        {0xBBB01B9283253CA2, 0x9EB1AAEDFB016F17}, // -253
        {0xEA9C227723EE8BCB, 0x465E15A979C1CADD}, // -252
        {0x92A1958A7675175F, 0x0BFACD89EC191ECA}, // -251
        {0xB749FAED14125D36, 0xCEF980EC671F667C}, // -250
        {0xE51C79A85916F484, 0x82B7E12780E7401B}, // -249
        {0x8F31CC0937AE58D2, 0xD1B2ECB8B0908811}, // -248
        {0xB2FE3F0B8599EF07, 0x861FA7E6DCB4AA16}, // -247
        {0xDFBDCECE67006AC9, 0x67A791E093E1D49B}, // -246
        {0x8BD6A141006042BD, 0xE0C8BB2C5C6D24E1}, // -245
        {0xAECC49914078536D, 0x58FAE9F773886E19}, // -244
        {0xDA7F5BF590966848, 0xAF39A475506A899F}, // -243
        {0x888F99797A5E012D, 0x6D8406C952429604}, // -242
        {0xAAB37FD7D8F58178, 0xC8E5087BA6D33B84}, // -241
        {0xD5605FCDCF32E1D6, 0xFB1E4A9A90880A65}, // -240
        {0x855C3BE0A17FCD26, 0x5CF2EEA09A550680}, // -239
        {0xA6B34AD8C9DFC06F, 0xF42FAA48C0EA481F}, // -238
        {0xD0601D8EFC57B08B, 0xF13B94DAF124DA27}, // -237
        {0x823C12795DB6CE57, 0x76C53D08D6B70859}, // -236
        {0xA2CB1717B52481ED, 0x54768C4B0C64CA6F}, // -235
        {0xCB7DDCDDA26DA268, 0xA9942F5DCF7DFD0A}, // -234
        {0xFE5D54150B090B02, 0xD3F93B35435D7C4D}, // -233
        {0x9EFA548D26E5A6E1, 0xC47BC5014A1A6DB0}, // -232
        {0xC6B8E9B0709F109A, 0x359AB6419CA1091C}, // -231
        {0xF867241C8CC6D4C0, 0xC30163D203C94B63}, // -230
        {0x9B407691D7FC44F8, 0x79E0DE63425DCF1E}, // -229
        {0xC21094364DFB5636, 0x985915FC12F542E5}, // -228
        {0xF294B943E17A2BC4, 0x3E6F5B7B17B2939E}, // -227
        {0x979CF3CA6CEC5B5A, 0xA705992CEECF9C43}, // -226
        {0xBD8430BD08277231, 0x50C6FF782A838354}, // -225
        {0xECE53CEC4A314EBD, 0xA4F8BF5635246429}, // -224
        {0x940F4613AE5ED136, 0x871B7795E136BE9A}, // -223
        {0xB913179899F68584, 0x28E2557B59846E40}, // -222
        {0xE757DD7EC07426E5, 0x331AEADA2FE589D0}, // -221
        {0x9096EA6F3848984F, 0x3FF0D2C85DEF7622}, // -220
        {0xB4BCA50B065ABE63, 0x0FED077A756B53AA}, // -219
        {0xE1EBCE4DC7F16DFB, 0xD3E8495912C62895}, // -218
        {0x8D3360F09CF6E4BD, 0x64712DD7ABBBD95D}, // -217
        {0xB080392CC4349DEC, 0xBD8D794D96AACFB4}, // -216
        {0xDCA04777F541C567, 0xECF0D7A0FC5583A1}, // -215
        {0x89E42CAAF9491B60, 0xF41686C49DB57245}, // -214
        {0xAC5D37D5B79B6239, 0x311C2875C522CED6}, // -213
        {0xD77485CB25823AC7, 0x7D633293366B828C}, // -212
        {0x86A8D39EF77164BC, 0xAE5DFF9C02033198}, // -211
        {0xA8530886B54DBDEB, 0xD9F57F830283FDFD}, // -210
        {0xD267CAA862A12D66, 0xD072DF63C324FD7C}, // -209
        {0x8380DEA93DA4BC60, 0x4247CB9E59F71E6E}, // -208
        {0xA46116538D0DEB78, 0x52D9BE85F074E609}, // -207
        {0xCD795BE870516656, 0x67902E276C921F8C}, // -206
        {0x806BD9714632DFF6, 0x00BA1CD8A3DB53B7}, // -205
        {0xA086CFCD97BF97F3, 0x80E8A40ECCD228A5}, // -204
        {0xC8A883C0FDAF7DF0, 0x6122CD128006B2CE}, // -203
        {0xFAD2A4B13D1B5D6C, 0x796B805720085F82}, // -202
        {0x9CC3A6EEC6311A63, 0xCBE3303674053BB1}, // -201
        {0xC3F490AA77BD60FC, 0xBEDBFC4411068A9D}, // -200
        {0xF4F1B4D515ACB93B, 0xEE92FB5515482D45}, // -199
        {0x991711052D8BF3C5, 0x751BDD152D4D1C4B}, // -198
        {0xBF5CD54678EEF0B6, 0xD262D45A78A0635E}, // -197
        {0xEF340A98172AACE4, 0x86FB897116C87C35}, // -196
        {0x9580869F0E7AAC0E, 0xD45D35E6AE3D4DA1}, // -195
        {0xBAE0A846D2195712, 0x8974836059CCA10A}, // -194
        {0xE998D258869FACD7, 0x2BD1A438703FC94C}, // -193
        {0x91FF83775423CC06, 0x7B6306A34627DDD0}, // -192
        {0xB67F6455292CBF08, 0x1A3BC84C17B1D543}, // -191
        {0xE41F3D6A7377EECA, 0x20CABA5F1D9E4A94}, // -190
        {0x8E938662882AF53E, 0x547EB47B7282EE9D}, // -189
        {0xB23867FB2A35B28D, 0xE99E619A4F23AA44}, // -188
        {0xDEC681F9F4C31F31, 0x6405FA00E2EC94D5}, // -187
        {0x8B3C113C38F9F37E, 0xDE83BC408DD3DD05}, // -186
        {0xAE0B158B4738705E, 0x9624AB50B148D446}, // -185
        {0xD98DDAEE19068C76, 0x3BADD624DD9B0958}, // -184
        {0x87F8A8D4CFA417C9, 0xE54CA5D70A80E5D7}, // -183
        {0xA9F6D30A038D1DBC, 0x5E9FCF4CCD211F4D}, // -182
        {0xD47487CC8470652B, 0x7647C32000696720}, // -181
        {0x84C8D4DFD2C63F3B, 0x29ECD9F40041E074}, // -180
        {0xA5FB0A17C777CF09, 0xF468107100525891}, // -179
        {0xCF79CC9DB955C2CC, 0x7182148D4066EEB5}, // -178
        {0x81AC1FE293D599BF, 0xC6F14CD848405531}, // -177
        {0xA21727DB38CB002F, 0xB8ADA00E5A506A7D}, // -176
        {0xCA9CF1D206FDC03B, 0xA6D90811F0E4851D}, // -175
        {0xFD442E4688BD304A, 0x908F4A166D1DA664}, // -174
        {0x9E4A9CEC15763E2E, 0x9A598E4E043287FF}, // -173
        {0xC5DD44271AD3CDBA, 0x40EFF1E1853F29FE}, // -172
        {0xF7549530E188C128, 0xD12BEE59E68EF47D}, // -171
        {0x9A94DD3E8CF578B9, 0x82BB74F8301958CF}, // -170
        {0xC13A148E3032D6E7, 0xE36A52363C1FAF02}, // -169
        {0xF18899B1BC3F8CA1, 0xDC44E6C3CB279AC2}, // -168
        {0x96F5600F15A7B7E5, 0x29AB103A5EF8C0BA}, // -167
        {0xBCB2B812DB11A5DE, 0x7415D448F6B6F0E8}, // -166
        {0xEBDF661791D60F56, 0x111B495B3464AD22}, // -165
        {0x936B9FCEBB25C995, 0xCAB10DD900BEEC35}, // -164
        {0xB84687C269EF3BFB, 0x3D5D514F40EEA743}, // -163
        {0xE65829B3046B0AFA, 0x0CB4A5A3112A5113}, // -162
        {0x8FF71A0FE2C2E6DC, 0x47F0E785EABA72AC}, // -161
        {0xB3F4E093DB73A093, 0x59ED216765690F57}, // -160
        {0xE0F218B8D25088B8, 0x306869C13EC3532D}, // -159
        {0x8C974F7383725573, 0x1E414218C73A13FC}, // -158
        {0xAFBD2350644EEACF, 0xE5D1929EF90898FB}, // -157
        {0xDBAC6C247D62A583, 0xDF45F746B74ABF3A}, // -156
        {0x894BC396CE5DA772, 0x6B8BBA8C328EB784}, // -155
        {0xAB9EB47C81F5114F, 0x066EA92F3F326565}, // -154
        {0xD686619BA27255A2, 0xC80A537B0EFEFEBE}, // -153
        {0x8613FD0145877585, 0xBD06742CE95F5F37}, // -152
        {0xA798FC4196E952E7, 0x2C48113823B73705}, // -151
        {0xD17F3B51FCA3A7A0, 0xF75A15862CA504C6}, // -150
        {0x82EF85133DE648C4, 0x9A984D73DBE722FC}, // -149
        {0xA3AB66580D5FDAF5, 0xC13E60D0D2E0EBBB}, // -148
        {0xCC963FEE10B7D1B3, 0x318DF905079926A9}, // -147
        {0xFFBBCFE994E5C61F, 0xFDF17746497F7053}, // -146
        {0x9FD561F1FD0F9BD3, 0xFEB6EA8BEDEFA634}, // -145
        {0xC7CABA6E7C5382C8, 0xFE64A52EE96B8FC1}, // -144
        {0xF9BD690A1B68637B, 0x3DFDCE7AA3C673B1}, // -143
        {0x9C1661A651213E2D, 0x06BEA10CA65C084F}, // -142
        {0xC31BFA0FE5698DB8, 0x486E494FCFF30A63}, // -141
        {0xF3E2F893DEC3F126, 0x5A89DBA3C3EFCCFB}, // -140
        {0x986DDB5C6B3A76B7, 0xF89629465A75E01D}, // -139
        {0xBE89523386091465, 0xF6BBB397F1135824}, // -138
        {0xEE2BA6C0678B597F, 0x746AA07DED582E2D}, // -137
        {0x94DB483840B717EF, 0xA8C2A44EB4571CDD}, // -136
        {0xBA121A4650E4DDEB, 0x92F34D62616CE414}, // -135
        {0xE896A0D7E51E1566, 0x77B020BAF9C81D18}, // -134
        {0x915E2486EF32CD60, 0x0ACE1474DC1D122F}, // -133
        {0xB5B5ADA8AAFF80B8, 0x0D819992132456BB}, // -132
        {0xE3231912D5BF60E6, 0x10E1FFF697ED6C6A}, // -131
        {0x8DF5EFABC5979C8F, 0xCA8D3FFA1EF463C2}, // -130
        {0xB1736B96B6FD83B3, 0xBD308FF8A6B17CB3}, // -129
        {0xDDD0467C64BCE4A0, 0xAC7CB3F6D05DDBDF}, // -128
        {0x8AA22C0DBEF60EE4, 0x6BCDF07A423AA96C}, // -127
        {0xAD4AB7112EB3929D, 0x86C16C98D2C953C7}, // -126
        {0xD89D64D57A607744, 0xE871C7BF077BA8B8}, // -125
        {0x87625F056C7C4A8B, 0x11471CD764AD4973}, // -124
        {0xA93AF6C6C79B5D2D, 0xD598E40D3DD89BD0}, // -123
        {0xD389B47879823479, 0x4AFF1D108D4EC2C4}, // -122
        {0x843610CB4BF160CB, 0xCEDF722A585139BB}, // -121
        {0xA54394FE1EEDB8FE, 0xC2974EB4EE658829}, // -120
        {0xCE947A3DA6A9273E, 0x733D226229FEEA33}, // -119
        {0x811CCC668829B887, 0x0806357D5A3F5260}, // -118
        {0xA163FF802A3426A8, 0xCA07C2DCB0CF26F8}, // -117
        {0xC9BCFF6034C13052, 0xFC89B393DD02F0B6}, // -116
        {0xFC2C3F3841F17C67, 0xBBAC2078D443ACE3}, // -115
        {0x9D9BA7832936EDC0, 0xD54B944B84AA4C0E}, // -114
        {0xC5029163F384A931, 0x0A9E795E65D4DF12}, // -113
        {0xF64335BCF065D37D, 0x4D4617B5FF4A16D6}, // -112
        {0x99EA0196163FA42E, 0x504BCED1BF8E4E46}, // -111
        {0xC06481FB9BCF8D39, 0xE45EC2862F71E1D7}, // -110
        {0xF07DA27A82C37088, 0x5D767327BB4E5A4D}, // -109
        {0x964E858C91BA2655, 0x3A6A07F8D510F870}, // -108
        {0xBBE226EFB628AFEA, 0x890489F70A55368C}, // -107
        {0xEADAB0ABA3B2DBE5, 0x2B45AC74CCEA842F}, // -106
        {0x92C8AE6B464FC96F, 0x3B0B8BC90012929E}, // -105
        {0xB77ADA0617E3BBCB, 0x09CE6EBB40173745}, // -104
        {0xE55990879DDCAABD, 0xCC420A6A101D0516}, // -103
        {0x8F57FA54C2A9EAB6, 0x9FA946824A12232E}, // -102
        {0xB32DF8E9F3546564, 0x47939822DC96ABFA}, // -101
        {0xDFF9772470297EBD, 0x59787E2B93BC56F8}, // -100
        {0x8BFBEA76C619EF36, 0x57EB4EDB3C55B65B}, //  -99
        {0xAEFAE51477A06B03, 0xEDE622920B6B23F2}, //  -98
        {0xDAB99E59958885C4, 0xE95FAB368E45ECEE}, //  -97
        {0x88B402F7FD75539B, 0x11DBCB0218EBB415}, //  -96
        {0xAAE103B5FCD2A881, 0xD652BDC29F26A11A}, //  -95
        {0xD59944A37C0752A2, 0x4BE76D3346F04960}, //  -94
        {0x857FCAE62D8493A5, 0x6F70A4400C562DDC}, //  -93
        {0xA6DFBD9FB8E5B88E, 0xCB4CCD500F6BB953}, //  -92
        {0xD097AD07A71F26B2, 0x7E2000A41346A7A8}, //  -91
        {0x825ECC24C873782F, 0x8ED400668C0C28C9}, //  -90
        {0xA2F67F2DFA90563B, 0x728900802F0F32FB}, //  -89
        {0xCBB41EF979346BCA, 0x4F2B40A03AD2FFBA}, //  -88
        {0xFEA126B7D78186BC, 0xE2F610C84987BFA9}, //  -87
        {0x9F24B832E6B0F436, 0x0DD9CA7D2DF4D7CA}, //  -86
        {0xC6EDE63FA05D3143, 0x91503D1C79720DBC}, //  -85
        {0xF8A95FCF88747D94, 0x75A44C6397CE912B}, //  -84
        {0x9B69DBE1B548CE7C, 0xC986AFBE3EE11ABB}, //  -83
        {0xC24452DA229B021B, 0xFBE85BADCE996169}, //  -82
        {0xF2D56790AB41C2A2, 0xFAE27299423FB9C4}, //  -81
        {0x97C560BA6B0919A5, 0xDCCD879FC967D41B}, //  -80
        {0xBDB6B8E905CB600F, 0x5400E987BBC1C921}, //  -79
        {0xED246723473E3813, 0x290123E9AAB23B69}, //  -78
        {0x9436C0760C86E30B, 0xF9A0B6720AAF6522}, //  -77
        {0xB94470938FA89BCE, 0xF808E40E8D5B3E6A}, //  -76
        {0xE7958CB87392C2C2, 0xB60B1D1230B20E05}, //  -75
        {0x90BD77F3483BB9B9, 0xB1C6F22B5E6F48C3}, //  -74
        {0xB4ECD5F01A4AA828, 0x1E38AEB6360B1AF4}, //  -73
        {0xE2280B6C20DD5232, 0x25C6DA63C38DE1B1}, //  -72
        {0x8D590723948A535F, 0x579C487E5A38AD0F}, //  -71
        {0xB0AF48EC79ACE837, 0x2D835A9DF0C6D852}, //  -70
        {0xDCDB1B2798182244, 0xF8E431456CF88E66}, //  -69
        {0x8A08F0F8BF0F156B, 0x1B8E9ECB641B5900}, //  -68
        {0xAC8B2D36EED2DAC5, 0xE272467E3D222F40}, //  -67
        {0xD7ADF884AA879177, 0x5B0ED81DCC6ABB10}, //  -66
        {0x86CCBB52EA94BAEA, 0x98E947129FC2B4EA}, //  -65
        {0xA87FEA27A539E9A5, 0x3F2398D747B36225}, //  -64
        {0xD29FE4B18E88640E, 0x8EEC7F0D19A03AAE}, //  -63
        {0x83A3EEEEF9153E89, 0x1953CF68300424AD}, //  -62
        {0xA48CEAAAB75A8E2B, 0x5FA8C3423C052DD8}, //  -61
        {0xCDB02555653131B6, 0x3792F412CB06794E}, //  -60
        {0x808E17555F3EBF11, 0xE2BBD88BBEE40BD1}, //  -59
        {0xA0B19D2AB70E6ED6, 0x5B6ACEAEAE9D0EC5}, //  -58
        {0xC8DE047564D20A8B, 0xF245825A5A445276}, //  -57
        {0xFB158592BE068D2E, 0xEED6E2F0F0D56713}, //  -56
        {0x9CED737BB6C4183D, 0x55464DD69685606C}, //  -55
        {0xC428D05AA4751E4C, 0xAA97E14C3C26B887}, //  -54
        {0xF53304714D9265DF, 0xD53DD99F4B3066A9}, //  -53
        {0x993FE2C6D07B7FAB, 0xE546A8038EFE402A}, //  -52
        {0xBF8FDB78849A5F96, 0xDE98520472BDD034}, //  -51
        {0xEF73D256A5C0F77C, 0x963E66858F6D4441}, //  -50
        {0x95A8637627989AAD, 0xDDE7001379A44AA9}, //  -49
        {0xBB127C53B17EC159, 0x5560C018580D5D53}, //  -48
        {0xE9D71B689DDE71AF, 0xAAB8F01E6E10B4A7}, //  -47
        {0x9226712162AB070D, 0xCAB3961304CA70E9}, //  -46
        {0xB6B00D69BB55C8D1, 0x3D607B97C5FD0D23}, //  -45
        {0xE45C10C42A2B3B05, 0x8CB89A7DB77C506B}, //  -44
        {0x8EB98A7A9A5B04E3, 0x77F3608E92ADB243}, //  -43
        {0xB267ED1940F1C61C, 0x55F038B237591ED4}, //  -42
        {0xDF01E85F912E37A3, 0x6B6C46DEC52F6689}, //  -41
        {0x8B61313BBABCE2C6, 0x2323AC4B3B3DA016}, //  -40
        {0xAE397D8AA96C1B77, 0xABEC975E0A0D081B}, //  -39
        {0xD9C7DCED53C72255, 0x96E7BD358C904A22}, //  -38
        {0x881CEA14545C7575, 0x7E50D64177DA2E55}, //  -37
        {0xAA242499697392D2, 0xDDE50BD1D5D0B9EA}, //  -36
        {0xD4AD2DBFC3D07787, 0x955E4EC64B44E865}, //  -35
        {0x84EC3C97DA624AB4, 0xBD5AF13BEF0B113F}, //  -34
        {0xA6274BBDD0FADD61, 0xECB1AD8AEACDD58F}, //  -33
        {0xCFB11EAD453994BA, 0x67DE18EDA5814AF3}, //  -32
        {0x81CEB32C4B43FCF4, 0x80EACF948770CED8}, //  -31
        {0xA2425FF75E14FC31, 0xA1258379A94D028E}, //  -30
        {0xCAD2F7F5359A3B3E, 0x096EE45813A04331}, //  -29
        {0xFD87B5F28300CA0D, 0x8BCA9D6E188853FD}, //  -28
        {0x9E74D1B791E07E48, 0x775EA264CF55347E}, //  -27
        {0xC612062576589DDA, 0x95364AFE032A819E}, //  -26
        {0xF79687AED3EEC551, 0x3A83DDBD83F52205}, //  -25
        {0x9ABE14CD44753B52, 0xC4926A9672793543}, //  -24
        {0xC16D9A0095928A27, 0x75B7053C0F178294}, //  -23
        {0xF1C90080BAF72CB1, 0x5324C68B12DD6339}, //  -22
        {0x971DA05074DA7BEE, 0xD3F6FC16EBCA5E04}, //  -21
        {0xBCE5086492111AEA, 0x88F4BB1CA6BCF585}, //  -20
        {0xEC1E4A7DB69561A5, 0x2B31E9E3D06C32E6}, //  -19
        {0x9392EE8E921D5D07, 0x3AFF322E62439FD0}, //  -18
        {0xB877AA3236A4B449, 0x09BEFEB9FAD487C3}, //  -17
        {0xE69594BEC44DE15B, 0x4C2EBE687989A9B4}, //  -16
        {0x901D7CF73AB0ACD9, 0x0F9D37014BF60A11}, //  -15
        {0xB424DC35095CD80F, 0x538484C19EF38C95}, //  -14
        {0xE12E13424BB40E13, 0x2865A5F206B06FBA}, //  -13
        {0x8CBCCC096F5088CB, 0xF93F87B7442E45D4}, //  -12
        {0xAFEBFF0BCB24AAFE, 0xF78F69A51539D749}, //  -11
        {0xDBE6FECEBDEDD5BE, 0xB573440E5A884D1C}, //  -10
        {0x89705F4136B4A597, 0x31680A88F8953031}, //   -9
        {0xABCC77118461CEFC, 0xFDC20D2B36BA7C3E}, //   -8
        {0xD6BF94D5E57A42BC, 0x3D32907604691B4D}, //   -7
        {0x8637BD05AF6C69B5, 0xA63F9A49C2C1B110}, //   -6
        {0xA7C5AC471B478423, 0x0FCF80DC33721D54}, //   -5
        {0xD1B71758E219652B, 0xD3C36113404EA4A9}, //   -4
        {0x83126E978D4FDF3B, 0x645A1CAC083126EA}, //   -3
        {0xA3D70A3D70A3D70A, 0x3D70A3D70A3D70A4}, //   -2
        {0xCCCCCCCCCCCCCCCC, 0xCCCCCCCCCCCCCCCD}, //   -1
        {0x8000000000000000, 0x0000000000000000}, //    0
        {0xA000000000000000, 0x0000000000000000}, //    1
        {0xC800000000000000, 0x0000000000000000}, //    2
        {0xFA00000000000000, 0x0000000000000000}, //    3
        {0x9C40000000000000, 0x0000000000000000}, //    4
        {0xC350000000000000, 0x0000000000000000}, //    5
        {0xF424000000000000, 0x0000000000000000}, //    6
        {0x9896800000000000, 0x0000000000000000}, //    7
        {0xBEBC200000000000, 0x0000000000000000}, //    8
        {0xEE6B280000000000, 0x0000000000000000}, //    9
        {0x9502F90000000000, 0x0000000000000000}, //   10
        {0xBA43B74000000000, 0x0000000000000000}, //   11
        {0xE8D4A51000000000, 0x0000000000000000}, //   12
        {0x9184E72A00000000, 0x0000000000000000}, //   13
        {0xB5E620F480000000, 0x0000000000000000}, //   14
        {0xE35FA931A0000000, 0x0000000000000000}, //   15
        {0x8E1BC9BF04000000, 0x0000000000000000}, //   16
        {0xB1A2BC2EC5000000, 0x0000000000000000}, //   17
        {0xDE0B6B3A76400000, 0x0000000000000000}, //   18
        {0x8AC7230489E80000, 0x0000000000000000}, //   19
        {0xAD78EBC5AC620000, 0x0000000000000000}, //   20
        {0xD8D726B7177A8000, 0x0000000000000000}, //   21
        {0x878678326EAC9000, 0x0000000000000000}, //   22
        {0xA968163F0A57B400, 0x0000000000000000}, //   23
        {0xD3C21BCECCEDA100, 0x0000000000000000}, //   24
        {0x84595161401484A0, 0x0000000000000000}, //   25
        {0xA56FA5B99019A5C8, 0x0000000000000000}, //   26
        {0xCECB8F27F4200F3A, 0x0000000000000000}, //   27
        {0x813F3978F8940984, 0x4000000000000000}, //   28
        {0xA18F07D736B90BE5, 0x5000000000000000}, //   29
        {0xC9F2C9CD04674EDE, 0xA400000000000000}, //   30
        {0xFC6F7C4045812296, 0x4D00000000000000}, //   31
        {0x9DC5ADA82B70B59D, 0xF020000000000000}, //   32
        {0xC5371912364CE305, 0x6C28000000000000}, //   33
        {0xF684DF56C3E01BC6, 0xC732000000000000}, //   34
        {0x9A130B963A6C115C, 0x3C7F400000000000}, //   35
        {0xC097CE7BC90715B3, 0x4B9F100000000000}, //   36
        {0xF0BDC21ABB48DB20, 0x1E86D40000000000}, //   37
        {0x96769950B50D88F4, 0x1314448000000000}, //   38
        {0xBC143FA4E250EB31, 0x17D955A000000000}, //   39
        {0xEB194F8E1AE525FD, 0x5DCFAB0800000000}, //   40
        {0x92EFD1B8D0CF37BE, 0x5AA1CAE500000000}, //   41
        {0xB7ABC627050305AD, 0xF14A3D9E40000000}, //   42
        {0xE596B7B0C643C719, 0x6D9CCD05D0000000}, //   43
        {0x8F7E32CE7BEA5C6F, 0xE4820023A2000000}, //   44
        {0xB35DBF821AE4F38B, 0xDDA2802C8A800000}, //   45
        {0xE0352F62A19E306E, 0xD50B2037AD200000}, //   46
        {0x8C213D9DA502DE45, 0x4526F422CC340000}, //   47
        {0xAF298D050E4395D6, 0x9670B12B7F410000}, //   48
        {0xDAF3F04651D47B4C, 0x3C0CDD765F114000}, //   49
        {0x88D8762BF324CD0F, 0xA5880A69FB6AC800}, //   50
        {0xAB0E93B6EFEE0053, 0x8EEA0D047A457A00}, //   51
        {0xD5D238A4ABE98068, 0x72A4904598D6D880}, //   52
        {0x85A36366EB71F041, 0x47A6DA2B7F864750}, //   53
        {0xA70C3C40A64E6C51, 0x999090B65F67D924}, //   54
        {0xD0CF4B50CFE20765, 0xFFF4B4E3F741CF6D}, //   55
        {0x82818F1281ED449F, 0xBFF8F10E7A8921A5}, //   56
        {0xA321F2D7226895C7, 0xAFF72D52192B6A0E}, //   57
        {0xCBEA6F8CEB02BB39, 0x9BF4F8A69F764491}, //   58
        {0xFEE50B7025C36A08, 0x02F236D04753D5B5}, //   59
        {0x9F4F2726179A2245, 0x01D762422C946591}, //   60
        {0xC722F0EF9D80AAD6, 0x424D3AD2B7B97EF6}, //   61
        {0xF8EBAD2B84E0D58B, 0xD2E0898765A7DEB3}, //   62
        {0x9B934C3B330C8577, 0x63CC55F49F88EB30}, //   63
        {0xC2781F49FFCFA6D5, 0x3CBF6B71C76B25FC}, //   64
        {0xF316271C7FC3908A, 0x8BEF464E3945EF7B}, //   65
        {0x97EDD871CFDA3A56, 0x97758BF0E3CBB5AD}, //   66
        {0xBDE94E8E43D0C8EC, 0x3D52EEED1CBEA318}, //   67
        {0xED63A231D4C4FB27, 0x4CA7AAA863EE4BDE}, //   68
        {0x945E455F24FB1CF8, 0x8FE8CAA93E74EF6B}, //   69
        {0xB975D6B6EE39E436, 0xB3E2FD538E122B45}, //   70
        {0xE7D34C64A9C85D44, 0x60DBBCA87196B617}, //   71
        {0x90E40FBEEA1D3A4A, 0xBC8955E946FE31CE}, //   72
        {0xB51D13AEA4A488DD, 0x6BABAB6398BDBE42}, //   73
        {0xE264589A4DCDAB14, 0xC696963C7EED2DD2}, //   74
        {0x8D7EB76070A08AEC, 0xFC1E1DE5CF543CA3}, //   75
        {0xB0DE65388CC8ADA8, 0x3B25A55F43294BCC}, //   76
        {0xDD15FE86AFFAD912, 0x49EF0EB713F39EBF}, //   77
        {0x8A2DBF142DFCC7AB, 0x6E3569326C784338}, //   78
        {0xACB92ED9397BF996, 0x49C2C37F07965405}, //   79
        {0xD7E77A8F87DAF7FB, 0xDC33745EC97BE907}, //   80
        {0x86F0AC99B4E8DAFD, 0x69A028BB3DED71A4}, //   81
        {0xA8ACD7C0222311BC, 0xC40832EA0D68CE0D}, //   82
        {0xD2D80DB02AABD62B, 0xF50A3FA490C30191}, //   83
        {0x83C7088E1AAB65DB, 0x792667C6DA79E0FB}, //   84
        {0xA4B8CAB1A1563F52, 0x577001B891185939}, //   85
        {0xCDE6FD5E09ABCF26, 0xED4C0226B55E6F87}, //   86
        {0x80B05E5AC60B6178, 0x544F8158315B05B5}, //   87
        {0xA0DC75F1778E39D6, 0x696361AE3DB1C722}, //   88
        {0xC913936DD571C84C, 0x03BC3A19CD1E38EA}, //   89
        {0xFB5878494ACE3A5F, 0x04AB48A04065C724}, //   90
        {0x9D174B2DCEC0E47B, 0x62EB0D64283F9C77}, //   91
        {0xC45D1DF942711D9A, 0x3BA5D0BD324F8395}, //   92
        {0xF5746577930D6500, 0xCA8F44EC7EE3647A}, //   93
        {0x9968BF6ABBE85F20, 0x7E998B13CF4E1ECC}, //   94
        {0xBFC2EF456AE276E8, 0x9E3FEDD8C321A67F}, //   95
        {0xEFB3AB16C59B14A2, 0xC5CFE94EF3EA101F}, //   96
        {0x95D04AEE3B80ECE5, 0xBBA1F1D158724A13}, //   97
        {0xBB445DA9CA61281F, 0x2A8A6E45AE8EDC98}, //   98
        {0xEA1575143CF97226, 0xF52D09D71A3293BE}, //   99
        {0x924D692CA61BE758, 0x593C2626705F9C57}, //  100
        {0xB6E0C377CFA2E12E, 0x6F8B2FB00C77836D}, //  101
        {0xE498F455C38B997A, 0x0B6DFB9C0F956448}, //  102
        {0x8EDF98B59A373FEC, 0x4724BD4189BD5EAD}, //  103
        {0xB2977EE300C50FE7, 0x58EDEC91EC2CB658}, //  104
        {0xDF3D5E9BC0F653E1, 0x2F2967B66737E3EE}, //  105
        {0x8B865B215899F46C, 0xBD79E0D20082EE75}, //  106
        {0xAE67F1E9AEC07187, 0xECD8590680A3AA12}, //  107
        {0xDA01EE641A708DE9, 0xE80E6F4820CC9496}, //  108
        {0x884134FE908658B2, 0x3109058D147FDCDE}, //  109
        {0xAA51823E34A7EEDE, 0xBD4B46F0599FD416}, //  110
        {0xD4E5E2CDC1D1EA96, 0x6C9E18AC7007C91B}, //  111
        {0x850FADC09923329E, 0x03E2CF6BC604DDB1}, //  112
        {0xA6539930BF6BFF45, 0x84DB8346B786151D}, //  113
        {0xCFE87F7CEF46FF16, 0xE612641865679A64}, //  114
        {0x81F14FAE158C5F6E, 0x4FCB7E8F3F60C07F}, //  115
        {0xA26DA3999AEF7749, 0xE3BE5E330F38F09E}, //  116
        {0xCB090C8001AB551C, 0x5CADF5BFD3072CC6}, //  117
        {0xFDCB4FA002162A63, 0x73D9732FC7C8F7F7}, //  118
        {0x9E9F11C4014DDA7E, 0x2867E7FDDCDD9AFB}, //  119
        {0xC646D63501A1511D, 0xB281E1FD541501B9}, //  120
        {0xF7D88BC24209A565, 0x1F225A7CA91A4227}, //  121
        {0x9AE757596946075F, 0x3375788DE9B06959}, //  122
        {0xC1A12D2FC3978937, 0x0052D6B1641C83AF}, //  123
        {0xF209787BB47D6B84, 0xC0678C5DBD23A49B}, //  124
        {0x9745EB4D50CE6332, 0xF840B7BA963646E1}, //  125
        {0xBD176620A501FBFF, 0xB650E5A93BC3D899}, //  126
        {0xEC5D3FA8CE427AFF, 0xA3E51F138AB4CEBF}, //  127
        {0x93BA47C980E98CDF, 0xC66F336C36B10138}, //  128
        {0xB8A8D9BBE123F017, 0xB80B0047445D4185}, //  129
        {0xE6D3102AD96CEC1D, 0xA60DC059157491E6}, //  130
        {0x9043EA1AC7E41392, 0x87C89837AD68DB30}, //  131
        {0xB454E4A179DD1877, 0x29BABE4598C311FC}, //  132
        {0xE16A1DC9D8545E94, 0xF4296DD6FEF3D67B}, //  133
        {0x8CE2529E2734BB1D, 0x1899E4A65F58660D}, //  134
        {0xB01AE745B101E9E4, 0x5EC05DCFF72E7F90}, //  135
        {0xDC21A1171D42645D, 0x76707543F4FA1F74}, //  136
        {0x899504AE72497EBA, 0x6A06494A791C53A9}, //  137
        {0xABFA45DA0EDBDE69, 0x0487DB9D17636893}, //  138
        {0xD6F8D7509292D603, 0x45A9D2845D3C42B7}, //  139
        {0x865B86925B9BC5C2, 0x0B8A2392BA45A9B3}, //  140
        {0xA7F26836F282B732, 0x8E6CAC7768D7141F}, //  141
        {0xD1EF0244AF2364FF, 0x3207D795430CD927}, //  142
        {0x8335616AED761F1F, 0x7F44E6BD49E807B9}, //  143
        {0xA402B9C5A8D3A6E7, 0x5F16206C9C6209A7}, //  144
        {0xCD036837130890A1, 0x36DBA887C37A8C10}, //  145
        {0x802221226BE55A64, 0xC2494954DA2C978A}, //  146
        {0xA02AA96B06DEB0FD, 0xF2DB9BAA10B7BD6D}, //  147
        {0xC83553C5C8965D3D, 0x6F92829494E5ACC8}, //  148
        {0xFA42A8B73ABBF48C, 0xCB772339BA1F17FA}, //  149
        {0x9C69A97284B578D7, 0xFF2A760414536EFC}, //  150
        {0xC38413CF25E2D70D, 0xFEF5138519684ABB}, //  151
        {0xF46518C2EF5B8CD1, 0x7EB258665FC25D6A}, //  152
        {0x98BF2F79D5993802, 0xEF2F773FFBD97A62}, //  153
        {0xBEEEFB584AFF8603, 0xAAFB550FFACFD8FB}, //  154
        {0xEEAABA2E5DBF6784, 0x95BA2A53F983CF39}, //  155
        {0x952AB45CFA97A0B2, 0xDD945A747BF26184}, //  156
        {0xBA756174393D88DF, 0x94F971119AEEF9E5}, //  157
        {0xE912B9D1478CEB17, 0x7A37CD5601AAB85E}, //  158
        {0x91ABB422CCB812EE, 0xAC62E055C10AB33B}, //  159
        {0xB616A12B7FE617AA, 0x577B986B314D600A}, //  160
        {0xE39C49765FDF9D94, 0xED5A7E85FDA0B80C}, //  161
        {0x8E41ADE9FBEBC27D, 0x14588F13BE847308}, //  162
        {0xB1D219647AE6B31C, 0x596EB2D8AE258FC9}, //  163
        {0xDE469FBD99A05FE3, 0x6FCA5F8ED9AEF3BC}, //  164
        {0x8AEC23D680043BEE, 0x25DE7BB9480D5855}, //  165
        {0xADA72CCC20054AE9, 0xAF561AA79A10AE6B}, //  166
        {0xD910F7FF28069DA4, 0x1B2BA1518094DA05}, //  167
        {0x87AA9AFF79042286, 0x90FB44D2F05D0843}, //  168
        {0xA99541BF57452B28, 0x353A1607AC744A54}, //  169
        {0xD3FA922F2D1675F2, 0x42889B8997915CE9}, //  170
        {0x847C9B5D7C2E09B7, 0x69956135FEBADA12}, //  171
        {0xA59BC234DB398C25, 0x43FAB9837E699096}, //  172
        {0xCF02B2C21207EF2E, 0x94F967E45E03F4BC}, //  173
        {0x8161AFB94B44F57D, 0x1D1BE0EEBAC278F6}, //  174
        {0xA1BA1BA79E1632DC, 0x6462D92A69731733}, //  175
        {0xCA28A291859BBF93, 0x7D7B8F7503CFDCFF}, //  176
        {0xFCB2CB35E702AF78, 0x5CDA735244C3D43F}, //  177
        {0x9DEFBF01B061ADAB, 0x3A0888136AFA64A8}, //  178
        {0xC56BAEC21C7A1916, 0x088AAA1845B8FDD1}, //  179
        {0xF6C69A72A3989F5B, 0x8AAD549E57273D46}, //  180
        {0x9A3C2087A63F6399, 0x36AC54E2F678864C}, //  181
        {0xC0CB28A98FCF3C7F, 0x84576A1BB416A7DE}, //  182
        {0xF0FDF2D3F3C30B9F, 0x656D44A2A11C51D6}, //  183
        {0x969EB7C47859E743, 0x9F644AE5A4B1B326}, //  184
        {0xBC4665B596706114, 0x873D5D9F0DDE1FEF}, //  185
        {0xEB57FF22FC0C7959, 0xA90CB506D155A7EB}, //  186
        {0x9316FF75DD87CBD8, 0x09A7F12442D588F3}, //  187
        {0xB7DCBF5354E9BECE, 0x0C11ED6D538AEB30}, //  188
        {0xE5D3EF282A242E81, 0x8F1668C8A86DA5FB}, //  189
        {0x8FA475791A569D10, 0xF96E017D694487BD}, //  190
        {0xB38D92D760EC4455, 0x37C981DCC395A9AD}, //  191
        {0xE070F78D3927556A, 0x85BBE253F47B1418}, //  192
        {0x8C469AB843B89562, 0x93956D7478CCEC8F}, //  193
        {0xAF58416654A6BABB, 0x387AC8D1970027B3}, //  194
        {0xDB2E51BFE9D0696A, 0x06997B05FCC0319F}, //  195
        {0x88FCF317F22241E2, 0x441FECE3BDF81F04}, //  196
        {0xAB3C2FDDEEAAD25A, 0xD527E81CAD7626C4}, //  197
        {0xD60B3BD56A5586F1, 0x8A71E223D8D3B075}, //  198
        {0x85C7056562757456, 0xF6872D5667844E4A}, //  199
        {0xA738C6BEBB12D16C, 0xB428F8AC016561DC}, //  200
        {0xD106F86E69D785C7, 0xE13336D701BEBA53}, //  201
        {0x82A45B450226B39C, 0xECC0024661173474}, //  202
        {0xA34D721642B06084, 0x27F002D7F95D0191}, //  203
        {0xCC20CE9BD35C78A5, 0x31EC038DF7B441F5}, //  204
        {0xFF290242C83396CE, 0x7E67047175A15272}, //  205
        {0x9F79A169BD203E41, 0x0F0062C6E984D387}, //  206
        {0xC75809C42C684DD1, 0x52C07B78A3E60869}, //  207
        {0xF92E0C3537826145, 0xA7709A56CCDF8A83}, //  208
        {0x9BBCC7A142B17CCB, 0x88A66076400BB692}, //  209
        {0xC2ABF989935DDBFE, 0x6ACFF893D00EA436}, //  210
        {0xF356F7EBF83552FE, 0x0583F6B8C4124D44}, //  211
        {0x98165AF37B2153DE, 0xC3727A337A8B704B}, //  212
        {0xBE1BF1B059E9A8D6, 0x744F18C0592E4C5D}, //  213
        {0xEDA2EE1C7064130C, 0x1162DEF06F79DF74}, //  214
        {0x9485D4D1C63E8BE7, 0x8ADDCB5645AC2BA9}, //  215
        {0xB9A74A0637CE2EE1, 0x6D953E2BD7173693}, //  216
        {0xE8111C87C5C1BA99, 0xC8FA8DB6CCDD0438}, //  217
        {0x910AB1D4DB9914A0, 0x1D9C9892400A22A3}, //  218
        {0xB54D5E4A127F59C8, 0x2503BEB6D00CAB4C}, //  219
        {0xE2A0B5DC971F303A, 0x2E44AE64840FD61E}, //  220
        {0x8DA471A9DE737E24, 0x5CEAECFED289E5D3}, //  221
        {0xB10D8E1456105DAD, 0x7425A83E872C5F48}, //  222
        {0xDD50F1996B947518, 0xD12F124E28F7771A}, //  223
        {0x8A5296FFE33CC92F, 0x82BD6B70D99AAA70}, //  224
        {0xACE73CBFDC0BFB7B, 0x636CC64D1001550C}, //  225
        {0xD8210BEFD30EFA5A, 0x3C47F7E05401AA4F}, //  226
        {0x8714A775E3E95C78, 0x65ACFAEC34810A72}, //  227
        {0xA8D9D1535CE3B396, 0x7F1839A741A14D0E}, //  228
        {0xD31045A8341CA07C, 0x1EDE48111209A051}, //  229
        {0x83EA2B892091E44D, 0x934AED0AAB460433}, //  230
        {0xA4E4B66B68B65D60, 0xF81DA84D56178540}, //  231
        {0xCE1DE40642E3F4B9, 0x36251260AB9D668F}, //  232
        {0x80D2AE83E9CE78F3, 0xC1D72B7C6B42601A}, //  233
        {0xA1075A24E4421730, 0xB24CF65B8612F820}, //  234
        {0xC94930AE1D529CFC, 0xDEE033F26797B628}, //  235
        {0xFB9B7CD9A4A7443C, 0x169840EF017DA3B2}, //  236
        {0x9D412E0806E88AA5, 0x8E1F289560EE864F}, //  237
        {0xC491798A08A2AD4E, 0xF1A6F2BAB92A27E3}, //  238
        {0xF5B5D7EC8ACB58A2, 0xAE10AF696774B1DC}, //  239
        {0x9991A6F3D6BF1765, 0xACCA6DA1E0A8EF2A}, //  240
        {0xBFF610B0CC6EDD3F, 0x17FD090A58D32AF4}, //  241
        {0xEFF394DCFF8A948E, 0xDDFC4B4CEF07F5B1}, //  242
        {0x95F83D0A1FB69CD9, 0x4ABDAF101564F98F}, //  243
        {0xBB764C4CA7A4440F, 0x9D6D1AD41ABE37F2}, //  244
        {0xEA53DF5FD18D5513, 0x84C86189216DC5EE}, //  245
        {0x92746B9BE2F8552C, 0x32FD3CF5B4E49BB5}, //  246
        {0xB7118682DBB66A77, 0x3FBC8C33221DC2A2}, //  247
        {0xE4D5E82392A40515, 0x0FABAF3FEAA5334B}, //  248
        {0x8F05B1163BA6832D, 0x29CB4D87F2A7400F}, //  249
        {0xB2C71D5BCA9023F8, 0x743E20E9EF511013}, //  250
        {0xDF78E4B2BD342CF6, 0x914DA9246B255417}, //  251
        {0x8BAB8EEFB6409C1A, 0x1AD089B6C2F7548F}, //  252
        {0xAE9672ABA3D0C320, 0xA184AC2473B529B2}, //  253
        {0xDA3C0F568CC4F3E8, 0xC9E5D72D90A2741F}, //  254
        {0x8865899617FB1871, 0x7E2FA67C7A658893}, //  255
        {0xAA7EEBFB9DF9DE8D, 0xDDBB901B98FEEAB8}, //  256
        {0xD51EA6FA85785631, 0x552A74227F3EA566}, //  257
        {0x8533285C936B35DE, 0xD53A88958F872760}, //  258
        {0xA67FF273B8460356, 0x8A892ABAF368F138}, //  259
        {0xD01FEF10A657842C, 0x2D2B7569B0432D86}, //  260
        {0x8213F56A67F6B29B, 0x9C3B29620E29FC74}, //  261
        {0xA298F2C501F45F42, 0x8349F3BA91B47B90}, //  262
        {0xCB3F2F7642717713, 0x241C70A936219A74}, //  263
        {0xFE0EFB53D30DD4D7, 0xED238CD383AA0111}, //  264
        {0x9EC95D1463E8A506, 0xF4363804324A40AB}, //  265
        {0xC67BB4597CE2CE48, 0xB143C6053EDCD0D6}, //  266
        {0xF81AA16FDC1B81DA, 0xDD94B7868E94050B}, //  267
        {0x9B10A4E5E9913128, 0xCA7CF2B4191C8327}, //  268
        {0xC1D4CE1F63F57D72, 0xFD1C2F611F63A3F1}, //  269
        {0xF24A01A73CF2DCCF, 0xBC633B39673C8CED}, //  270
        {0x976E41088617CA01, 0xD5BE0503E085D814}, //  271
        {0xBD49D14AA79DBC82, 0x4B2D8644D8A74E19}, //  272
        {0xEC9C459D51852BA2, 0xDDF8E7D60ED1219F}, //  273
        {0x93E1AB8252F33B45, 0xCABB90E5C942B504}, //  274
        {0xB8DA1662E7B00A17, 0x3D6A751F3B936244}, //  275
        {0xE7109BFBA19C0C9D, 0x0CC512670A783AD5}, //  276
        {0x906A617D450187E2, 0x27FB2B80668B24C6}, //  277
        {0xB484F9DC9641E9DA, 0xB1F9F660802DEDF7}, //  278
        {0xE1A63853BBD26451, 0x5E7873F8A0396974}, //  279
        {0x8D07E33455637EB2, 0xDB0B487B6423E1E9}, //  280
        {0xB049DC016ABC5E5F, 0x91CE1A9A3D2CDA63}, //  281
        {0xDC5C5301C56B75F7, 0x7641A140CC7810FC}, //  282
        {0x89B9B3E11B6329BA, 0xA9E904C87FCB0A9E}, //  283
        {0xAC2820D9623BF429, 0x546345FA9FBDCD45}, //  284
        {0xD732290FBACAF133, 0xA97C177947AD4096}, //  285
        {0x867F59A9D4BED6C0, 0x49ED8EABCCCC485E}, //  286
        {0xA81F301449EE8C70, 0x5C68F256BFFF5A75}, //  287
        {0xD226FC195C6A2F8C, 0x73832EEC6FFF3112}, //  288
        {0x83585D8FD9C25DB7, 0xC831FD53C5FF7EAC}, //  289
        {0xA42E74F3D032F525, 0xBA3E7CA8B77F5E56}, //  290
        {0xCD3A1230C43FB26F, 0x28CE1BD2E55F35EC}, //  291
        {0x80444B5E7AA7CF85, 0x7980D163CF5B81B4}, //  292
        {0xA0555E361951C366, 0xD7E105BCC3326220}, //  293
        {0xC86AB5C39FA63440, 0x8DD9472BF3FEFAA8}, //  294
        {0xFA856334878FC150, 0xB14F98F6F0FEB952}, //  295
        {0x9C935E00D4B9D8D2, 0x6ED1BF9A569F33D4}, //  296
        {0xC3B8358109E84F07, 0x0A862F80EC4700C9}, //  297
        {0xF4A642E14C6262C8, 0xCD27BB612758C0FB}, //  298
        {0x98E7E9CCCFBD7DBD, 0x8038D51CB897789D}, //  299
        {0xBF21E44003ACDD2C, 0xE0470A63E6BD56C4}, //  300
        {0xEEEA5D5004981478, 0x1858CCFCE06CAC75}, //  301
        {0x95527A5202DF0CCB, 0x0F37801E0C43EBC9}, //  302
        {0xBAA718E68396CFFD, 0xD30560258F54E6BB}, //  303
        {0xE950DF20247C83FD, 0x47C6B82EF32A206A}, //  304
        {0x91D28B7416CDD27E, 0x4CDC331D57FA5442}, //  305
        {0xB6472E511C81471D, 0xE0133FE4ADF8E953}, //  306
        {0xE3D8F9E563A198E5, 0x58180FDDD97723A7}, //  307
        {0x8E679C2F5E44FF8F, 0x570F09EAA7EA7649}, //  308
        {0xB201833B35D63F73, 0x2CD2CC6551E513DB}, //  309
        {0xDE81E40A034BCF4F, 0xF8077F7EA65E58D2}, //  310
        {0x8B112E86420F6191, 0xFB04AFAF27FAF783}, //  311
        {0xADD57A27D29339F6, 0x79C5DB9AF1F9B564}, //  312
        {0xD94AD8B1C7380874, 0x18375281AE7822BD}, //  313
        {0x87CEC76F1C830548, 0x8F2293910D0B15B6}, //  314
        {0xA9C2794AE3A3C69A, 0xB2EB3875504DDB23}, //  315
        {0xD433179D9C8CB841, 0x5FA60692A46151EC}, //  316
        {0x849FEEC281D7F328, 0xDBC7C41BA6BCD334}, //  317
        {0xA5C7EA73224DEFF3, 0x12B9B522906C0801}, //  318
        {0xCF39E50FEAE16BEF, 0xD768226B34870A01}, //  319
        {0x81842F29F2CCE375, 0xE6A1158300D46641}, //  320
        {0xA1E53AF46F801C53, 0x60495AE3C1097FD1}, //  321
        {0xCA5E89B18B602368, 0x385BB19CB14BDFC5}, //  322
        {0xFCF62C1DEE382C42, 0x46729E03DD9ED7B6}, //  323
        {0x9E19DB92B4E31BA9, 0x6C07A2C26A8346D2}, //  324
    };

    u64 M52 = (1ull << 52) - 1;
    u64 M63 = (1ull << 63) - 1;
    u64 vi = *(u64 *)&v;
    i64 vi64 = *(i64 *)&v;
    u64 vi_abs = vi & M63;
    double v_abs = *(double *)&vi_abs; // abs(v)
    u64 v_to_u64 = v_abs;              // cvt to unsigned long long
    double u64_to_v = v_to_u64;        // cvt to double
    i64 v_to_i64 = v;                  // cvt to long long
    double i64_to_v = v_to_i64;        // cvt to double
    u64 sign = (vi >> 63);
    i64 exp = vi_abs >> 52;
    u64 frac = vi & M52;
#if 1 // print integer
    // if( (vi64 == *(i64*)&i64_to_v) & ( abs(v_to_i64) <= (i64)( ((1ll<<53) - 1) ) )  ) // branch miss may high in random data
    if ((vi_abs == *(u64 *)&u64_to_v) & (v_to_u64 <= (u64)(((1ull << 53) - 1))))
    {
        // small_int = 1;
        // printf("small integer v_to_i64 = %lld \n", v_to_u64);
#if 1
        buffer[0] = '-';
        // int len = static_cast<int>(itoa_i64_yy((int64_t)v_to_i64,buffer) - buffer);
        int len = static_cast<int>(itoa_u64_impl((uint64_t)v_to_u64, buffer + sign) - buffer);
        buffer[len] = '\0';
        return len;
#else
        // return i64toa_sse2(v_to_i64,buffer);
        buffer[0] = '-';
        return sign + u64toa_sse2(v_to_u64, buffer + sign);
#endif
    }
#endif
    buffer[0] = '-';
    buffer += sign;
    if (exp == 0x7ff) [[unlikely]]
    {
        *(int *)buffer = frac ? *(int *)"nan" : *(int *)"inf";
        return sign + 3;
    }
    // if (vi_abs == 0)
    // {
    //     *(short *)buffer = *(short *)"0";
    //     return sign + 1;
    // }
    u64 normal = (exp > 0);
    u64 c;
    ll q;
    if (normal) // [[likely]]
    {
        c = frac | (1ull << 52);
        q = exp - 1075;
    }
    else
    {
        c = frac;
        q = -1074;
    }
    *(u64 *)buffer = *(u64 *)"0.000000"; // 8byte
    u64 cbl;
    u64 cb = c << 2;
    u64 cbr = cb | 2;
    ll k;
    bool lower_boundary_is_closer = (frac == 0);
    if (lower_boundary_is_closer) [[unlikely]]
    {
        cbl = cb - 1;
        k = (q * 1262611 - 524031) >> 22;
    }
    else
    {
        cbl = cb - 2;
        k = (q * 1262611) >> 22;
    }
    ll h = q + 1 + (((k) * (-1741647)) >> 19); //[1,4]
    const u64x2 *pow10_ptr = &g[-kMin];
    u64x2 pow10 = pow10_ptr[-k];
    u64 lo = pow10.lo;
    u64 hi = pow10.hi;
    u64 x1_l, x1_m, x1_r;
    u64 y1_l, y1_m, y1_r, y0_l, y0_m, y0_r;

    _mulx_u64(lo, cbl << h, &x1_l); // x0 not use
    y0_l = _mulx_u64(hi, cbl << h, &y1_l);
    _addcarry_u64(_addcarry_u64(0, y0_l, x1_l, &y0_l), y1_l, 0, &y1_l);
    u64 vbl = y1_l | (y0_l > 1);

    // _mulx_u64(lo, cb << h,  &x1_m); // x0 not use
    // y0_m = _mulx_u64(hi, cb << h,  &y1_m);
    // _addcarry_u64(_addcarry_u64(0, y0_m, x1_m, &y0_m), y1_m, 0, &y1_m);
    // u64 vb  = y1_m | (y0_m > 1);

    // _mulx_u64(lo, cbr << h, &x1_r); // x0 not use
    // y0_r = _mulx_u64(hi, cbr << h, &y1_r);
    // _addcarry_u64(_addcarry_u64(0, y0_r, x1_r, &y0_r), y1_r, 0, &y1_r);
    // u64 vbr = y1_r | (y0_r > 1);

    // u64 r1 = hi >> (63 - h);
    // u64 r2 = ((hi << h) << 1) | (lo >> (63 - h));
    // u64 r2_vbr = r2 + y0;
    // u64 r1_vbr = r1 + y1 + (r2_vbr < r2);
    // u64 vbr = r1_vbr | (r2_vbr > 1);
    // u64 vbl = RoundToOdd(pow10, cbl << h);
    u64 vb = RoundToOdd(pow10, cb << h);
    u64 vbr = RoundToOdd(pow10, cbr << h);
    u64 lower = vbl + (c & 1);
    u64 upper = vbr - (c & 1);
    u64 s = vb >> 2;
    u64 sp;
    _mulx_u64(1844674407370955162ull, s, &sp); // sp = s/10
    u64 sp10 = sp * 10;
    u64 digit_out = s;
    digit_out += (((vb & -4) < lower) |
                  ((vb | 3) < upper) |
                  ((vb & 3) == 3) |
                  ((vb & 7) == 6)); // s or s + 1
    if (lower <= sp10 * 4)
        digit_out = sp10;
    if (sp10 * 4 + 40 <= upper)
        digit_out = sp10 + 10;
    // compute digit_out and k end; then print to buffer
    // result = digit_out * 10^k
    ll e10 = k;
    if (((u64)(1e15) <= digit_out) & (digit_out < (u64)(1e16))) // 16 digit
    {
        digit_out *= 10; // format to 17 digit
        e10 += 15;
    }
    else
    {
        e10 += 16;
    }
    int flag = ((-7 <= e10) & (e10 <= -1));
    int flag2 = ((-7 <= e10) & (e10 <= 0));
    uint32_t high1 = digit_out / (ll)(1e16);       // 1<= high1 <=9
    uint32_t high9 = digit_out / (ll)(1e8);        // 1e8 <= high9 < 1e9
    uint32_t high2_9 = high9 - high1 * (ll)(1e8);  // 0<= high2_9 < 1e8
    uint32_t low8 = digit_out - high9 * (ll)(1e8); // 0<= low8 < 1e8
    int ctz;                                       // count tail zero
    const __m128i a0 = Convert8DigitsSSE2((uint32_t)high2_9);
    const __m128i a1 = Convert8DigitsSSE2((uint32_t)low8);
    // Convert to bytes, add '0' , or '0'
    const __m128i va = _mm_or_si128(_mm_packus_epi16(a0, a1), _mm_set1_epi8('0'));
    __m128i num_low16_print = va;
    // ll high2_9_print = _mm_extract_epi64(num_low16_print, 0);
    // ll low8_print = _mm_extract_epi64(num_low16_print, 1);
    // Count tail zero
    const unsigned mask = _mm_movemask_epi8(_mm_cmpeq_epi8(va, _mm_set1_epi8('0')));
#ifdef _MSC_VER
    unsigned long ctz_t;
    _BitScanForward(&ctz_t, ((~mask) << 16) | (0x4000 << flag));
    ctz = ctz_t;
#else
    ctz = __builtin_clz(((~mask) << 16) | (0x4000 << flag)); // when ctz = 16 and flag==0 set ctz=17
#endif
    char *ptr = buffer;
    ll start_write_pos = flag ? (1 - e10) : 0; // when e10 range in [-7,-1] , start_pos = 1-e10
    *(short *)(ptr + start_write_pos) = high1 | ('.' * 256 + '0');
    _mm_storeu_si128((__m128i *)(ptr + start_write_pos + 2 - flag), num_low16_print);
#if 1
    if (((1 <= e10) & (e10 <= 8))) // not exec this code the print result also right
    {
        // v range in [1e1 ,1e9)
        // 23 -> 23
        // 100 -> 100
        // 12.34 -> 12.34
        ll float_point_pos = e10 + 1;
        ll dot_right_value = *(ll *)(ptr + float_point_pos + 1);
        ll dot_left_value = _mm_extract_epi64(num_low16_print, 0); // high2_9 print result; sse4.1
        *(ll *)(ptr + 1) = dot_left_value;                         // move high2_9 print result to orgin dot pos

        // origin code ; when print integer not open
        // *(ptr + float_point_pos) = (ctz + e10 < 16) ? '.' : 0;                     // xx.00 ; if dot right value = all 0 , not print dot
        // *(ll *)(ptr + float_point_pos + 1) = dot_right_value;                      //
        // int end_pos = (18 - ctz > float_point_pos) ? (18 - ctz) : float_point_pos; // max(18-ctz,float_point_pos)
        // *(ptr + end_pos) = '\0';                                                   // remove tail zero
        // return sign + ((ctz + e10 < 16) ? end_pos : float_point_pos);              // length = if(ctz + e10 < 16) end_pos else float_point_pos

        // when print integer open,use this code
        *(ptr + float_point_pos) = '.';
        *(ll *)(ptr + float_point_pos + 1) = dot_right_value;
        *(ptr + 18 - ctz) = '\0';
        return sign + 18 - ctz;
    }
#endif
    const ll *exp_ptr = &exp_result3[324];
    *(ll *)(ptr + 18 - flag + start_write_pos - ctz) = flag2 ? 0 : exp_ptr[e10]; // remove tail zero
    // when double value < 1e-309 ; equal digit_out < 1e15
    // also 5e-324 <= v < 1e-309 ;
#if 1
    if (digit_out < (ull)(1e15)) // [[unlikely]]
    {
        char *buf_ptr = ptr;
        u64 len = dec_length(digit_out); // 1<= len <= 15
        u64 tz_num = ctz;
        u64 lz_num = 17 - len; // left zero num: lz_num >= 2
        // such as 00000001234500000 ; significant = 5,tz = 5,lz= 7
        // u64 signficant = 17 - tz_num - lz_num;// also = len - ctz
        u64 signficant = len - tz_num;
        u64 start_pos = lz_num + 1; // 0.00xxx ; first x pos is 3 + 1
        if (signficant > 1ull)      // [[likely]]
        {
            buf_ptr[0] = buf_ptr[start_pos];
            if (signficant <= 9) // move 8 byte
                *(i64 *)&buf_ptr[2] = *(i64 *)&buf_ptr[start_pos + 1];
            else
            { // move 16 byte
                *(i64 *)&buf_ptr[2] = *(i64 *)&buf_ptr[start_pos + 1];
                *(i64 *)&buf_ptr[2 + 8] = *(i64 *)&buf_ptr[start_pos + 1 + 8];
            }
            *(i64 *)&buf_ptr[signficant + 1] = exp_ptr[e10 - lz_num]; // write exp10 ASCII
            return sign + 1 + signficant + 5;                         // 1.2e-310 , [1.2] -> 1+signficant , [e-310] -> 5  ; sign is 1, negative double
        }
        else
        {
            *(char *)&buf_ptr[0] = buf_ptr[start_pos];
            *(i64 *)&buf_ptr[1] = exp_ptr[e10 - lz_num];
            return sign + 1 + 5; // example : 5e-324 , 1e-310 , 1e-323 ; sign is 1, negative double
        }
    }
#endif
    int exp10_length = 4 | (e10 >= 100) | (e10 <= -100);                          // 4 + (abs(e10)>=100) ; e+10 = 4 , e+100 = 5
    return sign + start_write_pos + 18 - ctz - flag + (flag2 ? 0 : exp10_length); // write buf length
}





// ========================================= for benchmark =========================================

extern "C" {
#ifdef HAVE_AVX512
char *dtoa_xjb_avx512(double val, char *buf) {
    buf += d2s_avx512(val, buf);
    *buf = '\0';
    return buf;
}
#endif
char *dtoa_xjb_sse(double val, char *buf) {
    buf += d2s_sse(val, buf);
    *buf = '\0';
    return buf;
}
}


#endif
#endif