/*
 Code from https://github.com/ibireme/yyjson
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>


/*==============================================================================
 * Compiler Macros
 *============================================================================*/

/* compiler builtin check (clang) */
#ifndef yy_has_builtin
#   ifdef __has_builtin
#       define yy_has_builtin(x) __has_builtin(x)
#   else
#       define yy_has_builtin(x) 0
#   endif
#endif

/* compiler attribute check (gcc/clang) */
#ifndef yy_has_attribute
#   ifdef __has_attribute
#       define yy_has_attribute(x) __has_attribute(x)
#   else
#       define yy_has_attribute(x) 0
#   endif
#endif

/* inline */
#ifndef yy_inline
#   if _MSC_VER >= 1200
#       define yy_inline __forceinline
#   elif defined(_MSC_VER)
#       define yy_inline __inline
#   elif yy_has_attribute(always_inline) || __GNUC__ >= 4
#       define yy_inline __inline__ __attribute__((always_inline))
#   elif defined(__clang__) || defined(__GNUC__)
#       define yy_inline __inline__
#   elif defined(__cplusplus) || (__STDC__ >= 1 && __STDC_VERSION__ >= 199901L)
#       define yy_inline inline
#   else
#       define yy_inline
#   endif
#endif

/* noinline */
#ifndef yy_noinline
#   if _MSC_VER >= 1200
#       define yy_noinline __declspec(noinline)
#   elif yy_has_attribute(noinline) || __GNUC__ >= 4
#       define yy_noinline __attribute__((noinline))
#   else
#       define yy_noinline
#   endif
#endif

/* align */
#ifndef yy_align
#   if defined(_MSC_VER)
#       define yy_align(x) __declspec(align(x))
#   elif yy_has_attribute(aligned) || defined(__GNUC__)
#       define yy_align(x) __attribute__((aligned(x)))
#   elif __cplusplus >= 201103L
#       define yy_align(x) alignas(x)
#   else
#       define yy_align(x)
#   endif
#endif

/* likely */
#ifndef yy_likely
#   if yy_has_builtin(__builtin_expect) || __GNUC__ >= 4
#       define yy_likely(expr) __builtin_expect(!!(expr), 1)
#   else
#       define yy_likely(expr) (expr)
#   endif
#endif

/* unlikely */
#ifndef yy_unlikely
#   if yy_has_builtin(__builtin_expect) || __GNUC__ >= 4
#       define yy_unlikely(expr) __builtin_expect(!!(expr), 0)
#   else
#       define yy_unlikely(expr) (expr)
#   endif
#endif


/*==============================================================================
 * Flags
 *============================================================================*/

/* gcc version check */
#ifndef yy_gcc_available
#   define yy_gcc_available(major, minor, patch) \
        ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= \
        (major * 10000 + minor * 100 + patch))
#endif

/* msvc intrinsic */
#if _MSC_VER >= 1400
#   include <intrin.h>
#   if defined(_M_AMD64) || defined(_M_ARM64)
#       define MSC_HAS_BIT_SCAN_64 1
#       pragma intrinsic(_BitScanForward64)
#       pragma intrinsic(_BitScanReverse64)
#   endif
#   if defined(_M_AMD64) || defined(_M_ARM64) || \
        defined(_M_IX86) || defined(_M_ARM)
#       define MSC_HAS_BIT_SCAN 1
#       pragma intrinsic(_BitScanForward)
#       pragma intrinsic(_BitScanReverse)
#   endif
#   if defined(_M_AMD64)
#       define MSC_HAS_UMUL128 1
#       pragma intrinsic(_umul128)
#   endif
#endif

/* gcc builtin */
#if yy_has_builtin(__builtin_clzll) || yy_gcc_available(3, 4, 0)
#   define GCC_HAS_CLZLL 1
#endif

#if yy_has_builtin(__builtin_ctzll) || yy_gcc_available(3, 4, 0)
#   define GCC_HAS_CTZLL 1
#endif

/* int128 type */
#ifndef YY_HAS_INT128
#   if (__SIZEOF_INT128__ == 16) && \
       (defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER))
#       define YY_HAS_INT128 1
#   else
#       define YY_HAS_INT128 0
#   endif
#endif

/*==============================================================================
 * Macros
 *============================================================================*/

/* Macros used for loop unrolling and other purpose. */

/* Macros used to provide branch prediction information for compiler. */
#undef  likely
#define likely(x)       yy_likely(x)
#undef  unlikely
#define unlikely(x)     yy_unlikely(x)

/* Macros used to provide inline information for compiler. */
#undef  static_inline
#define static_inline   static yy_inline
#undef  static_noinline
#define static_noinline static yy_noinline

/* Used to write u64 literal for C89 which doesn't support "ULL" suffix. */
#undef  U64
#define U64(hi, lo) ((((u64)hi##UL) << 32) + lo##UL)



/*==============================================================================
 * Integer Constants
 *============================================================================*/

/* U64 constant values */
#undef  U64_MAX
#define U64_MAX         U64(0xFFFFFFFF, 0xFFFFFFFF)
#undef  I64_MAX
#define I64_MAX         U64(0x7FFFFFFF, 0xFFFFFFFF)
#undef  USIZE_MAX
#define USIZE_MAX       ((usize)(~(usize)0))

/* Maximum number of digits for reading u64 safety. */
#undef  U64_SAFE_DIG
#define U64_SAFE_DIG    19

/* Padding size for JSON reader input. */
#undef  PADDING_SIZE
#define PADDING_SIZE    4


/*==============================================================================
 * IEEE-754 Double Number Constants
 *============================================================================*/

/* Inf raw value */
#define F64_RAW_INF U64(0x7FF00000, 0x00000000)

/* NaN raw value */
#define F64_RAW_NAN U64(0x7FF80000, 0x00000000)

/* double number bits */
#define F64_BITS 64

/* double number exponent part bits */
#define F64_EXP_BITS 11

/* double number significand part bits */
#define F64_SIG_BITS 52

/* double number significand part bits (with 1 hidden bit) */
#define F64_SIG_FULL_BITS 53

/* double number significand bit mask */
#define F64_SIG_MASK U64(0x000FFFFF, 0xFFFFFFFF)

/* double number exponent bit mask */
#define F64_EXP_MASK U64(0x7FF00000, 0x00000000)

/* double number exponent bias */
#define F64_EXP_BIAS 1023

/* double number significant digits count in decimal */
#define F64_DEC_DIG 17

/* max significant digits count in decimal when reading double number */
#define F64_MAX_DEC_DIG 768

/* maximum decimal power of normal number (1.7976931348623157e308) */
#define F64_MAX_DEC_EXP 308

/* minimum decimal power of normal number (4.9406564584124654e-324) */
#define F64_MIN_DEC_EXP -324

/* maximum binary power of normal number */
#define F64_MAX_BIN_EXP 1024

/* minimum binary power of normal number */
#define F64_MIN_BIN_EXP -1021



/*==============================================================================
 * Types
 *============================================================================*/

/** Type define for primitive types. */
typedef float       f32;
typedef double      f64;
typedef int8_t      i8;
typedef uint8_t     u8;
typedef int16_t     i16;
typedef uint16_t    u16;
typedef int32_t     i32;
typedef uint32_t    u32;
typedef int64_t     i64;
typedef uint64_t    u64;
typedef size_t      usize;
#if YY_HAS_INT128
__extension__ typedef __int128          i128;
__extension__ typedef unsigned __int128 u128;
#endif

/** 16/32/64-bits vector */
typedef struct v16 { char c1, c2; } v16;
typedef struct v32 { char c1, c2, c3, c4; } v32;
typedef struct v64 { char c1, c2, c3, c4, c5, c6, c7, c8; } v64;

/** 16/32/64-bits vector union, used for unalignd memory access on modern CPU */
typedef union v16_uni { v16 v; u16 u; } v16_uni;
typedef union v32_uni { v32 v; u32 u; } v32_uni;
typedef union v64_uni { v64 v; u64 u; } v64_uni;

/** 64-bit floating point union, used to avoid the type-based aliasing rule */
typedef union { u64 u; f64 f; } f64_uni;

#if _MSC_VER || (__STDC__ >= 1 && __STDC_VERSION__ >= 199901L)

#define v16_make(c1, c2) \
    ((v16){c1, c2})

#define v32_make(c1, c2, c3, c4) \
    ((v32){c1, c2, c3, c4})

#else

static_inline v16 v16_make(char c1, char c2) {
    v16 v;
    v.c1 = c1;
    v.c2 = c2;
    return v;
}

static_inline v32 v32_make(char c1, char c2, char c3, char c4) {
    v32 v;
    v.c1 = c1;
    v.c2 = c2;
    v.c3 = c3;
    v.c4 = c4;
    return v;
}

#endif



/*==============================================================================
 * Number Utils
 *============================================================================*/

/** Convert raw binary to double. */
static_inline f64 f64_from_raw(u64 u) {
    f64_uni uni;
    uni.u = u;
    return uni.f;
}

/** Convert double to raw binary. */
static_inline u64 f64_to_raw(f64 f) {
    f64_uni uni;
    uni.f = f;
    return uni.u;
}



/*==============================================================================
 * Bits Utils
 *============================================================================*/

/** Returns the number of leading 0-bits in value (input should not be 0). */
static_inline
u32 u64_lz_bits(u64 v) {
#if GCC_HAS_CLZLL
    return (u32)__builtin_clzll(v);
#elif MSC_HAS_BIT_SCAN_64
    u32 r;
    _BitScanReverse64(&r, v);
    return (u32)63 - r;
#elif MSC_HAS_BIT_SCAN
    u32 hi, lo;
    bool hi_set = _BitScanReverse(&hi, (u32)(v >> 32)) != 0;
    _BitScanReverse(&lo, (u32)v);
    hi |= 32;
    return (u32)63 - (u32)(hi_set ? hi : lo);
#else
    /* branchless, use de Bruijn sequences */
    const u8 table[64] = {
        63, 16, 62,  7, 15, 36, 61,  3,  6, 14, 22, 26, 35, 47, 60,  2,
         9,  5, 28, 11, 13, 21, 42, 19, 25, 31, 34, 40, 46, 52, 59,  1,
        17,  8, 37,  4, 23, 27, 48, 10, 29, 12, 43, 20, 32, 41, 53, 18,
        38, 24, 49, 30, 44, 33, 54, 39, 50, 45, 55, 51, 56, 57, 58,  0
    };
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    return table[(v * U64(0x03F79D71, 0xB4CB0A89)) >> 58];
#endif
}

/** Returns the number of trailing 0-bits in value (input should not be 0). */
static_inline u32 u64_tz_bits(u64 v) {
#if GCC_HAS_CTZLL
    return (u32)__builtin_ctzll(v);
#elif MSC_HAS_BIT_SCAN_64
    u32 r;
    _BitScanForward64(&r, v);
    return r;
#elif MSC_HAS_BIT_SCAN
    u32 lo, hi;
    bool lo_set = _BitScanForward(&lo, (u32)(v)) != 0;
    _BitScanForward(&hi, (u32)(v >> 32));
    hi += 32;
    return lo_set ? lo : hi;
#else
    /* branchless, use de Bruijn sequences */
    const u8 table[64] = {
         0,  1,  2, 53,  3,  7, 54, 27,  4, 38, 41,  8, 34, 55, 48, 28,
        62,  5, 39, 46, 44, 42, 22,  9, 24, 35, 59, 56, 49, 18, 29, 11,
        63, 52,  6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
        51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12
    };
    return table[((v & (~v + 1)) * U64(0x022FDD63, 0xCC95386D)) >> 58];
#endif
}



/*==============================================================================
 * 128-bit Integer Utils
 *============================================================================*/

/** Multiplies two 64-bit unsigned integers (a * b),
    returns the 128-bit result as 'hi' and 'lo'. */
static_inline void u128_mul(u64 a, u64 b, u64 *hi, u64 *lo) {
#if YY_HAS_INT128
    u128 m = (u128)a * b;
    *hi = (u64)(m >> 64);
    *lo = (u64)(m);
#elif MSC_HAS_UMUL128
    *lo = _umul128(a, b, hi);
#else
    u32 a0 = (u32)(a), a1 = (u32)(a >> 32);
    u32 b0 = (u32)(b), b1 = (u32)(b >> 32);
    u64 p00 = (u64)a0 * b0, p01 = (u64)a0 * b1;
    u64 p10 = (u64)a1 * b0, p11 = (u64)a1 * b1;
    u64 m0 = p01 + (p00 >> 32);
    u32 m00 = (u32)(m0), m01 = (u32)(m0 >> 32);
    u64 m1 = p10 + m00;
    u32 m10 = (u32)(m1), m11 = (u32)(m1 >> 32);
    *hi = p11 + m01 + m11;
    *lo = ((u64)m10 << 32) | (u32)p00;
#endif
}

/** Multiplies two 64-bit unsigned integers and add a value (a * b + c),
    returns the 128-bit result as 'hi' and 'lo'. */
static_inline void u128_mul_add(u64 a, u64 b, u64 c, u64 *hi, u64 *lo) {
#if YY_HAS_INT128
    u128 m = (u128)a * b + c;
    *hi = (u64)(m >> 64);
    *lo = (u64)(m);
#else
    u64 h, l, t;
    u128_mul(a, b, &h, &l);
    t = l + c;
    h += ((t < l) | (t < c));
    *hi = h;
    *lo = t;
#endif
}



/*==============================================================================
 * Integer Writer
 *============================================================================*/

/** Digit table from 00 to 99. */
yy_align(2)
static const char digit_table[200] = {
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4',
    '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
    '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
    '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
    '2', '0', '2', '1', '2', '2', '2', '3', '2', '4',
    '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4',
    '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
    '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
    '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
    '5', '0', '5', '1', '5', '2', '5', '3', '5', '4',
    '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4',
    '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
    '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
    '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
    '8', '0', '8', '1', '8', '2', '8', '3', '8', '4',
    '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4',
    '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
};

static_inline u8 *write_u32_len_8(u32 val, u8 *buf) {
    /* 8 digits: aabbccdd */
    u32 aa, bb, cc, dd, aabb, ccdd;
    aabb = (u32)(((u64)val * 109951163) >> 40); /* (val / 10000) */
    ccdd = val - aabb * 10000; /* (val % 10000) */
    aa = (aabb * 5243) >> 19; /* (aabb / 100) */
    cc = (ccdd * 5243) >> 19; /* (ccdd / 100) */
    bb = aabb - aa * 100; /* (aabb % 100) */
    dd = ccdd - cc * 100; /* (ccdd % 100) */
    ((v16 *)buf)[0] = ((v16 *)digit_table)[aa];
    ((v16 *)buf)[1] = ((v16 *)digit_table)[bb];
    ((v16 *)buf)[2] = ((v16 *)digit_table)[cc];
    ((v16 *)buf)[3] = ((v16 *)digit_table)[dd];
    return buf + 8;
}

static_inline u8 *write_u32_len_1_8(u32 val, u8 *buf) {
    u32 aa, bb, cc, dd, aabb, bbcc, ccdd, lz;
    
    if (val < 100) { /* 1-2 digits: aa */
        lz = val < 10;
        ((v16 *)buf)[0] = *(v16 *)&(digit_table[val * 2 + lz]);
        buf -= lz;
        return buf + 2;
        
    } else if (val < 10000) { /* 3-4 digits: aabb */
        aa = (val * 5243) >> 19; /* (val / 100) */
        bb = val - aa * 100; /* (val % 100) */
        lz = aa < 10;
        ((v16 *)buf)[0] = *(v16 *)&(digit_table[aa * 2 + lz]);
        buf -= lz;
        ((v16 *)buf)[1] = ((v16 *)digit_table)[bb];
        return buf + 4;
        
    } else if (val < 1000000) { /* 5-6 digits: aabbcc */
        aa = (u32)(((u64)val * 429497) >> 32); /* (val / 10000) */
        bbcc = val - aa * 10000; /* (val % 10000) */
        bb = (bbcc * 5243) >> 19; /* (bbcc / 100) */
        cc = bbcc - bb * 100; /* (bbcc % 100) */
        lz = aa < 10;
        ((v16 *)buf)[0] = *(v16 *)&(digit_table[aa * 2 + lz]);
        buf -= lz;
        ((v16 *)buf)[1] = ((v16 *)digit_table)[bb];
        ((v16 *)buf)[2] = ((v16 *)digit_table)[cc];
        return buf + 6;
        
    } else { /* 7-8 digits: aabbccdd */
        aabb = (u32)(((u64)val * 109951163) >> 40); /* (val / 10000) */
        ccdd = val - aabb * 10000; /* (val % 10000) */
        aa = (aabb * 5243) >> 19; /* (aabb / 100) */
        cc = (ccdd * 5243) >> 19; /* (ccdd / 100) */
        bb = aabb - aa * 100; /* (aabb % 100) */
        dd = ccdd - cc * 100; /* (ccdd % 100) */
        lz = aa < 10;
        ((v16 *)buf)[0] = *(v16 *)&(digit_table[aa * 2 + lz]);
        buf -= lz;
        ((v16 *)buf)[1] = ((v16 *)digit_table)[bb];
        ((v16 *)buf)[2] = ((v16 *)digit_table)[cc];
        ((v16 *)buf)[3] = ((v16 *)digit_table)[dd];
        return buf + 8;
    }
}


/*==============================================================================
 * Number Writer
 *============================================================================*/

/** The significant bits kept in pow5_inv_sig_table. */
#define POW5_INV_SIG_BITS 122

/** The significant bits table of pow5 multiplicative inverse.
    (generate with yy_make_tables.c) */
static const u64 pow5_inv_sig_table[291][2] = {
    { U64(0x04000000, 0x00000000), U64(0x00000000, 0x00000001) },
    { U64(0x03333333, 0x33333333), U64(0x33333333, 0x33333334) },
    { U64(0x028F5C28, 0xF5C28F5C), U64(0x28F5C28F, 0x5C28F5C3) },
    { U64(0x020C49BA, 0x5E353F7C), U64(0xED916872, 0xB020C49C) },
    { U64(0x0346DC5D, 0x63886594), U64(0xAF4F0D84, 0x4D013A93) },
    { U64(0x029F16B1, 0x1C6D1E10), U64(0x8C3F3E03, 0x70CDC876) },
    { U64(0x0218DEF4, 0x16BDB1A6), U64(0xD698FE69, 0x270B06C5) },
    { U64(0x035AFE53, 0x5795E90A), U64(0xF0F4CA41, 0xD811A46E) },
    { U64(0x02AF31DC, 0x4611873B), U64(0xF3F70834, 0xACDAE9F1) },
    { U64(0x0225C17D, 0x04DAD296), U64(0x5CC5A02A, 0x23E254C1) },
    { U64(0x036F9BFB, 0x3AF7B756), U64(0xFAD5CD10, 0x396A2135) },
    { U64(0x02BFAFFC, 0x2F2C92AB), U64(0xFBDE3DA6, 0x9454E75E) },
    { U64(0x0232F330, 0x25BD4223), U64(0x2FE4FE1E, 0xDD10B918) },
    { U64(0x0384B84D, 0x092ED038), U64(0x4CA19697, 0xC81AC1BF) },
    { U64(0x02D09370, 0xD4257360), U64(0x3D4E1213, 0x067BCE33) },
    { U64(0x024075F3, 0xDCEAC2B3), U64(0x643E74DC, 0x052FD829) },
    { U64(0x039A5652, 0xFB113785), U64(0x6D30BAF9, 0xA1E626A7) },
    { U64(0x02E1DEA8, 0xC8DA92D1), U64(0x2426FBFA, 0xE7EB5220) },
    { U64(0x024E4BBA, 0x3A487574), U64(0x1CEBFCC8, 0xB9890E80) },
    { U64(0x03B07929, 0xF6DA5586), U64(0x94ACC7A7, 0x8F41B0CC) },
    { U64(0x02F39421, 0x9248446B), U64(0xAA23D2EC, 0x729AF3D7) },
    { U64(0x025C7681, 0x41D369EF), U64(0xBB4FDBF0, 0x5BAF2979) },
    { U64(0x03C72402, 0x02EBDCB2), U64(0xC54C931A, 0x2C4B758D) },
    { U64(0x0305B668, 0x02564A28), U64(0x9DD6DC14, 0xF03C5E0B) },
    { U64(0x026AF853, 0x3511D4ED), U64(0x4B1249AA, 0x59C9E4D6) },
    { U64(0x03DE5A1E, 0xBB4FBB15), U64(0x44EA0F76, 0xF60FD489) },
    { U64(0x03184818, 0x95D96277), U64(0x6A54D92B, 0xF80CAA07) },
    { U64(0x0279D346, 0xDE4781F9), U64(0x21DD7A89, 0x933D54D2) },
    { U64(0x03F61ED7, 0xCA0C0328), U64(0x362F2A75, 0xB8622150) },
    { U64(0x032B4BDF, 0xD4D668EC), U64(0xF825BB91, 0x604E810D) },
    { U64(0x0289097F, 0xDD7853F0), U64(0xC684960D, 0xE6A5340B) },
    { U64(0x02073ACC, 0xB12D0FF3), U64(0xD203AB3E, 0x521DC33C) },
    { U64(0x033EC47A, 0xB514E652), U64(0xE99F7863, 0xB696052C) },
    { U64(0x02989D2E, 0xF743EB75), U64(0x87B2C6B6, 0x2BAB3757) },
    { U64(0x0213B0F2, 0x5F69892A), U64(0xD2F56BC4, 0xEFBC2C45) },
    { U64(0x0352B4B6, 0xFF0F41DE), U64(0x1E55793B, 0x192D13A2) },
    { U64(0x02A89092, 0x65A5CE4B), U64(0x4B77942F, 0x475742E8) },
    { U64(0x022073A8, 0x515171D5), U64(0xD5F94359, 0x05DF68BA) },
    { U64(0x03671F73, 0xB54F1C89), U64(0x565B9EF4, 0xD6324129) },
    { U64(0x02B8E5F6, 0x2AA5B06D), U64(0xDEAFB25D, 0x78283421) },
    { U64(0x022D84C4, 0xEEEAF38B), U64(0x188C8EB1, 0x2CECF681) },
    { U64(0x037C07A1, 0x7E44B8DE), U64(0x8DADB11B, 0x7B14BD9B) },
    { U64(0x02C99FB4, 0x6503C718), U64(0x7157C0E2, 0xC8DD647C) },
    { U64(0x023AE629, 0xEA696C13), U64(0x8DDFCD82, 0x3A4AB6CA) },
    { U64(0x03917043, 0x10A8ACEC), U64(0x1632E269, 0xF6DDF142) },
    { U64(0x02DAC035, 0xA6ED5723), U64(0x44F581EE, 0x5F17F435) },
    { U64(0x024899C4, 0x858AAC1C), U64(0x372ACE58, 0x4C1329C4) },
    { U64(0x03A75C6D, 0xA27779C6), U64(0xBEAAE3C0, 0x79B842D3) },
    { U64(0x02EC49F1, 0x4EC5FB05), U64(0x65558300, 0x61603576) },
    { U64(0x0256A18D, 0xD89E626A), U64(0xB7779C00, 0x4DE6912B) },
    { U64(0x03BDCF49, 0x5A9703DD), U64(0xF258F99A, 0x163DB512) },
    { U64(0x02FE3F6D, 0xE212697E), U64(0x5B7A6148, 0x11CAF741) },
    { U64(0x0264FF8B, 0x1B41EDFE), U64(0xAF951AA0, 0x0E3BF901) },
    { U64(0x03D4CC11, 0xC5364997), U64(0x7F54F766, 0x7D2CC19B) },
    { U64(0x0310A341, 0x6A91D479), U64(0x32AA5F85, 0x30F09AE3) },
    { U64(0x0273B5CD, 0xEEDB1060), U64(0xF5551937, 0x5A5A1582) },
    { U64(0x03EC5616, 0x4AF81A34), U64(0xBBBB5B8B, 0xC3C3559D) },
    { U64(0x03237811, 0xD593482A), U64(0x2FC91609, 0x6969114A) },
    { U64(0x0282C674, 0xAADC39BB), U64(0x596DAB3A, 0xBABA743C) },
    { U64(0x0202385D, 0x557CFAFC), U64(0x478AEF62, 0x2EFB9030) },
    { U64(0x0336C095, 0x5594C4C6), U64(0xD8DE4BD0, 0x4B2C19E6) },
    { U64(0x029233AA, 0xAADD6A38), U64(0xAD7EA30D, 0x08F014B8) },
    { U64(0x020E8FBB, 0xBBE454FA), U64(0x24654F3D, 0xA0C01093) },
    { U64(0x034A7F92, 0xC63A2190), U64(0x3A3BB1FC, 0x346680EB) },
    { U64(0x02A1FFA8, 0x9E94E7A6), U64(0x94FC8E63, 0x5D1ECD89) },
    { U64(0x021B32ED, 0x4BAA52EB), U64(0xAA63A51C, 0x4A7F0AD4) },
    { U64(0x035EB7E2, 0x12AA1E45), U64(0xDD6C3B60, 0x7731AAED) },
    { U64(0x02B22CB4, 0xDBBB4B6B), U64(0x1789C919, 0xF8F488BD) },
    { U64(0x022823C3, 0xE2FC3C55), U64(0xAC6E3A7B, 0x2D906D64) },
    { U64(0x03736C6C, 0x9E606089), U64(0x13E390C5, 0x15B3E23A) },
    { U64(0x02C2BD23, 0xB1E6B3A0), U64(0xDCB60D6A, 0x77C31B62) },
    { U64(0x0235641C, 0x8E52294D), U64(0x7D5E7121, 0xF968E2B5) },
    { U64(0x0388A02D, 0xB0837548), U64(0xC8971B69, 0x8F0E3787) },
    { U64(0x02D3B357, 0xC0692AA0), U64(0xA078E2BA, 0xD8D82C6C) },
    { U64(0x0242F5DF, 0xCD20EEE6), U64(0xE6C71BC8, 0xAD79BD24) },
    { U64(0x039E5632, 0xE1CE4B0B), U64(0x0AD82C74, 0x48C2C839) },
    { U64(0x02E511C2, 0x4E3EA26F), U64(0x3BE02390, 0x3A356CFA) },
    { U64(0x0250DB01, 0xD8321B8C), U64(0x2FE682D9, 0xC82ABD95) },
    { U64(0x03B4919C, 0x8D1CF8E0), U64(0x4CA4048F, 0xA6AAC8EE) },
    { U64(0x02F6DAE3, 0xA4172D80), U64(0x3D5003A6, 0x1EEF0725) },
    { U64(0x025F1582, 0xE9AC2466), U64(0x9773361E, 0x7F259F51) },
    { U64(0x03CB559E, 0x42AD070A), U64(0x8BEB89CA, 0x6508FEE8) },
    { U64(0x0309114B, 0x688A6C08), U64(0x6FEFA16E, 0xB73A6586) },
    { U64(0x026DA76F, 0x86D52339), U64(0xF3261ABE, 0xF8FB846B) },
    { U64(0x03E2A57F, 0x3E21D1F6), U64(0x51D69131, 0x8E5F3A45) },
    { U64(0x031BB798, 0xFE8174C5), U64(0x0E4540F4, 0x71E5C837) },
    { U64(0x027C92E0, 0xCB9AC3D0), U64(0xD8376729, 0xF4B7D360) },
    { U64(0x03FA849A, 0xDF5E061A), U64(0xF38BD843, 0x21261EFF) },
    { U64(0x032ED07B, 0xE5E4D1AF), U64(0x293CAD02, 0x80EB4BFF) },
    { U64(0x028BD9FC, 0xB7EA4158), U64(0xEDCA2402, 0x00BC3CCC) },
    { U64(0x02097B30, 0x9321CDE0), U64(0xBE3B5001, 0x9A3030A4) },
    { U64(0x03425EB4, 0x1E9C7C9A), U64(0xC9F88002, 0x904D1A9F) },
    { U64(0x029B7EF6, 0x7EE396E2), U64(0x3B2D3335, 0x403DAEE6) },
    { U64(0x0215FF2B, 0x98B6124E), U64(0x95BDC291, 0x003158B8) },
    { U64(0x03566512, 0x8DF01D4A), U64(0x892F9DB4, 0xCD1BC126) },
    { U64(0x02AB840E, 0xD7F34AA2), U64(0x07594AF7, 0x0A7C9A85) },
    { U64(0x0222D00B, 0xDFF5D54E), U64(0x6C476F2C, 0x0863AED1) },
    { U64(0x036AE679, 0x66562217), U64(0x13A57EAC, 0xDA3917B4) },
    { U64(0x02BBEB94, 0x51DE81AC), U64(0x0FB7988A, 0x482DAC90) },
    { U64(0x022FEFA9, 0xDB1867BC), U64(0xD95FAD3B, 0x6CF156DA) },
    { U64(0x037FE5DC, 0x91C0A5FA), U64(0xF565E1F8, 0xAE4EF15C) },
    { U64(0x02CCB7E3, 0xA7CD5195), U64(0x911E4E60, 0x8B725AB0) },
    { U64(0x023D5FE9, 0x530AA7AA), U64(0xDA7EA51A, 0x0928488D) },
    { U64(0x03956642, 0x1E7772AA), U64(0xF7310829, 0xA8407415) },
    { U64(0x02DDEB68, 0x185F8EEF), U64(0x2C2739BA, 0xED005CDE) },
    { U64(0x024B22B9, 0xAD193F25), U64(0xBCEC2E2F, 0x24004A4B) },
    { U64(0x03AB6AC2, 0xAE8ECB6F), U64(0x94AD16B1, 0xD333AA11) },
    { U64(0x02EF889B, 0xBED8A2BF), U64(0xAA241227, 0xDC2954DB) },
    { U64(0x02593A16, 0x3246E899), U64(0x54E9A81F, 0xE35443E2) },
    { U64(0x03C1F689, 0xEA0B0DC2), U64(0x2175D9CC, 0x9EED396A) },
    { U64(0x03019207, 0xEE6F3E34), U64(0xE7917B0A, 0x18BDC788) },
    { U64(0x0267A806, 0x5858FE90), U64(0xB9412F3B, 0x46FE393A) },
    { U64(0x03D90CD6, 0xF3C1974D), U64(0xF535185E, 0xD7FD285C) },
    { U64(0x03140A45, 0x8FCE12A4), U64(0xC42A79E5, 0x7997537D) },
    { U64(0x02766E9E, 0x0CA4DBB7), U64(0x03552E51, 0x2E12A931) },
    { U64(0x03F0B0FC, 0xE107C5F1), U64(0x9EEEB081, 0xE3510EB4) },
    { U64(0x0326F3FD, 0x80D304C1), U64(0x4BF226CE, 0x4F740BC3) },
    { U64(0x02858FFE, 0x00A8D09A), U64(0xA3281F0B, 0x72C33C9C) },
    { U64(0x02047331, 0x9A20A6E2), U64(0x1C2018D5, 0xF568FD4A) },
    { U64(0x033A51E8, 0xF69AA49C), U64(0xF9CCF489, 0x88A7FBA9) },
    { U64(0x02950E53, 0xF87BB6E3), U64(0xFB0A5D3A, 0xD3B99621) },
    { U64(0x0210D843, 0x2D2FC583), U64(0x2F3B7DC8, 0xA96144E7) },
    { U64(0x034E26D1, 0xE1E608D1), U64(0xE52BFC74, 0x42353B0C) },
    { U64(0x02A4EBDB, 0x1B1E6D74), U64(0xB7566390, 0x34F76270) },
    { U64(0x021D897C, 0x15B1F12A), U64(0x2C451C73, 0x5D92B526) },
    { U64(0x03627593, 0x55E981DD), U64(0x13A1C71E, 0xFC1DEEA3) },
    { U64(0x02B52ADC, 0x44BACE4A), U64(0x761B05B2, 0x634B2550) },
    { U64(0x022A88B0, 0x36FBD83B), U64(0x91AF37C1, 0xE908EAA6) },
    { U64(0x03774119, 0xF192F392), U64(0x82B1F2CF, 0xDB417770) },
    { U64(0x02C5CDAE, 0x5ADBF60E), U64(0xCEF4C23F, 0xE29AC5F3) },
    { U64(0x0237D7BE, 0xAF165E72), U64(0x3F2A34FF, 0xE87BD190) },
    { U64(0x038C8C64, 0x4B56FD83), U64(0x984387FF, 0xDA5FB5B2) },
    { U64(0x02D6D6B6, 0xA2ABFE02), U64(0xE0360666, 0x484C915B) },
    { U64(0x02457892, 0x1BBCCB35), U64(0x802B3851, 0xD3707449) },
    { U64(0x03A25A83, 0x5F947855), U64(0x99DEC082, 0xEBE72075) },
    { U64(0x02E84869, 0x19439377), U64(0xAE4BCD35, 0x8985B391) },
    { U64(0x02536D20, 0xE102DC5F), U64(0xBEA30A91, 0x3AD15C74) },
    { U64(0x03B8AE9B, 0x019E2D65), U64(0xFDD1AA81, 0xF7B560B9) },
    { U64(0x02FA2548, 0xCE182451), U64(0x97DAEECE, 0x5FC44D61) },
    { U64(0x0261B76D, 0x71ACE9DA), U64(0xDFE258A5, 0x1969D781) },
    { U64(0x03CF8BE2, 0x4F7B0FC4), U64(0x996A276E, 0x8F0FBF34) },
    { U64(0x030C6FE8, 0x3F95A636), U64(0xE121B925, 0x3F3FCC2A) },
    { U64(0x02705986, 0x994484F8), U64(0xB41AFA84, 0x32997022) },
    { U64(0x03E6F5A4, 0x286DA18D), U64(0xECF7F739, 0xEA8F19CF) },
    { U64(0x031F2AE9, 0xB9F14E0B), U64(0x23F99294, 0xBBA5AE40) },
    { U64(0x027F5587, 0xC7F43E6F), U64(0x4FFADBAA, 0x2FB7BE99) },
    { U64(0x03FEEF3F, 0xA6539718), U64(0x7FF7C5DD, 0x1925FDC2) },
    { U64(0x033258FF, 0xB842DF46), U64(0xCCC637E4, 0x141E649B) },
    { U64(0x028EAD99, 0x60357F6B), U64(0xD704F983, 0x434B83AF) },
    { U64(0x020BBE14, 0x4CF79923), U64(0x126A6135, 0xCF6F9C8C) },
    { U64(0x0345FCED, 0x47F28E9E), U64(0x83DD6856, 0x18B29414) },
    { U64(0x029E63F1, 0x065BA54B), U64(0x9CB12044, 0xE08EDCDD) },
    { U64(0x02184FF4, 0x05161DD6), U64(0x16F419D0, 0xB3A57D7D) },
    { U64(0x035A1986, 0x6E89C956), U64(0x8B20294D, 0xEC3BFBFB) },
    { U64(0x02AE7AD1, 0xF207D445), U64(0x3C19BAA4, 0xBCFCC996) },
    { U64(0x02252F0E, 0x5B39769D), U64(0xC9AE2EEA, 0x30CA3ADF) },
    { U64(0x036EB1B0, 0x91F58A96), U64(0x0F7D17DD, 0x1ADD2AFD) },
    { U64(0x02BEF48D, 0x41913BAB), U64(0x3F97464A, 0x7BE42264) },
    { U64(0x02325D3D, 0xCE0DC955), U64(0xCC790508, 0x631CE850) },
    { U64(0x0383C862, 0xE3494222), U64(0xE0C1A1A7, 0x04FB0D4D) },
    { U64(0x02CFD382, 0x4F6DCE82), U64(0x4D67B485, 0x9D95A43E) },
    { U64(0x023FDC68, 0x3F8B0B9B), U64(0x711FC39E, 0x17AAE9CB) },
    { U64(0x039960A6, 0xCC11AC2B), U64(0xE832D296, 0x8C44A945) },
    { U64(0x02E11A1F, 0x09A7BCEF), U64(0xECF57545, 0x3D03BA9E) },
    { U64(0x024DAE7F, 0x3AEC9726), U64(0x572AC437, 0x6402FBB1) },
    { U64(0x03AF7D98, 0x5E47583D), U64(0x58446D25, 0x6CD192B5) },
    { U64(0x02F2CAE0, 0x4B6C4697), U64(0x79D05751, 0x23DADBC4) },
    { U64(0x025BD580, 0x3C569EDF), U64(0x94A6AC40, 0xE97BE303) },
    { U64(0x03C62266, 0xC6F0FE32), U64(0x8771139B, 0x0F2C9E6C) },
    { U64(0x0304E852, 0x38C0CB5B), U64(0x9F8DA948, 0xD8F07EBD) },
    { U64(0x026A5374, 0xFA33D5E2), U64(0xE60AEDD3, 0xE0C06564) },
    { U64(0x03DD5254, 0xC3862304), U64(0xA344AFB9, 0x679A3BD2) },
    { U64(0x03177510, 0x9C6B4F36), U64(0xE903BFC7, 0x8614FCA8) },
    { U64(0x02792A73, 0xB055D8F8), U64(0xBA696639, 0x3810CA20) },
    { U64(0x03F510B9, 0x1A22F4C1), U64(0x2A423D28, 0x59B4769A) },
    { U64(0x032A73C7, 0x481BF700), U64(0xEE9B6420, 0x47C39215) },
    { U64(0x02885C9F, 0x6CE32C00), U64(0xBEE2B680, 0x396941AA) },
    { U64(0x0206B07F, 0x8A4F5666), U64(0xFF1BC533, 0x61210155) },
    { U64(0x033DE732, 0x76E5570B), U64(0x31C60852, 0x35019BBB) },
    { U64(0x0297EC28, 0x5F1DDF3C), U64(0x27D1A041, 0xC4014963) },
    { U64(0x02132353, 0x7F4B18FC), U64(0xECA7B367, 0xD0010782) },
    { U64(0x0351D21F, 0x3211C194), U64(0xADD91F0C, 0x8001A59D) },
    { U64(0x02A7DB4C, 0x280E3476), U64(0xF17A7F3D, 0x3334847E) },
    { U64(0x021FE2A3, 0x533E905F), U64(0x27953297, 0x5C2A0398) },
    { U64(0x0366376B, 0xB8641A31), U64(0xD8EEB758, 0x93766C26) },
    { U64(0x02B82C56, 0x2D1CE1C1), U64(0x7A5892AD, 0x42C52352) },
    { U64(0x022CF044, 0xF0E3E7CD), U64(0xFB7A0EF1, 0x02374F75) },
    { U64(0x037B1A07, 0xE7D30C7C), U64(0xC59017E8, 0x038BB254) },
    { U64(0x02C8E19F, 0xECA8D6CA), U64(0x37A67986, 0x693C8EAA) },
    { U64(0x023A4E19, 0x8A20ABD4), U64(0xF951FAD1, 0xEDCA0BBB) },
    { U64(0x03907CF5, 0xA9CDDFBB), U64(0x28832AE9, 0x7C76792B) },
    { U64(0x02D9FD91, 0x54A4B2FC), U64(0x2068EF21, 0x305EC756) },
    { U64(0x0247FE0D, 0xDD508F30), U64(0x19ED8C1A, 0x8D189F78) },
    { U64(0x03A66349, 0x621A7EB3), U64(0x5CAF4690, 0xE1C0FF26) },
    { U64(0x02EB82A1, 0x1B48655C), U64(0x4A25D20D, 0x81673285) },
    { U64(0x0256021A, 0x7C39EAB0), U64(0x3B5174D7, 0x9AB8F537) },
    { U64(0x03BCD02A, 0x605CAAB3), U64(0x921BEE25, 0xC45B21F1) },
    { U64(0x02FD7355, 0x19E3BBC2), U64(0xDB498B51, 0x69E2818E) },
    { U64(0x02645C44, 0x14B62FCF), U64(0x15D46F74, 0x54B53472) },
    { U64(0x03D3C6D3, 0x5456B2E4), U64(0xEFBA4BED, 0x545520B6) },
    { U64(0x030FD242, 0xA9DEF583), U64(0xF2FB6FF1, 0x10441A2B) },
    { U64(0x02730E9B, 0xBB18C469), U64(0x8F2F8CC0, 0xD9D014EF) },
    { U64(0x03EB4A92, 0xC4F46D75), U64(0xB1E5AE01, 0x5C80217F) },
    { U64(0x0322A20F, 0x03F6BDF7), U64(0xC1848B34, 0x4A001ACC) },
    { U64(0x02821B3F, 0x365EFE5F), U64(0xCE03A290, 0x3B3348A3) },
    { U64(0x0201AF65, 0xC518CB7F), U64(0xD802E873, 0x628F6D4F) },
    { U64(0x0335E56F, 0xA1C14599), U64(0x599E40B8, 0x9DB2487F) },
    { U64(0x02918459, 0x4E3437AD), U64(0xE14B66FA, 0x17C1D399) },
    { U64(0x020E037A, 0xA4F692F1), U64(0x81091F2E, 0x7967DC7A) },
    { U64(0x03499F2A, 0xA18A84B5), U64(0x9B41CB7D, 0x8F0C93F6) },
    { U64(0x02A14C22, 0x1AD536F7), U64(0xAF67D5FE, 0x0C0A0FF8) },
    { U64(0x021AA34E, 0x7BDDC592), U64(0xF2B977FE, 0x70080CC7) },
    { U64(0x035DD217, 0x2C9608EB), U64(0x1DF58CCA, 0x4CD9AE0B) },
    { U64(0x02B174DF, 0x56DE6D88), U64(0xE4C470A1, 0xD7148B3C) },
    { U64(0x022790B2, 0xABE5246D), U64(0x83D05A1B, 0x1276D5CA) },
    { U64(0x0372811D, 0xDFD50715), U64(0x9FB3C35E, 0x83F1560F) },
    { U64(0x02C200E4, 0xB310D277), U64(0xB2F635E5, 0x365AAB3F) },
    { U64(0x0234CD83, 0xC273DB92), U64(0xF591C4B7, 0x5EAEEF66) },
    { U64(0x0387AF39, 0x371FC5B7), U64(0xEF4FA125, 0x644B18A3) },
    { U64(0x02D2F294, 0x2C196AF9), U64(0x8C3FB41D, 0xE9D5AD4F) },
    { U64(0x02425BA9, 0xBCE12261), U64(0x3CFFC34B, 0x2177BDD9) },
    { U64(0x039D5F75, 0xFB01D09B), U64(0x94CC6BAB, 0x68BF9628) },
    { U64(0x02E44C5E, 0x6267DA16), U64(0x10A38955, 0xED6611B9) },
    { U64(0x02503D18, 0x4EB97B44), U64(0xDA1C6DDE, 0x5784DAFB) },
    { U64(0x03B394F3, 0xB128C53A), U64(0xF693E2FD, 0x58D49191) },
    { U64(0x02F610C2, 0xF4209DC8), U64(0xC5431BFD, 0xE0AA0E0E) },
    { U64(0x025E73CF, 0x29B3B16D), U64(0x6A9C1664, 0xB3BB3E72) },
    { U64(0x03CA52E5, 0x0F85E8AF), U64(0x10F9BD6D, 0xEC5ECA4F) },
    { U64(0x03084250, 0xD937ED58), U64(0xDA616457, 0xF04BD50C) },
    { U64(0x026D01DA, 0x475FF113), U64(0xE1E78379, 0x8D09773D) },
    { U64(0x03E19C90, 0x72331B53), U64(0x030C058F, 0x480F252E) },
    { U64(0x031AE3A6, 0xC1C27C42), U64(0x68D66AD9, 0x06728425) },
    { U64(0x027BE952, 0x349B969B), U64(0x8711EF14, 0x052869B7) },
    { U64(0x03F97550, 0x542C242C), U64(0x0B4FE4EC, 0xD50D75F2) },
    { U64(0x032DF773, 0x7689B689), U64(0xA2A650BD, 0x773DF7F5) },
    { U64(0x028B2C5C, 0x5ED49207), U64(0xB551DA31, 0x2C31932A) },
    { U64(0x0208F049, 0xE576DB39), U64(0x5DDB14F4, 0x235ADC22) },
    { U64(0x03418076, 0x3BF15EC2), U64(0x2FC4EE53, 0x6BC49369) },
    { U64(0x029ACD2B, 0x63277F01), U64(0xBFD0BEA9, 0x2303A921) },
    { U64(0x021570EF, 0x8285FF34), U64(0x9973CBBA, 0x8269541A) },
    { U64(0x0355817F, 0x373CCB87), U64(0x5BEC792A, 0x6A42202A) },
    { U64(0x02AACDFF, 0x5F63D605), U64(0xE3239421, 0xEE9B4CEF) },
    { U64(0x02223E65, 0xE5E97804), U64(0xB5B6101B, 0x25490A59) },
    { U64(0x0369FD6F, 0xD64259A1), U64(0x22BCE691, 0xD541AA27) },
    { U64(0x02BB3126, 0x4501E14D), U64(0xB563EBA7, 0xDDCE21B9) },
    { U64(0x022F5A85, 0x0401810A), U64(0xF78322EC, 0xB171B494) },
    { U64(0x037EF73B, 0x399C01AB), U64(0x259E9E47, 0x824F8753) },
    { U64(0x02CBF8FC, 0x2E1667BC), U64(0x1E187E9F, 0x9B72D2A9) },
    { U64(0x023CC730, 0x24DEB963), U64(0x4B46CBB2, 0xE2C24221) },
    { U64(0x039471E6, 0xA1645BD2), U64(0x120ADF84, 0x9E039D01) },
    { U64(0x02DD27EB, 0xB4504974), U64(0xDB3BE603, 0xB19C7D9A) },
    { U64(0x024A8656, 0x29D9D45D), U64(0x7C2FEB36, 0x27B0647C) },
    { U64(0x03AA7089, 0xDC8FBA2F), U64(0x2D197856, 0xA5E7072C) },
    { U64(0x02EEC06E, 0x4A0C94F2), U64(0x8A7AC6AB, 0xB7EC05BD) },
    { U64(0x025899F1, 0xD4D6DD8E), U64(0xD52F0556, 0x2CBCD164) },
    { U64(0x03C0F64F, 0xBAF1627E), U64(0x21E4D556, 0xADFAE8A0) },
    { U64(0x0300C50C, 0x958DE864), U64(0xE7EA4445, 0x57FBED4D) },
    { U64(0x0267040A, 0x113E5383), U64(0xECBB69D1, 0x132FF10A) },
    { U64(0x03D80676, 0x81FD526C), U64(0xADF8A94E, 0x851981AA) },
    { U64(0x0313385E, 0xCE6441F0), U64(0x8B2D543E, 0xD0E13488) },
    { U64(0x0275C6B2, 0x3EB69B26), U64(0xD5BDDCFF, 0x0D80F6D3) },
    { U64(0x03EFA450, 0x64575EA4), U64(0x892FC7FE, 0x7C018AEB) },
    { U64(0x03261D0D, 0x1D12B21D), U64(0x3A8C9FFE, 0xC99AD589) },
    { U64(0x0284E40A, 0x7DA88E7D), U64(0xC8707FFF, 0x07AF113B) },
    { U64(0x0203E9A1, 0xFE2071FE), U64(0x39F39998, 0xD2F2742F) },
    { U64(0x033975CF, 0xFD00B663), U64(0x8FEC28F4, 0x84B7204B) },
    { U64(0x02945E3F, 0xFD9A2B82), U64(0xD989BA5D, 0x36F8E6A2) },
    { U64(0x02104B66, 0x647B5602), U64(0x47A161E4, 0x2BFA521C) },
    { U64(0x034D4570, 0xA0C5566A), U64(0x0C35696D, 0x132A1CF9) },
    { U64(0x02A4378D, 0x4D6AAB88), U64(0x09C45457, 0x4288172D) },
    { U64(0x021CF93D, 0xD7888939), U64(0xA169DD12, 0x9BA0128B) },
    { U64(0x03618EC9, 0x58DA7529), U64(0x0242FB50, 0xF9001DAB) },
    { U64(0x02B4723A, 0xAD7B90ED), U64(0x9B68C90D, 0x940017BC) },
    { U64(0x0229F4FB, 0xBDFC73F1), U64(0x4920A0D7, 0xA999AC96) },
    { U64(0x037654C5, 0xFCC71FE8), U64(0x75010159, 0x0F5C4757) },
    { U64(0x02C5109E, 0x63D27FED), U64(0x2A673447, 0x3F7D05DF) },
    { U64(0x0237407E, 0xB641FFF0), U64(0xEEB8F69F, 0x65FD9E4C) },
    { U64(0x038B9A64, 0x56CFFFE7), U64(0xE45B2432, 0x3CC8FD46) },
    { U64(0x02D6151D, 0x123FFFEC), U64(0xB6AF5028, 0x30A0CA9F) },
    { U64(0x0244DDB0, 0xDB666656), U64(0xF88C4020, 0x26E7087F) },
    { U64(0x03A162B4, 0x923D708B), U64(0x2746CD00, 0x3E3E73FE) },
    { U64(0x02E7822A, 0x0E978D3C), U64(0x1F6BD733, 0x64FEC332) },
    { U64(0x0252CE88, 0x0BAC70FC), U64(0xE5EFDF5C, 0x50CBCF5B) },
    { U64(0x03B7B0D9, 0xAC471B2E), U64(0x3CB2FEFA, 0x1ADFB22B) },
    { U64(0x02F95A47, 0xBD05AF58), U64(0x308F3261, 0xAF195B56) },
    { U64(0x02611506, 0x30D15913), U64(0x5A0C284E, 0x25ADE2AB) },
    { U64(0x03CE8809, 0xE7B55B52), U64(0x29AD0D49, 0xD5E30445) },
    { U64(0x030BA007, 0xEC9115DB), U64(0x548A7107, 0xDE4F369D) },
    { U64(0x026FB339, 0x8A0DAB15), U64(0xDD3B8D9F, 0xE50C2BB1) },
    { U64(0x03E5EB8F, 0x434911BC), U64(0x952C15CC, 0xA1AD12B5) },
    { U64(0x031E560C, 0x35D40E30), U64(0x775677D6, 0xE7BDA891) }
};

/** The significant bits kept in pow5_sig_table. */
#define POW5_SIG_BITS 121

/** The significant bits table of pow5.
    (generate with yy_make_tables.c) */
static const u64 pow5_sig_table[326][2] = {
    { U64(0x01000000, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x01400000, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x01900000, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x01F40000, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x01388000, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x0186A000, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x01E84800, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x01312D00, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x017D7840, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x01DCD650, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x012A05F2, 0x00000000), U64(0x00000000, 0x00000000) },
    { U64(0x0174876E, 0x80000000), U64(0x00000000, 0x00000000) },
    { U64(0x01D1A94A, 0x20000000), U64(0x00000000, 0x00000000) },
    { U64(0x012309CE, 0x54000000), U64(0x00000000, 0x00000000) },
    { U64(0x016BCC41, 0xE9000000), U64(0x00000000, 0x00000000) },
    { U64(0x01C6BF52, 0x63400000), U64(0x00000000, 0x00000000) },
    { U64(0x011C3793, 0x7E080000), U64(0x00000000, 0x00000000) },
    { U64(0x01634578, 0x5D8A0000), U64(0x00000000, 0x00000000) },
    { U64(0x01BC16D6, 0x74EC8000), U64(0x00000000, 0x00000000) },
    { U64(0x01158E46, 0x0913D000), U64(0x00000000, 0x00000000) },
    { U64(0x015AF1D7, 0x8B58C400), U64(0x00000000, 0x00000000) },
    { U64(0x01B1AE4D, 0x6E2EF500), U64(0x00000000, 0x00000000) },
    { U64(0x010F0CF0, 0x64DD5920), U64(0x00000000, 0x00000000) },
    { U64(0x0152D02C, 0x7E14AF68), U64(0x00000000, 0x00000000) },
    { U64(0x01A78437, 0x9D99DB42), U64(0x00000000, 0x00000000) },
    { U64(0x0108B2A2, 0xC2802909), U64(0x40000000, 0x00000000) },
    { U64(0x014ADF4B, 0x7320334B), U64(0x90000000, 0x00000000) },
    { U64(0x019D971E, 0x4FE8401E), U64(0x74000000, 0x00000000) },
    { U64(0x01027E72, 0xF1F12813), U64(0x08800000, 0x00000000) },
    { U64(0x01431E0F, 0xAE6D7217), U64(0xCAA00000, 0x00000000) },
    { U64(0x0193E593, 0x9A08CE9D), U64(0xBD480000, 0x00000000) },
    { U64(0x01F8DEF8, 0x808B0245), U64(0x2C9A0000, 0x00000000) },
    { U64(0x013B8B5B, 0x5056E16B), U64(0x3BE04000, 0x00000000) },
    { U64(0x018A6E32, 0x246C99C6), U64(0x0AD85000, 0x00000000) },
    { U64(0x01ED09BE, 0xAD87C037), U64(0x8D8E6400, 0x00000000) },
    { U64(0x01342617, 0x2C74D822), U64(0xB878FE80, 0x00000000) },
    { U64(0x01812F9C, 0xF7920E2B), U64(0x66973E20, 0x00000000) },
    { U64(0x01E17B84, 0x357691B6), U64(0x403D0DA8, 0x00000000) },
    { U64(0x012CED32, 0xA16A1B11), U64(0xE8262889, 0x00000000) },
    { U64(0x0178287F, 0x49C4A1D6), U64(0x622FB2AB, 0x40000000) },
    { U64(0x01D6329F, 0x1C35CA4B), U64(0xFABB9F56, 0x10000000) },
    { U64(0x0125DFA3, 0x71A19E6F), U64(0x7CB54395, 0xCA000000) },
    { U64(0x016F578C, 0x4E0A060B), U64(0x5BE2947B, 0x3C800000) },
    { U64(0x01CB2D6F, 0x618C878E), U64(0x32DB399A, 0x0BA00000) },
    { U64(0x011EFC65, 0x9CF7D4B8), U64(0xDFC90400, 0x47440000) },
    { U64(0x0166BB7F, 0x0435C9E7), U64(0x17BB4500, 0x59150000) },
    { U64(0x01C06A5E, 0xC5433C60), U64(0xDDAA1640, 0x6F5A4000) },
    { U64(0x0118427B, 0x3B4A05BC), U64(0x8A8A4DE8, 0x45986800) },
    { U64(0x015E531A, 0x0A1C872B), U64(0xAD2CE162, 0x56FE8200) },
    { U64(0x01B5E7E0, 0x8CA3A8F6), U64(0x987819BA, 0xECBE2280) },
    { U64(0x0111B0EC, 0x57E6499A), U64(0x1F4B1014, 0xD3F6D590) },
    { U64(0x01561D27, 0x6DDFDC00), U64(0xA71DD41A, 0x08F48AF4) },
    { U64(0x01ABA471, 0x4957D300), U64(0xD0E54920, 0x8B31ADB1) },
    { U64(0x010B46C6, 0xCDD6E3E0), U64(0x828F4DB4, 0x56FF0C8E) },
    { U64(0x014E1878, 0x814C9CD8), U64(0xA3332121, 0x6CBECFB2) },
    { U64(0x01A19E96, 0xA19FC40E), U64(0xCBFFE969, 0xC7EE839E) },
    { U64(0x0105031E, 0x2503DA89), U64(0x3F7FF1E2, 0x1CF51243) },
    { U64(0x014643E5, 0xAE44D12B), U64(0x8F5FEE5A, 0xA43256D4) },
    { U64(0x0197D4DF, 0x19D60576), U64(0x7337E9F1, 0x4D3EEC89) },
    { U64(0x01FDCA16, 0xE04B86D4), U64(0x1005E46D, 0xA08EA7AB) },
    { U64(0x013E9E4E, 0x4C2F3444), U64(0x8A03AEC4, 0x845928CB) },
    { U64(0x018E45E1, 0xDF3B0155), U64(0xAC849A75, 0xA56F72FD) },
    { U64(0x01F1D75A, 0x5709C1AB), U64(0x17A5C113, 0x0ECB4FBD) },
    { U64(0x01372698, 0x7666190A), U64(0xEEC798AB, 0xE93F11D6) },
    { U64(0x0184F03E, 0x93FF9F4D), U64(0xAA797ED6, 0xE38ED64B) },
    { U64(0x01E62C4E, 0x38FF8721), U64(0x1517DE8C, 0x9C728BDE) },
    { U64(0x012FDBB0, 0xE39FB474), U64(0xAD2EEB17, 0xE1C7976B) },
    { U64(0x017BD29D, 0x1C87A191), U64(0xD87AA5DD, 0xDA397D46) },
    { U64(0x01DAC744, 0x63A989F6), U64(0x4E994F55, 0x50C7DC97) },
    { U64(0x0128BC8A, 0xBE49F639), U64(0xF11FD195, 0x527CE9DE) },
    { U64(0x0172EBAD, 0x6DDC73C8), U64(0x6D67C5FA, 0xA71C2456) },
    { U64(0x01CFA698, 0xC95390BA), U64(0x88C1B779, 0x50E32D6C) },
    { U64(0x0121C81F, 0x7DD43A74), U64(0x957912AB, 0xD28DFC63) },
    { U64(0x016A3A27, 0x5D494911), U64(0xBAD75756, 0xC7317B7C) },
    { U64(0x01C4C8B1, 0x349B9B56), U64(0x298D2D2C, 0x78FDDA5B) },
    { U64(0x011AFD6E, 0xC0E14115), U64(0xD9F83C3B, 0xCB9EA879) },
    { U64(0x0161BCCA, 0x7119915B), U64(0x50764B4A, 0xBE865297) },
    { U64(0x01BA2BFD, 0x0D5FF5B2), U64(0x2493DE1D, 0x6E27E73D) },
    { U64(0x01145B7E, 0x285BF98F), U64(0x56DC6AD2, 0x64D8F086) },
    { U64(0x0159725D, 0xB272F7F3), U64(0x2C938586, 0xFE0F2CA8) },
    { U64(0x01AFCEF5, 0x1F0FB5EF), U64(0xF7B866E8, 0xBD92F7D2) },
    { U64(0x010DE159, 0x3369D1B5), U64(0xFAD34051, 0x767BDAE3) },
    { U64(0x015159AF, 0x80444623), U64(0x79881065, 0xD41AD19C) },
    { U64(0x01A5B01B, 0x605557AC), U64(0x57EA147F, 0x49218603) },
    { U64(0x01078E11, 0x1C3556CB), U64(0xB6F24CCF, 0x8DB4F3C1) },
    { U64(0x01497195, 0x6342AC7E), U64(0xA4AEE003, 0x712230B2) },
    { U64(0x019BCDFA, 0xBC13579E), U64(0x4DDA9804, 0x4D6ABCDF) },
    { U64(0x010160BC, 0xB58C16C2), U64(0xF0A89F02, 0xB062B60B) },
    { U64(0x0141B8EB, 0xE2EF1C73), U64(0xACD2C6C3, 0x5C7B638E) },
    { U64(0x01922726, 0xDBAAE390), U64(0x98077874, 0x339A3C71) },
    { U64(0x01F6B0F0, 0x92959C74), U64(0xBE095691, 0x4080CB8E) },
    { U64(0x013A2E96, 0x5B9D81C8), U64(0xF6C5D61A, 0xC8507F38) },
    { U64(0x0188BA3B, 0xF284E23B), U64(0x34774BA1, 0x7A649F07) },
    { U64(0x01EAE8CA, 0xEF261ACA), U64(0x01951E89, 0xD8FDC6C8) },
    { U64(0x0132D17E, 0xD577D0BE), U64(0x40FD3316, 0x279E9C3D) },
    { U64(0x017F85DE, 0x8AD5C4ED), U64(0xD13C7FDB, 0xB186434C) },
    { U64(0x01DF6756, 0x2D8B3629), U64(0x458B9FD2, 0x9DE7D420) },
    { U64(0x012BA095, 0xDC7701D9), U64(0xCB7743E3, 0xA2B0E494) },
    { U64(0x017688BB, 0x5394C250), U64(0x3E5514DC, 0x8B5D1DB9) },
    { U64(0x01D42AEA, 0x2879F2E4), U64(0x4DEA5A13, 0xAE346527) },
    { U64(0x01249AD2, 0x594C37CE), U64(0xB0B2784C, 0x4CE0BF38) },
    { U64(0x016DC186, 0xEF9F45C2), U64(0x5CDF165F, 0x6018EF06) },
    { U64(0x01C931E8, 0xAB871732), U64(0xF416DBF7, 0x381F2AC8) },
    { U64(0x011DBF31, 0x6B346E7F), U64(0xD88E497A, 0x83137ABD) },
    { U64(0x01652EFD, 0xC6018A1F), U64(0xCEB1DBD9, 0x23D8596C) },
    { U64(0x01BE7ABD, 0x3781ECA7), U64(0xC25E52CF, 0x6CCE6FC7) },
    { U64(0x01170CB6, 0x42B133E8), U64(0xD97AF3C1, 0xA40105DC) },
    { U64(0x015CCFE3, 0xD35D80E3), U64(0x0FD9B0B2, 0x0D014754) },
    { U64(0x01B403DC, 0xC834E11B), U64(0xD3D01CDE, 0x90419929) },
    { U64(0x01108269, 0xFD210CB1), U64(0x6462120B, 0x1A28FFB9) },
    { U64(0x0154A304, 0x7C694FDD), U64(0xBD7A968D, 0xE0B33FA8) },
    { U64(0x01A9CBC5, 0x9B83A3D5), U64(0x2CD93C31, 0x58E00F92) },
    { U64(0x010A1F5B, 0x81324665), U64(0x3C07C59E, 0xD78C09BB) },
    { U64(0x014CA732, 0x617ED7FE), U64(0x8B09B706, 0x8D6F0C2A) },
    { U64(0x019FD0FE, 0xF9DE8DFE), U64(0x2DCC24C8, 0x30CACF34) },
    { U64(0x0103E29F, 0x5C2B18BE), U64(0xDC9F96FD, 0x1E7EC180) },
    { U64(0x0144DB47, 0x3335DEEE), U64(0x93C77CBC, 0x661E71E1) },
    { U64(0x01961219, 0x000356AA), U64(0x38B95BEB, 0x7FA60E59) },
    { U64(0x01FB969F, 0x40042C54), U64(0xC6E7B2E6, 0x5F8F91EF) },
    { U64(0x013D3E23, 0x88029BB4), U64(0xFC50CFCF, 0xFBB9BB35) },
    { U64(0x018C8DAC, 0x6A0342A2), U64(0x3B6503C3, 0xFAA82A03) },
    { U64(0x01EFB117, 0x8484134A), U64(0xCA3E44B4, 0xF9523484) },
    { U64(0x0135CEAE, 0xB2D28C0E), U64(0xBE66EAF1, 0x1BD360D2) },
    { U64(0x0183425A, 0x5F872F12), U64(0x6E00A5AD, 0x62C83907) },
    { U64(0x01E412F0, 0xF768FAD7), U64(0x0980CF18, 0xBB7A4749) },
    { U64(0x012E8BD6, 0x9AA19CC6), U64(0x65F0816F, 0x752C6C8D) },
    { U64(0x017A2ECC, 0x414A03F7), U64(0xFF6CA1CB, 0x527787B1) },
    { U64(0x01D8BA7F, 0x519C84F5), U64(0xFF47CA3E, 0x2715699D) },
    { U64(0x0127748F, 0x9301D319), U64(0xBF8CDE66, 0xD86D6202) },
    { U64(0x017151B3, 0x77C247E0), U64(0x2F701600, 0x8E88BA83) },
    { U64(0x01CDA620, 0x55B2D9D8), U64(0x3B4C1B80, 0xB22AE923) },
    { U64(0x012087D4, 0x358FC827), U64(0x250F9130, 0x6F5AD1B6) },
    { U64(0x0168A9C9, 0x42F3BA30), U64(0xEE53757C, 0x8B318623) },
    { U64(0x01C2D43B, 0x93B0A8BD), U64(0x29E852DB, 0xADFDE7AC) },
    { U64(0x0119C4A5, 0x3C4E6976), U64(0x3A3133C9, 0x4CBEB0CC) },
    { U64(0x016035CE, 0x8B6203D3), U64(0xC8BD80BB, 0x9FEE5CFF) },
    { U64(0x01B84342, 0x2E3A84C8), U64(0xBAECE0EA, 0x87E9F43E) },
    { U64(0x01132A09, 0x5CE492FD), U64(0x74D40C92, 0x94F238A7) },
    { U64(0x0157F48B, 0xB41DB7BC), U64(0xD2090FB7, 0x3A2EC6D1) },
    { U64(0x01ADF1AE, 0xA12525AC), U64(0x068B53A5, 0x08BA7885) },
    { U64(0x010CB70D, 0x24B7378B), U64(0x84171447, 0x25748B53) },
    { U64(0x014FE4D0, 0x6DE5056E), U64(0x651CD958, 0xEED1AE28) },
    { U64(0x01A3DE04, 0x895E46C9), U64(0xFE640FAF, 0x2A8619B2) },
    { U64(0x01066AC2, 0xD5DAEC3E), U64(0x3EFE89CD, 0x7A93D00F) },
    { U64(0x01480573, 0x8B51A74D), U64(0xCEBE2C40, 0xD938C413) },
    { U64(0x019A06D0, 0x6E261121), U64(0x426DB751, 0x0F86F518) },
    { U64(0x01004442, 0x44D7CAB4), U64(0xC9849292, 0xA9B4592F) },
    { U64(0x01405552, 0xD60DBD61), U64(0xFBE5B737, 0x54216F7A) },
    { U64(0x01906AA7, 0x8B912CBA), U64(0x7ADF2505, 0x2929CB59) },
    { U64(0x01F48551, 0x6E7577E9), U64(0x1996EE46, 0x73743E2F) },
    { U64(0x0138D352, 0xE5096AF1), U64(0xAFFE54EC, 0x0828A6DD) },
    { U64(0x01870827, 0x9E4BC5AE), U64(0x1BFDEA27, 0x0A32D095) },
    { U64(0x01E8CA31, 0x85DEB719), U64(0xA2FD64B0, 0xCCBF84BA) },
    { U64(0x01317E5E, 0xF3AB3270), U64(0x05DE5EEE, 0x7FF7B2F4) },
    { U64(0x017DDDF6, 0xB095FF0C), U64(0x0755F6AA, 0x1FF59FB1) },
    { U64(0x01DD5574, 0x5CBB7ECF), U64(0x092B7454, 0xA7F3079E) },
    { U64(0x012A5568, 0xB9F52F41), U64(0x65BB28B4, 0xE8F7E4C3) },
    { U64(0x0174EAC2, 0xE8727B11), U64(0xBF29F2E2, 0x2335DDF3) },
    { U64(0x01D22573, 0xA28F19D6), U64(0x2EF46F9A, 0xAC035570) },
    { U64(0x01235768, 0x45997025), U64(0xDD58C5C0, 0xAB821566) },
    { U64(0x016C2D42, 0x56FFCC2F), U64(0x54AEF730, 0xD6629AC0) },
    { U64(0x01C73892, 0xECBFBF3B), U64(0x29DAB4FD, 0x0BFB4170) },
    { U64(0x011C835B, 0xD3F7D784), U64(0xFA28B11E, 0x277D08E6) },
    { U64(0x0163A432, 0xC8F5CD66), U64(0x38B2DD65, 0xB15C4B1F) },
    { U64(0x01BC8D3F, 0x7B3340BF), U64(0xC6DF94BF, 0x1DB35DE7) },
    { U64(0x0115D847, 0xAD000877), U64(0xDC4BBCF7, 0x72901AB0) },
    { U64(0x015B4E59, 0x98400A95), U64(0xD35EAC35, 0x4F34215C) },
    { U64(0x01B221EF, 0xFE500D3B), U64(0x48365742, 0xA30129B4) },
    { U64(0x010F5535, 0xFEF20845), U64(0x0D21F689, 0xA5E0BA10) },
    { U64(0x01532A83, 0x7EAE8A56), U64(0x506A742C, 0x0F58E894) },
    { U64(0x01A7F524, 0x5E5A2CEB), U64(0xE4851137, 0x132F22B9) },
    { U64(0x0108F936, 0xBAF85C13), U64(0x6ED32AC2, 0x6BFD75B4) },
    { U64(0x014B3784, 0x69B67318), U64(0x4A87F573, 0x06FCD321) },
    { U64(0x019E0565, 0x84240FDE), U64(0x5D29F2CF, 0xC8BC07E9) },
    { U64(0x0102C35F, 0x729689EA), U64(0xFA3A37C1, 0xDD7584F1) },
    { U64(0x01437437, 0x4F3C2C65), U64(0xB8C8C5B2, 0x54D2E62E) },
    { U64(0x01945145, 0x230B377F), U64(0x26FAF71E, 0xEA079FB9) },
    { U64(0x01F96596, 0x6BCE055E), U64(0xF0B9B4E6, 0xA48987A8) },
    { U64(0x013BDF7E, 0x0360C35B), U64(0x56741110, 0x26D5F4C9) },
    { U64(0x018AD75D, 0x8438F432), U64(0x2C111554, 0x308B71FB) },
    { U64(0x01ED8D34, 0xE547313E), U64(0xB7155AA9, 0x3CAE4E7A) },
    { U64(0x01347841, 0x0F4C7EC7), U64(0x326D58A9, 0xC5ECF10C) },
    { U64(0x01819651, 0x531F9E78), U64(0xFF08AED4, 0x37682D4F) },
    { U64(0x01E1FBE5, 0xA7E78617), U64(0x3ECADA89, 0x454238A3) },
    { U64(0x012D3D6F, 0x88F0B3CE), U64(0x873EC895, 0xCB496366) },
    { U64(0x01788CCB, 0x6B2CE0C2), U64(0x290E7ABB, 0x3E1BBC3F) },
    { U64(0x01D6AFFE, 0x45F818F2), U64(0xB352196A, 0x0DA2AB4F) },
    { U64(0x01262DFE, 0xEBBB0F97), U64(0xB0134FE2, 0x4885AB11) },
    { U64(0x016FB97E, 0xA6A9D37D), U64(0x9C1823DA, 0xDAA715D6) },
    { U64(0x01CBA7DE, 0x5054485D), U64(0x031E2CD1, 0x9150DB4B) },
    { U64(0x011F48EA, 0xF234AD3A), U64(0x21F2DC02, 0xFAD2890F) },
    { U64(0x01671B25, 0xAEC1D888), U64(0xAA6F9303, 0xB9872B53) },
    { U64(0x01C0E1EF, 0x1A724EAA), U64(0xD50B77C4, 0xA7E8F628) },
    { U64(0x01188D35, 0x7087712A), U64(0xC5272ADA, 0xE8F199D9) },
    { U64(0x015EB082, 0xCCA94D75), U64(0x7670F591, 0xA32E004F) },
    { U64(0x01B65CA3, 0x7FD3A0D2), U64(0xD40D32F6, 0x0BF98063) },
    { U64(0x0111F9E6, 0x2FE44483), U64(0xC4883FD9, 0xC77BF03E) },
    { U64(0x0156785F, 0xBBDD55A4), U64(0xB5AA4FD0, 0x395AEC4D) },
    { U64(0x01AC1677, 0xAAD4AB0D), U64(0xE314E3C4, 0x47B1A760) },
    { U64(0x010B8E0A, 0xCAC4EAE8), U64(0xADED0E5A, 0xACCF089C) },
    { U64(0x014E718D, 0x7D7625A2), U64(0xD96851F1, 0x5802CAC3) },
    { U64(0x01A20DF0, 0xDCD3AF0B), U64(0x8FC2666D, 0xAE037D74) },
    { U64(0x010548B6, 0x8A044D67), U64(0x39D98004, 0x8CC22E68) },
    { U64(0x01469AE4, 0x2C8560C1), U64(0x084FE005, 0xAFF2BA03) },
    { U64(0x0198419D, 0x37A6B8F1), U64(0x4A63D807, 0x1BEF6883) },
    { U64(0x01FE5204, 0x8590672D), U64(0x9CFCCE08, 0xE2EB42A4) },
    { U64(0x013EF342, 0xD37A407C), U64(0x821E00C5, 0x8DD309A7) },
    { U64(0x018EB013, 0x8858D09B), U64(0xA2A580F6, 0xF147CC10) },
    { U64(0x01F25C18, 0x6A6F04C2), U64(0x8B4EE134, 0xAD99BF15) },
    { U64(0x0137798F, 0x428562F9), U64(0x97114CC0, 0xEC80176D) },
    { U64(0x018557F3, 0x1326BBB7), U64(0xFCD59FF1, 0x27A01D48) },
    { U64(0x01E6ADEF, 0xD7F06AA5), U64(0xFC0B07ED, 0x7188249A) },
    { U64(0x01302CB5, 0xE6F642A7), U64(0xBD86E4F4, 0x66F516E0) },
    { U64(0x017C37E3, 0x60B3D351), U64(0xACE89E31, 0x80B25C98) },
    { U64(0x01DB45DC, 0x38E0C826), U64(0x1822C5BD, 0xE0DEF3BE) },
    { U64(0x01290BA9, 0xA38C7D17), U64(0xCF15BB96, 0xAC8B5857) },
    { U64(0x01734E94, 0x0C6F9C5D), U64(0xC2DB2A7C, 0x57AE2E6D) },
    { U64(0x01D02239, 0x0F8B8375), U64(0x3391F51B, 0x6D99BA08) },
    { U64(0x01221563, 0xA9B73229), U64(0x403B3931, 0x24801445) },
    { U64(0x016A9ABC, 0x9424FEB3), U64(0x904A077D, 0x6DA01956) },
    { U64(0x01C5416B, 0xB92E3E60), U64(0x745C895C, 0xC9081FAC) },
    { U64(0x011B48E3, 0x53BCE6FC), U64(0x48B9D5D9, 0xFDA513CB) },
    { U64(0x01621B1C, 0x28AC20BB), U64(0x5AE84B50, 0x7D0E58BE) },
    { U64(0x01BAA1E3, 0x32D728EA), U64(0x31A25E24, 0x9C51EEEE) },
    { U64(0x0114A52D, 0xFFC67992), U64(0x5F057AD6, 0xE1B33554) },
    { U64(0x0159CE79, 0x7FB817F6), U64(0xF6C6D98C, 0x9A2002AA) },
    { U64(0x01B04217, 0xDFA61DF4), U64(0xB4788FEF, 0xC0A80354) },
    { U64(0x010E294E, 0xEBC7D2B8), U64(0xF0CB59F5, 0xD8690214) },
    { U64(0x0151B3A2, 0xA6B9C767), U64(0x2CFE3073, 0x4E83429A) },
    { U64(0x01A6208B, 0x50683940), U64(0xF83DBC90, 0x22241340) },
    { U64(0x0107D457, 0x124123C8), U64(0x9B2695DA, 0x15568C08) },
    { U64(0x0149C96C, 0xD6D16CBA), U64(0xC1F03B50, 0x9AAC2F0A) },
    { U64(0x019C3BC8, 0x0C85C7E9), U64(0x726C4A24, 0xC1573ACD) },
    { U64(0x0101A55D, 0x07D39CF1), U64(0xE783AE56, 0xF8D684C0) },
    { U64(0x01420EB4, 0x49C8842E), U64(0x616499EC, 0xB70C25F0) },
    { U64(0x01929261, 0x5C3AA539), U64(0xF9BDC067, 0xE4CF2F6C) },
    { U64(0x01F736F9, 0xB3494E88), U64(0x782D3081, 0xDE02FB47) },
    { U64(0x013A825C, 0x100DD115), U64(0x4B1C3E51, 0x2AC1DD0C) },
    { U64(0x018922F3, 0x1411455A), U64(0x9DE34DE5, 0x7572544F) },
    { U64(0x01EB6BAF, 0xD91596B1), U64(0x455C215E, 0xD2CEE963) },
    { U64(0x0133234D, 0xE7AD7E2E), U64(0xCB5994DB, 0x43C151DE) },
    { U64(0x017FEC21, 0x6198DDBA), U64(0x7E2FFA12, 0x14B1A655) },
    { U64(0x01DFE729, 0xB9FF1529), U64(0x1DBBF896, 0x99DE0FEB) },
    { U64(0x012BF07A, 0x143F6D39), U64(0xB2957B5E, 0x202AC9F3) },
    { U64(0x0176EC98, 0x994F4888), U64(0x1F3ADA35, 0xA8357C6F) },
    { U64(0x01D4A7BE, 0xBFA31AAA), U64(0x270990C3, 0x1242DB8B) },
    { U64(0x0124E8D7, 0x37C5F0AA), U64(0x5865FA79, 0xEB69C937) },
    { U64(0x016E230D, 0x05B76CD4), U64(0xEE7F7918, 0x66443B85) },
    { U64(0x01C9ABD0, 0x4725480A), U64(0x2A1F575E, 0x7FD54A66) },
    { U64(0x011E0B62, 0x2C774D06), U64(0x5A53969B, 0x0FE54E80) },
    { U64(0x01658E3A, 0xB7952047), U64(0xF0E87C41, 0xD3DEA220) },
    { U64(0x01BEF1C9, 0x657A6859), U64(0xED229B52, 0x48D64AA8) },
    { U64(0x0117571D, 0xDF6C8138), U64(0x3435A113, 0x6D85EEA9) },
    { U64(0x015D2CE5, 0x5747A186), U64(0x41430958, 0x48E76A53) },
    { U64(0x01B4781E, 0xAD1989E7), U64(0xD193CBAE, 0x5B2144E8) },
    { U64(0x0110CB13, 0x2C2FF630), U64(0xE2FC5F4C, 0xF8F4CB11) },
    { U64(0x0154FDD7, 0xF73BF3BD), U64(0x1BBB7720, 0x3731FDD5) },
    { U64(0x01AA3D4D, 0xF50AF0AC), U64(0x62AA54E8, 0x44FE7D4A) },
    { U64(0x010A6650, 0xB926D66B), U64(0xBDAA7511, 0x2B1F0E4E) },
    { U64(0x014CFFE4, 0xE7708C06), U64(0xAD151255, 0x75E6D1E2) },
    { U64(0x01A03FDE, 0x214CAF08), U64(0x585A56EA, 0xD360865B) },
    { U64(0x010427EA, 0xD4CFED65), U64(0x37387652, 0xC41C53F8) },
    { U64(0x014531E5, 0x8A03E8BE), U64(0x850693E7, 0x752368F7) },
    { U64(0x01967E5E, 0xEC84E2EE), U64(0x264838E1, 0x526C4334) },
    { U64(0x01FC1DF6, 0xA7A61BA9), U64(0xAFDA4719, 0xA7075402) },
    { U64(0x013D92BA, 0x28C7D14A), U64(0x0DE86C70, 0x08649481) },
    { U64(0x018CF768, 0xB2F9C59C), U64(0x9162878C, 0x0A7DB9A1) },
    { U64(0x01F03542, 0xDFB83703), U64(0xB5BB296F, 0x0D1D280A) },
    { U64(0x01362149, 0xCBD32262), U64(0x5194F9E5, 0x68323906) },
    { U64(0x0183A99C, 0x3EC7EAFA), U64(0xE5FA385E, 0xC23EC747) },
    { U64(0x01E49403, 0x4E79E5B9), U64(0x9F78C676, 0x72CE7919) },
    { U64(0x012EDC82, 0x110C2F94), U64(0x03AB7C0A, 0x07C10BB0) },
    { U64(0x017A93A2, 0x954F3B79), U64(0x04965B0C, 0x89B14E9C) },
    { U64(0x01D9388B, 0x3AA30A57), U64(0x45BBF1CF, 0xAC1DA243) },
    { U64(0x0127C357, 0x04A5E676), U64(0x8B957721, 0xCB92856A) },
    { U64(0x0171B42C, 0xC5CF6014), U64(0x2E7AD4EA, 0x3E7726C4) },
    { U64(0x01CE2137, 0xF7433819), U64(0x3A198A24, 0xCE14F075) },
    { U64(0x0120D4C2, 0xFA8A030F), U64(0xC44FF657, 0x00CD1649) },
    { U64(0x016909F3, 0xB92C83D3), U64(0xB563F3EC, 0xC1005BDB) },
    { U64(0x01C34C70, 0xA777A4C8), U64(0xA2BCF0E7, 0xF14072D2) },
    { U64(0x011A0FC6, 0x68AAC6FD), U64(0x65B61690, 0xF6C847C3) },
    { U64(0x016093B8, 0x02D578BC), U64(0xBF239C35, 0x347A59B4) },
    { U64(0x01B8B8A6, 0x038AD6EB), U64(0xEEEC8342, 0x8198F021) },
    { U64(0x01137367, 0xC236C653), U64(0x7553D209, 0x90FF9615) },
    { U64(0x01585041, 0xB2C477E8), U64(0x52A8C68B, 0xF53F7B9A) },
    { U64(0x01AE6452, 0x1F7595E2), U64(0x6752F82E, 0xF28F5A81) },
    { U64(0x010CFEB3, 0x53A97DAD), U64(0x8093DB1D, 0x57999890) },
    { U64(0x01503E60, 0x2893DD18), U64(0xE0B8D1E4, 0xAD7FFEB4) },
    { U64(0x01A44DF8, 0x32B8D45F), U64(0x18E7065D, 0xD8DFFE62) },
    { U64(0x0106B0BB, 0x1FB384BB), U64(0x6F9063FA, 0xA78BFEFD) },
    { U64(0x01485CE9, 0xE7A065EA), U64(0x4B747CF9, 0x516EFEBC) },
    { U64(0x019A7424, 0x61887F64), U64(0xDE519C37, 0xA5CABE6B) },
    { U64(0x01008896, 0xBCF54F9F), U64(0x0AF301A2, 0xC79EB703) },
    { U64(0x0140AABC, 0x6C32A386), U64(0xCDAFC20B, 0x798664C4) },
    { U64(0x0190D56B, 0x873F4C68), U64(0x811BB28E, 0x57E7FDF5) },
    { U64(0x01F50AC6, 0x690F1F82), U64(0xA1629F31, 0xEDE1FD72) },
    { U64(0x013926BC, 0x01A973B1), U64(0xA4DDA37F, 0x34AD3E67) },
    { U64(0x0187706B, 0x0213D09E), U64(0x0E150C5F, 0x01D88E01) },
    { U64(0x01E94C85, 0xC298C4C5), U64(0x919A4F76, 0xC24EB181) },
    { U64(0x0131CFD3, 0x999F7AFB), U64(0x7B0071AA, 0x39712EF1) },
    { U64(0x017E43C8, 0x800759BA), U64(0x59C08E14, 0xC7CD7AAD) },
    { U64(0x01DDD4BA, 0xA0093028), U64(0xF030B199, 0xF9C0D958) },
    { U64(0x012AA4F4, 0xA405BE19), U64(0x961E6F00, 0x3C1887D7) },
    { U64(0x01754E31, 0xCD072D9F), U64(0xFBA60AC0, 0x4B1EA9CD) },
    { U64(0x01D2A1BE, 0x4048F907), U64(0xFA8F8D70, 0x5DE65440) },
    { U64(0x0123A516, 0xE82D9BA4), U64(0xFC99B866, 0x3AAFF4A8) },
    { U64(0x016C8E5C, 0xA239028E), U64(0x3BC0267F, 0xC95BF1D2) },
    { U64(0x01C7B1F3, 0xCAC74331), U64(0xCAB0301F, 0xBBB2EE47) },
    { U64(0x011CCF38, 0x5EBC89FF), U64(0x1EAE1E13, 0xD54FD4EC) },
    { U64(0x01640306, 0x766BAC7E), U64(0xE659A598, 0xCAA3CA27) },
    { U64(0x01BD03C8, 0x1406979E), U64(0x9FF00EFE, 0xFD4CBCB1) },
    { U64(0x0116225D, 0x0C841EC3), U64(0x23F6095F, 0x5E4FF5EF) },
    { U64(0x015BAAF4, 0x4FA52673), U64(0xECF38BB7, 0x35E3F36A) },
    { U64(0x01B295B1, 0x638E7010), U64(0xE8306EA5, 0x035CF045) },
    { U64(0x010F9D8E, 0xDE39060A), U64(0x911E4527, 0x221A162B) },
    { U64(0x015384F2, 0x95C7478D), U64(0x3565D670, 0xEAA09BB6) },
    { U64(0x01A8662F, 0x3B391970), U64(0x82BF4C0D, 0x2548C2A3) },
    { U64(0x01093FDD, 0x8503AFE6), U64(0x51B78F88, 0x374D79A6) },
    { U64(0x014B8FD4, 0xE6449BDF), U64(0xE625736A, 0x4520D810) },
    { U64(0x019E73CA, 0x1FD5C2D7), U64(0xDFAED044, 0xD6690E14) },
    { U64(0x0103085E, 0x53E599C6), U64(0xEBCD422B, 0x0601A8CC) },
    { U64(0x0143CA75, 0xE8DF0038), U64(0xA6C092B5, 0xC78212FF) },
    { U64(0x0194BD13, 0x6316C046), U64(0xD070B763, 0x396297BF) },
    { U64(0x01F9EC58, 0x3BDC7058), U64(0x848CE53C, 0x07BB3DAF) },
    { U64(0x013C33B7, 0x2569C637), U64(0x52D80F45, 0x84D5068D) },
    { U64(0x018B40A4, 0xEEC437C5), U64(0x278E1316, 0xE60A4831) }
};

/**
 Convert exponent from base 2 to base 10.
    => floor(log10(pow(2, exp)))
    => exp * log10(2)
    => exp * 78913 / 262144; (exp <= 1650)
 */
static_inline u32 exp_base2to10(u32 exp) {
    return (exp * (u32)78913) >> 18;
}

/**
 Convert exponent from base 5 to base 10.
    => floor(log10(pow(5, exp)))
    => exp * log10(5)
    => exp * 183231 / 262144; (exp <= 1650)
 */
static_inline u32 exp_base5to10(u32 exp) {
    return (exp * (u32)183231) >> 18;
}

/**
 Returns bits count of 5^exp.
    => (exp == 0) ? 1 : ceil(log2(pow(5, exp)))
    => (exp == 0) ? 1 : ceil(e * log2(5))
    => (exp * 152170 / 65536) + 1; (exp <= 642)
 */
static_inline u32 exp_base5bits(u32 exp) {
    return ((exp * (u32)152170) >> 16) + 1;
}

/** Returns whether val is divisible by 5^exp (val should not be 0). */
static_inline bool u64_is_divisible_by_pow5(u64 val, u32 exp) {
    while ((i32)exp-- > 0) {
        u64 div = val / 5;
        u64 mod = val - (div * 5);
        if (mod) return (i32)exp < 0;
        val = div;
    }
    return true;
}

/** Returns whether val is divisible by 2^exp (val should not be 0). */
static_inline bool u64_is_divisible_by_pow2(u64 val, u32 exp) {
    return u64_tz_bits(val) >= exp;
}

/** Returns (a * b) >> shr, shr should in range [64, 128]  */
static_inline u64 u128_mul_shr(u64 a, u64 b_hi, u64 b_lo, u64 shr) {
#if YY_HAS_INT128
    u128 r_lo = (u128)a * b_lo;
    u128 r_hi = (u128)a * b_hi;
    return (u64)(((r_lo >> 64) + r_hi) >> (shr - 64));
#else
    u64 r0_hi, r0_lo, r1_hi, r1_lo;
    u128_mul(a, b_lo, &r0_hi, &r0_lo);
    u128_mul_add(a, b_hi, r0_hi, &r1_hi, &r1_lo);
    return (r1_hi << (128 - shr)) | (r1_lo >> (shr - 64));
#endif
}

/** Returns the number of digits in decimal.
    It was used to print floating-point number, the max length should be 17. */
static_inline u32 u64_dec_len(u64 u) {
#if GCC_HAS_CLZLL | MSC_HAS_BIT_SCAN_64 | MSC_HAS_BIT_SCAN
    const u64 powers_of_10[] = {
        (u64)0UL,
        (u64)10UL,
        (u64)100UL,
        (u64)1000UL,
        (u64)10000UL,
        (u64)100000UL,
        (u64)1000000UL,
        (u64)10000000UL,
        (u64)100000000UL,
        (u64)1000000000UL,
        (u64)1000000000UL * 10UL,
        (u64)1000000000UL * 100UL,
        (u64)1000000000UL * 1000UL,
        (u64)1000000000UL * 10000UL,
        (u64)1000000000UL * 100000UL,
        (u64)1000000000UL * 1000000UL,
        (u64)1000000000UL * 10000000UL,
        (u64)1000000000UL * 100000000UL,
        (u64)1000000000UL * 1000000000UL
    };
    u32 t = (64 - u64_lz_bits(u | 1)) * 1233 >> 12;
    return t - (u < powers_of_10[t]) + 1;
#else
    if (u >= (u64)1000000000UL * 10000000UL) return 17;
    if (u >= (u64)1000000000UL * 1000000UL) return 16;
    if (u >= (u64)1000000000UL * 100000UL) return 15;
    if (u >= (u64)1000000000UL * 10000UL) return 14;
    if (u >= (u64)1000000000UL * 1000UL) return 13;
    if (u >= (u64)1000000000UL * 100UL) return 12;
    if (u >= (u64)1000000000UL * 10UL) return 11;
    if (u >= (u64)1000000000UL) return 10;
    if (u >= (u64)100000000UL) return 9;
    if (u >= (u64)10000000UL) return 8;
    if (u >= (u64)1000000UL) return 7;
    if (u >= (u64)100000UL) return 6;
    if (u >= (u64)10000UL) return 5;
    if (u >= (u64)1000UL) return 4;
    if (u >= (u64)100UL) return 3;
    if (u >= (u64)10UL) return 2;
    return 1;
#endif
}

/** Write an unsigned integer with a length of 1 to 17. */
static_inline u8 *write_u64_len_1_17(u64 val, u8 *buf) {
    u64 hgh;
    u32 mid, low, one;
    
    if (val < 100000000) { /* 1-8 digits */
        buf = write_u32_len_1_8((u32)val, buf);
        return buf;
        
    } else if (val < (u64)100000000 * 100000000) { /* 9-16 digits */
        hgh = val / 100000000;
        low = (u32)(val - hgh * 100000000); /* (val % 100000000) */
        buf = write_u32_len_1_8((u32)hgh, buf);
        buf = write_u32_len_8(low, buf);
        return buf;
        
    } else { /* 17 digits */
        hgh = val / 100000000;
        low = (u32)(val - hgh * 100000000); /* (val % 100000000) */
        one = (u32)(hgh / 100000000);
        mid = (u32)(hgh - (u64)one * 100000000); /* (hgh % 100000000) */
        *buf++ = (u8)one + (u8)'0';
        buf = write_u32_len_8(mid, buf);
        buf = write_u32_len_8(low, buf);
        return buf;
    }
}

/**  Write mutiple '0', count should in range 0 to 20. */
static_inline u8 *write_zeros(u8 *cur, u32 count) {
    u8 *end = cur + count;
    *(v32 *)&cur[0] = v32_make('0','0','0','0');
    if (count <= 4) return end;
    *(v32 *)&cur[4] = v32_make('0','0','0','0');
    if (count <= 8) return end;
    *(v32 *)&cur[8] = v32_make('0','0','0','0');
    if (count <= 12) return end;
    *(v32 *)&cur[12] = v32_make('0','0','0','0');
    if (count <= 16) return end;
    *(v32 *)&cur[16] = v32_make('0','0','0','0');
    return end;
}

/** Write a signed integer in the range -324 to 308. */
static_inline u8 *write_f64_exp(i32 exp, u8 *buf) {
    buf[0] = '-';
    buf += exp < 0;
    exp = exp < 0 ? -exp : exp;
    if (exp < 100) {
        u32 lz = exp < 10;
        *(v16 *)&buf[0] = *(v16 *)&digit_table[exp * 2 + lz];
        return buf - lz + 2;
    } else {
        u32 hi = (exp * 656) >> 16; /* exp / 100 */
        u32 lo = exp - hi * 100; /* exp % 100 */
        buf[0] = (u8)hi + (u8)'0';
        *(v16 *)&buf[1] = *(v16 *)&digit_table[lo * 2];
        return buf + 3;
    }
}

/**
 Convert double number from binary to a shortest decimal representation.
 
 This algorithm refers to Ulf Adams's Ryu algorithm.
 Paper: https://dl.acm.org/citation.cfm?id=3192369
 Code: https://github.com/ulfjack/ryu
 
 @param sig_raw IEEE-754 significand part in binary.
 @param exp_raw IEEE-754 exponent part in binary.
 @param sig_dec Output shortest significand in decimal.
 @param exp_dec Output exponent in decimal.
 */
static_inline void f64_to_dec(u64 sig_raw, u32 exp_raw,
                              u64 *sig_dec, i32 *exp_dec) {
    i32 exp_bin; /* exponent base 2 */
    u64 sig_bin; /* significand base 2 */
    i32 exp; /* exponent base 10 */
    u64 sig; /* significand base 10 */
    u64 sig_up; /* upper halfway between sig and next ulp */
    u64 sig_lo; /* lower halfway between sig and prev ulp */
    
    u8 last_trim_num = 0;
    u64 sig_up_div;
    u64 sig_lo_div;
    u64 sig_lo_mod;
    u64 sig_div;
    u64 sig_mod;
    
    bool sig_lo_end_zero = false;
    bool sig_end_zero = false;
    bool sig_not_zero = (sig_raw != 0);
    bool accept_bounds = ((sig_raw & 1) == 0);
    
    if (exp_raw != 0) { /* normal */
        exp_bin = (i32)exp_raw - F64_EXP_BIAS - F64_SIG_BITS;
        sig_bin =  sig_raw | ((u64)1 << F64_SIG_BITS);
        
        /* fast path for integer number which does not have decimal part */
        if (-F64_SIG_BITS <= exp_bin && exp_bin <= 0) {
            if (u64_tz_bits(sig_bin) >= (u32)-exp_bin) {
                sig = sig_bin >> -exp_bin;
                exp = 0;
                while (true) {
                    u64 div = sig / 10;
                    if (sig > 10 * div) break;
                    sig = div;
                    exp++;
                }
                *sig_dec = sig;
                *exp_dec = exp;
                return;
            }
        }
        
        exp_bin -= 2;
        sig_bin <<= 2;
    } else { /* subnormal */
        exp_bin = 1 - F64_EXP_BIAS - F64_SIG_BITS - 2;
        sig_bin = sig_raw << 2;
    }
    
    if (exp_bin < 0) {
        i32 e10 = exp_base5to10(-exp_bin) - (exp_bin < -1);
        i32 neg = -(e10 + exp_bin);
        i32 shr = e10 - exp_base5bits(neg) + POW5_SIG_BITS;
        u64 mul_hi = pow5_sig_table[neg][0];
        u64 mul_lo = pow5_sig_table[neg][1];
        u32 sig_up_dist = 2;
        u32 sig_lo_dist = 1 + sig_not_zero;
        sig_up = u128_mul_shr(sig_bin + sig_up_dist, mul_hi, mul_lo, shr);
        sig_lo = u128_mul_shr(sig_bin - sig_lo_dist, mul_hi, mul_lo, shr);
        sig    = u128_mul_shr(sig_bin,               mul_hi, mul_lo, shr);
        exp = -neg;
        if (e10 <= 1) {
            sig_end_zero = true;
            if (accept_bounds) {
                sig_lo_end_zero = sig_not_zero;
            } else {
                --sig_up;
            }
        } else if (e10 < 63) {
            sig_end_zero = u64_is_divisible_by_pow2(sig_bin, e10 - 1);
        }
    } else {
        u32 e10 = exp_base2to10(exp_bin) - (exp_bin > 3);
        i32 shr = -exp_bin + e10 + POW5_INV_SIG_BITS + exp_base5bits(e10) - 1;
        u64 mul_hi = pow5_inv_sig_table[e10][0];
        u64 mul_lo = pow5_inv_sig_table[e10][1];
        u32 sig_up_dist = 2;
        u32 sig_lo_dist = 1 + sig_not_zero;
        sig_up = u128_mul_shr(sig_bin + sig_up_dist, mul_hi, mul_lo, shr);
        sig_lo = u128_mul_shr(sig_bin - sig_lo_dist, mul_hi, mul_lo, shr);
        sig    = u128_mul_shr(sig_bin,               mul_hi, mul_lo, shr);
        exp = e10;
        if (e10 <= 21) {
            if ((sig_bin % 5) == 0) {
                sig_end_zero = u64_is_divisible_by_pow5(sig_bin, e10);
            } else if (accept_bounds) {
                sig_lo_end_zero = u64_is_divisible_by_pow5(sig_bin - 1 -
                                                           sig_not_zero, e10);
            } else {
                sig_up -= u64_is_divisible_by_pow5(sig_bin + 2, e10);
            }
        }
    }
    
    if (!(sig_lo_end_zero | sig_end_zero)) {
        bool round_up = false;
        
#define if_can_trim(div, len) \
        sig_up_div = sig_up / div; \
        sig_lo_div = sig_lo / div; \
        if (sig_up_div > sig_lo_div)
        
#define do_trim(div, len) \
        sig_div = sig / div; \
        sig_mod = sig - div * sig_div; \
        round_up = (sig_mod >= div / 2); \
        sig = sig_div; \
        sig_up = sig_up_div; \
        sig_lo = sig_lo_div; \
        exp += len;
        
        /* optimize for numbers with few digits */
        if_can_trim(((u64)1000000 * 1000000), 12) {
            do_trim(((u64)1000000 * 1000000), 12);
            if_can_trim(10000, 4) {
                do_trim(10000, 4);
            }
        } else {
            if_can_trim(1000000, 6) {
                do_trim(1000000, 6);
            }
        }
        if_can_trim(100, 2) {
            do_trim(100, 2);
        }
        while (true) {
            if_can_trim(10, 1) {
                do_trim(10, 1);
            } else break;
           if_can_trim(10, 1) {
               do_trim(10, 1);
           } else break;
           if_can_trim(10, 1) {
               do_trim(10, 1);
           } else break;
           break;
        }
        sig += (sig == sig_lo) | round_up;
        *sig_dec = sig;
        *exp_dec = exp;
        
    } else {
        while (true) {
            sig_up_div = sig_up / 10;
            sig_lo_div = sig_lo / 10;
            if (sig_up_div <= sig_lo_div) break;
            
            sig_div = sig / 10;
            sig_mod = sig - sig_div * 10;
            sig_end_zero &= (last_trim_num == 0);
            last_trim_num = (u8)sig_mod;
            sig_lo_mod = sig_lo - sig_lo_div * 10;
            sig_lo_end_zero &= (sig_lo_mod == 0);
            sig = sig_div;
            sig_up = sig_up_div;
            sig_lo = sig_lo_div;
            exp++;
        }
        
        if (sig_lo_end_zero) {
            while (true) {
                sig_lo_div = sig_lo / 10;
                sig_lo_mod = sig_lo - sig_lo_div * 10;
                if (sig_lo_mod != 0) break;
                
                sig_up_div = sig_up / 10;
                sig_div = sig / 10;
                sig_mod = sig - sig_div * 10;
                sig_end_zero &= (last_trim_num == 0);
                last_trim_num = (u8)sig_mod;
                sig = sig_div;
                sig_up = sig_up_div;
                sig_lo = sig_lo_div;
                exp++;
            }
        }
        if (sig_end_zero && last_trim_num == 5 && sig % 2 == 0) {
            last_trim_num = 4;
        }
        if (sig == sig_lo && (!accept_bounds || !sig_lo_end_zero)) {
            sig++;
        } else if (last_trim_num >= 5) {
            sig++;
        }
        
        *sig_dec = sig;
        *exp_dec = exp;
    }
}

/** Write a double number (require 32 bytes). no null-terminated */
static_inline u8 *ryu_yy_imp(u8 *buf, u64 raw) {
    u64 sig; /* significand in decimal */
    i32 exp; /* exponent in decimal */
    i32 sig_len; /* significand digit count */
    i32 dot_pos; /* decimal point position in significand digit sequence */
    
    /* decode from raw bytes with IEEE-754 double format. */
    bool sign = (bool)(raw >> (F64_BITS - 1));
    u64 sig_raw = raw & F64_SIG_MASK;
    u32 exp_raw = (raw & F64_EXP_MASK) >> F64_SIG_BITS;
    
    if (unlikely(exp_raw == ((u32)1 << F64_EXP_BITS) - 1)) {
        /* nan or inf */
        if (sig_raw == 0) {
            buf[0] = '-';
            buf += sign;
            *(v32 *)&buf[0] = v32_make('I', 'n', 'f', 'i');
            *(v32 *)&buf[4] = v32_make('n', 'i', 't', 'y');
            buf += 8;
            return buf;
        } else {
            *(v32 *)&buf[0] = v32_make('N', 'a', 'N', '\0');
            return buf + 3;
        }
    }
    
    /* add sign for all finite double value, include -0.0 */
    buf[0] = '-';
    buf += sign;
    if ((raw << 1) == 0) {
        *(v32 *)&buf[0] = v32_make('0', '.', '0', '\0');
        buf += 3;
        return buf;
    }
    
    /* get shortest decimal representation */
    f64_to_dec(sig_raw, exp_raw, &sig, &exp);
    sig_len = u64_dec_len(sig);
    dot_pos = sig_len + exp;
    
    if (0 < dot_pos && dot_pos <= 21) {
        /* dot is after the first digit, digit count <= 21 */
        if (sig_len <= dot_pos) {
            /* dot is after last digit, 123e2 -> 12300 */
            buf = write_u64_len_1_17(sig, buf);
            buf = write_zeros(buf, exp);
            *(v16 *)buf = v16_make('.', '0');
            buf += 2;
            return buf;
        } else {
            /* dot is inside the digits, 123e-2 -> 1.23 */
            u8 *end = write_u64_len_1_17(sig, buf + 1);
            while(dot_pos-- > 0) {
                buf[0] = buf[1];
                buf++;
            }
            *buf = '.';
            return end;
        }
    } else if (-6 < dot_pos && dot_pos <= 0) {
        /* dot is before first digit, and the padding zero count < 6,
         123e-4 -> 0.0123
         123e-8 -> 0.00000123
         */
        *(v16 *)buf = v16_make('0', '.');
        buf = write_zeros(buf + 2, -dot_pos);
        buf = write_u64_len_1_17(sig, buf);
        return buf;
    } else {
        if (sig_len == 1) {
            /* single digit, no need dot, 1e45*/
            buf[0] = (u8)sig + (u8)'0';
            buf[1] = 'e';
            buf = write_f64_exp(dot_pos - 1, buf + 2);
            return buf;
        } else {
            /* format as 1.23e45  */
            u8 *hdr = buf;
            buf = write_u64_len_1_17(sig, buf + 1);
            hdr[0] = hdr[1];
            hdr[1] = '.';
            buf[0] = 'e';
            buf = write_f64_exp(dot_pos - 1, buf + 1);
            return buf;
        }
    }
}

char *dtoa_ryu_yy(double val, char *buf) {
    char *end = (char *)ryu_yy_imp((u8 *)buf, f64_to_raw(val));
    *end = '\0';
    return end;;
}
