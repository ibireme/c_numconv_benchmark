/*
 A fast but inaccurate (0-2 ulp error) method to read double by ibireme.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <math.h>



/* compiler builtin check (clang) */
#ifndef has_builtin
#   ifdef __has_builtin
#       define has_builtin(x) __has_builtin(x)
#   else
#       define has_builtin(x) 0
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
#ifndef always_inline
#   if _MSC_VER >= 1200
#       define always_inline __forceinline
#   elif defined(_MSC_VER)
#       define always_inline __inline
#   elif yy_has_attribute(always_inline) || __GNUC__ >= 4
#       define always_inline __inline__ __attribute__((always_inline))
#   elif defined(__clang__) || defined(__GNUC__)
#       define always_inline __inline__
#   elif defined(__cplusplus) || (__STDC__ >= 1 && __STDC_VERSION__ >= 199901L)
#       define always_inline inline
#   else
#       define always_inline
#   endif
#endif

#define static_inline static always_inline

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

/* int128 type */
#ifndef HAS_INT128
#   if (__SIZEOF_INT128__ == 16) && \
    (defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER))
#       define HAS_INT128 1
#   else
#       define HAS_INT128 0
#   endif
#endif

/* gcc version check */
#ifndef gcc_available
#   define gcc_available(major, minor, patch) \
        ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= \
        (major * 10000 + minor * 100 + patch))
#endif

/* real gcc check */
#ifndef yy_is_real_gcc
#   if !defined(__clang__) && !defined(__INTEL_COMPILER) && !defined(__ICC) && \
defined(__GNUC__) && defined(__GNUC_MINOR__)
#       define yy_is_real_gcc 1
#   endif
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
#if has_builtin(__builtin_clzll) || gcc_available(3, 4, 0)
#   define GCC_HAS_CLZLL 1
#endif

#if has_builtin(__builtin_ctzll) || gcc_available(3, 4, 0)
#   define GCC_HAS_CTZLL 1
#endif

/* IEEE 754 floating-point binary representation */
#ifndef HAS_IEEE_754
#   if __STDC_IEC_559__
#       define HAS_IEEE_754 1
#   elif (FLT_RADIX == 2) && (DBL_MANT_DIG == 53) && \
         (DBL_MIN_EXP == -1021) && (DBL_MAX_EXP == 1024) && \
         (DBL_MIN_10_EXP == -307) && (DBL_MAX_10_EXP == 308)
#       define HAS_IEEE_754 1
#   else
#       define HAS_IEEE_754 0
#       error Not IEEE-754
#   endif
#endif

/*
 On the x86 architecture, some compilers may use x87 FPU instructions for
 floating-point arithmetic. The x87 FPU loads all floating point number as
 80-bit double-extended precision internally, then rounds the result to original
 precision, which may produce inaccurate results. For a more detailed
 explanation, see the paper: https://arxiv.org/abs/cs/0701192
 
 Here are some examples of double precision calculation error:
 
     2877.0 / 1e6 == 0.002877, but x87 returns 0.0028770000000000002
     43683.0 * 1e21 == 4.3683e25, but x87 returns 4.3683000000000004e25
 
 Here are some examples of compiler flags to generate x87 instructions on x86:
 
     clang -m32 -mno-sse
     gcc/icc -m32 -mfpmath=387
     msvc /arch:SSE or /arch:IA32
 
 If we are sure that there's no similar error described above, we can define the
 YYJSON_DOUBLE_MATH_CORRECT as 1 to enable the fast path calculation. This is
 not an accurate detection, it's just try to avoid the error at compiler time.
 An accurate detection can be done at runtime:
 
     bool is_double_math_correct(void) {
         volatile double r = 43683.0;
         r *= 1e21;
         return r == 4.3683e25;
     }
 
 */
#ifndef YYJSON_DOUBLE_MATH_CORRECT
#   if !defined(FLT_EVAL_METHOD) && defined(__FLT_EVAL_METHOD__)
#       define FLT_EVAL_METHOD __FLT_EVAL_METHOD__
#   endif
#   if defined(FLT_EVAL_METHOD) && FLT_EVAL_METHOD != 0 && FLT_EVAL_METHOD != 1
#       define YYJSON_DOUBLE_MATH_CORRECT 0
#   elif defined(i386) || defined(__i386) || defined(__i386__) || \
        defined(_X86_) || defined(__X86__) || defined(_M_IX86) || \
        defined(__I86__) || defined(__IA32__) || defined(__THW_INTEL)
#       if (defined(_MSC_VER) && _M_IX86_FP == 2) || __SSE2_MATH__
#           define YYJSON_DOUBLE_MATH_CORRECT 1
#       else
#           define YYJSON_DOUBLE_MATH_CORRECT 0
#       endif
#   elif defined(__x86_64) || defined(__x86_64__) || \
        defined(__amd64) || defined(__amd64__) || \
        defined(_M_AMD64) || defined(_M_X64) || \
        defined(__ia64) || defined(_IA64) || defined(__IA64__) ||  \
        defined(__ia64__) || defined(_M_IA64) || defined(__itanium__) || \
        defined(__arm64) || defined(__arm64__) || \
        defined(__aarch64__) || defined(_M_ARM64) || \
        defined(__arm) || defined(__arm__) || defined(_ARM_) || \
        defined(_ARM) || defined(_M_ARM) || defined(__TARGET_ARCH_ARM) || \
        defined(mips) || defined(__mips) || defined(__mips__) || \
        defined(MIPS) || defined(_MIPS_) || defined(__MIPS__) || \
        defined(_ARCH_PPC64) || defined(__PPC64__) || \
        defined(__ppc64__) || defined(__powerpc64__) || \
        defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) || \
        defined(__ppc__) || defined(__ppc) || defined(__PPC__) || \
        defined(__sparcv9) || defined(__sparc_v9__) || \
        defined(__sparc) || defined(__sparc__) || defined(__sparc64__) || \
        defined(__alpha) || defined(__alpha__) || defined(_M_ALPHA) || \
        defined(__or1k__) || defined(__OR1K__) || defined(OR1K) || \
        defined(__hppa) || defined(__hppa__) || defined(__HPPA__) || \
        defined(__riscv) || defined(__riscv__) || \
        defined(__s390__) || defined(__avr32__) || defined(__SH4__) || \
        defined(__e2k__) || defined(__arc__) || defined(__EMSCRIPTEN__)
#       define YYJSON_DOUBLE_MATH_CORRECT 1
#   else
#       define YYJSON_DOUBLE_MATH_CORRECT 0 /* unknown */
#   endif
#endif


#define repeat_in_1_18(x) { x(1) x(2) x(3) x(4) x(5) x(6) x(7) \
                            x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) \
                            x(16) x(17) x(18) }


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
#if HAS_INT128
__extension__ typedef __int128          i128;
__extension__ typedef unsigned __int128 u128;
#endif

/** 64-bit floating point union, used to avoid the type-based aliasing rule */
typedef union { u64 u; f64 f; } f64_uni;



/*==============================================================================
 * Integer Constants
 *============================================================================*/

/* Used to write u64 literal for C89 which doesn't support "ULL" suffix. */
#undef  U64
#define U64(hi, lo) ((((u64)hi##UL) << 32U) + lo##UL)

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



/*==============================================================================
 * IEEE-754 Double Number Constants
 *============================================================================*/

/* Inf raw value (positive) */
#define F64_RAW_INF U64(0x7FF00000, 0x00000000)

/* NaN raw value (positive, without payload) */
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
#define F64_MIN_DEC_EXP (-324)

/* maximum binary power of normal number */
#define F64_MAX_BIN_EXP 1024

/* minimum binary power of normal number */
#define F64_MIN_BIN_EXP (-1021)



/*==============================================================================
 * Bits Utils
 *============================================================================*/

/** Returns the number of leading 0-bits in value (input should not be 0). */
static_inline
u32 u64_lz_bits(u64 v) {
#if GCC_HAS_CLZLL
    return (u32)__builtin_clzll(v);
#elif MSC_HAS_BIT_SCAN_64
    unsigned long r;
    _BitScanReverse64(&r, v);
    return (u32)63 - (u32)r;
#elif MSC_HAS_BIT_SCAN
    unsigned long hi, lo;
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
    unsigned long r;
    _BitScanForward64(&r, v);
    return (u32)r;
#elif MSC_HAS_BIT_SCAN
    unsigned long lo, hi;
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
 * JSON Number Reader (IEEE-754)
 *============================================================================*/

/** Maximum pow10 exponent for double value. */
#define F64_POW10_EXP_MAX_EXACT 22

/** Maximum pow10 exponent cached (same as F64_MAX_DEC_EXP). */
#define F64_POW10_EXP_MAX 308

/** Cached pow10 table (size: 2.4KB) (generate with misc/make_tables.c). */
static const f64 f64_pow10_table[F64_POW10_EXP_MAX + 1] = {
    1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
    1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
    1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29,
    1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38, 1e39,
    1e40, 1e41, 1e42, 1e43, 1e44, 1e45, 1e46, 1e47, 1e48, 1e49,
    1e50, 1e51, 1e52, 1e53, 1e54, 1e55, 1e56, 1e57, 1e58, 1e59,
    1e60, 1e61, 1e62, 1e63, 1e64, 1e65, 1e66, 1e67, 1e68, 1e69,
    1e70, 1e71, 1e72, 1e73, 1e74, 1e75, 1e76, 1e77, 1e78, 1e79,
    1e80, 1e81, 1e82, 1e83, 1e84, 1e85, 1e86, 1e87, 1e88, 1e89,
    1e90, 1e91, 1e92, 1e93, 1e94, 1e95, 1e96, 1e97, 1e98, 1e99,
    1e100, 1e101, 1e102, 1e103, 1e104, 1e105, 1e106, 1e107, 1e108, 1e109,
    1e110, 1e111, 1e112, 1e113, 1e114, 1e115, 1e116, 1e117, 1e118, 1e119,
    1e120, 1e121, 1e122, 1e123, 1e124, 1e125, 1e126, 1e127, 1e128, 1e129,
    1e130, 1e131, 1e132, 1e133, 1e134, 1e135, 1e136, 1e137, 1e138, 1e139,
    1e140, 1e141, 1e142, 1e143, 1e144, 1e145, 1e146, 1e147, 1e148, 1e149,
    1e150, 1e151, 1e152, 1e153, 1e154, 1e155, 1e156, 1e157, 1e158, 1e159,
    1e160, 1e161, 1e162, 1e163, 1e164, 1e165, 1e166, 1e167, 1e168, 1e169,
    1e170, 1e171, 1e172, 1e173, 1e174, 1e175, 1e176, 1e177, 1e178, 1e179,
    1e180, 1e181, 1e182, 1e183, 1e184, 1e185, 1e186, 1e187, 1e188, 1e189,
    1e190, 1e191, 1e192, 1e193, 1e194, 1e195, 1e196, 1e197, 1e198, 1e199,
    1e200, 1e201, 1e202, 1e203, 1e204, 1e205, 1e206, 1e207, 1e208, 1e209,
    1e210, 1e211, 1e212, 1e213, 1e214, 1e215, 1e216, 1e217, 1e218, 1e219,
    1e220, 1e221, 1e222, 1e223, 1e224, 1e225, 1e226, 1e227, 1e228, 1e229,
    1e230, 1e231, 1e232, 1e233, 1e234, 1e235, 1e236, 1e237, 1e238, 1e239,
    1e240, 1e241, 1e242, 1e243, 1e244, 1e245, 1e246, 1e247, 1e248, 1e249,
    1e250, 1e251, 1e252, 1e253, 1e254, 1e255, 1e256, 1e257, 1e258, 1e259,
    1e260, 1e261, 1e262, 1e263, 1e264, 1e265, 1e266, 1e267, 1e268, 1e269,
    1e270, 1e271, 1e272, 1e273, 1e274, 1e275, 1e276, 1e277, 1e278, 1e279,
    1e280, 1e281, 1e282, 1e283, 1e284, 1e285, 1e286, 1e287, 1e288, 1e289,
    1e290, 1e291, 1e292, 1e293, 1e294, 1e295, 1e296, 1e297, 1e298, 1e299,
    1e300, 1e301, 1e302, 1e303, 1e304, 1e305, 1e306, 1e307, 1e308
};



/*==============================================================================
 * Digit Character Matcher
 *============================================================================*/

/** Digit type */
typedef u8 digi_type;

/** Digit: '0'. */
static const digi_type DIGI_TYPE_ZERO       = 1 << 0;

/** Digit: [1-9]. */
static const digi_type DIGI_TYPE_NONZERO    = 1 << 1;

/** Plus sign (positive): '+'. */
static const digi_type DIGI_TYPE_POS        = 1 << 2;

/** Minus sign (negative): '-'. */
static const digi_type DIGI_TYPE_NEG        = 1 << 3;

/** Decimal point: '.' */
static const digi_type DIGI_TYPE_DOT        = 1 << 4;

/** Exponent sign: 'e, 'E'. */
static const digi_type DIGI_TYPE_EXP        = 1 << 5;

/** Digit type table (generate with misc/make_tables.c) */
static const digi_type digi_table[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x08, 0x10, 0x00,
    0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/** Match a character with specified type. */
static_inline bool digi_is_type(u8 d, digi_type type) {
    return (digi_table[d] & type) != 0;
}

/** Match a sign: '+', '-' */
static_inline bool digi_is_sign(u8 d) {
    return digi_is_type(d, DIGI_TYPE_POS | DIGI_TYPE_NEG);
}

/** Match a none zero digit: [1-9] */
static_inline bool digi_is_nonzero(u8 d) {
    return digi_is_type(d, DIGI_TYPE_NONZERO);
}

/** Match a digit: [0-9] */
static_inline bool digi_is_digit(u8 d) {
    return digi_is_type(d, DIGI_TYPE_ZERO | DIGI_TYPE_NONZERO);
}

/** Match an exponent sign: 'e', 'E'. */
static_inline bool digi_is_exp(u8 d) {
    return digi_is_type(d, DIGI_TYPE_EXP);
}

/** Match a floating point indicator: '.', 'e', 'E'. */
static_inline bool digi_is_fp(u8 d) {
    return digi_is_type(d, DIGI_TYPE_DOT | DIGI_TYPE_EXP);
}

/** Match a digit or floating point indicator: [0-9], '.', 'e', 'E'. */
static_inline bool digi_is_digit_or_fp(u8 d) {
    return digi_is_type(d, DIGI_TYPE_ZERO | DIGI_TYPE_NONZERO |
                           DIGI_TYPE_DOT | DIGI_TYPE_EXP);
}



double strtod_yy_fast(const char *str, size_t len, char **end) {
    
#define return_err() do { \
    *end = (char *)str; \
    return 0.0; \
} while(false)
    
#define return_f64(_v) do { \
    *end = (char *)cur; \
    return sign ? -(f64)(_v) : (f64)(_v); \
} while(false)
    
    const u8 *sig_cut = NULL; /* significant part cutting position for long number */
    const u8 *sig_end = NULL; /* significant part ending position */
    const u8 *dot_pos = NULL; /* decimal point position */
    
    u64 sig = 0; /* significant part of the number */
    i32 exp = 0; /* exponent part of the number */
    
    bool exp_sign = false; /* temporary exponent sign from literal part */
    i64 exp_sig = 0; /* temporary exponent number from significant part */
    i64 exp_lit = 0; /* temporary exponent number from exponent literal part */
    u64 num; /* temporary number for reading */
    const u8 *tmp; /* temporary cursor for reading */
    
    const u8 *cur = (const u8 *)str;
    const u8 *hdr = cur;
    bool sign = (*hdr == '-');
    cur += sign;
    
    if (unlikely(!digi_is_nonzero(*cur))) { /* 0 or non-digit char */
        if (unlikely(*cur != '0')) return_err();
        /* begin with 0 */
        if (likely(!digi_is_digit_or_fp(*++cur))) return_f64(0);
        if (likely(*cur == '.')) {
            dot_pos = cur++;
            if (unlikely(!digi_is_digit(*cur))) return_err();
            while (unlikely(*cur == '0')) cur++;
            if (likely(digi_is_digit(*cur))) {
                /* first non-zero digit after decimal point */
                sig = (u64)(*cur - '0'); /* read first digit */
                cur--;
                goto digi_frac_1; /* continue read fraction part */
            }
        }
        if (unlikely(digi_is_digit(*cur))) return_err();
        if (unlikely(digi_is_exp(*cur))) { /* 0 with any exponent is still 0 */
            cur += (usize)1 + digi_is_sign(cur[1]);
            if (unlikely(!digi_is_digit(*cur))) return_err();
            while (digi_is_digit(*++cur));
        }
        return_f64(0);
    }
    
    /* begin with non-zero digit,  */
    sig = (u64)(*cur - '0');
    
    /*
     Read integral part, same as the following code.
     For more explanation, see the comments under label `skip_ascii_begin`.
     
         for (int i = 1; i <= 18; i++) {
            num = cur[i] - '0';
            if (num <= 9) sig = num + sig * 10;
            else goto digi_sepr_i;
         }
     */
#if yy_is_real_gcc
#define expr_intg(i) \
    if (likely((num = (u64)(cur[i] - (u8)'0')) <= 9)) sig = num + sig * 10; \
    else { __asm volatile("":"=m"(cur[i])::); goto digi_sepr_##i; }
#else
#define expr_intg(i) \
    if (likely((num = (u64)(cur[i] - (u8)'0')) <= 9)) sig = num + sig * 10; \
    else { goto digi_sepr_##i; }
#endif
    repeat_in_1_18(expr_intg);
#undef expr_intg
    
    cur += 19; /* skip continuous 19 digits */
    if (!digi_is_digit_or_fp(*cur)) return_f64(sig);
    goto digi_intg_more; /* read more digits in integral part */
    
    /* process first non-digit character */
#define expr_sepr(i) \
    digi_sepr_##i: \
    if (likely(!digi_is_fp(cur[i]))) { cur += i; return_f64(sig); } \
    dot_pos = cur + i; \
    if (likely(cur[i] == '.')) goto digi_frac_##i; \
    cur += i; sig_end = cur; goto digi_exp_more;
    repeat_in_1_18(expr_sepr)
#undef expr_sepr
    
    /* read fraction part */
#if yy_is_real_gcc
#define expr_frac(i) \
    digi_frac_##i: \
    if (likely((num = (u64)(cur[i + 1] - (u8)'0')) <= 9)) \
        sig = num + sig * 10; \
    else { __asm volatile("":"=m"(cur[i + 1])::); goto digi_stop_##i; }
#else
#define expr_frac(i) \
    digi_frac_##i: \
    if (likely((num = (u64)(cur[i + 1] - (u8)'0')) <= 9)) \
        sig = num + sig * 10; \
    else { goto digi_stop_##i; }
#endif
    repeat_in_1_18(expr_frac)
#undef expr_frac
    
    cur += 20; /* skip 19 digits and 1 decimal point */
    if (!digi_is_digit(*cur)) goto digi_frac_end; /* fraction part end */
    goto digi_frac_more; /* read more digits in fraction part */
    
    /* significant part end */
#define expr_stop(i) \
    digi_stop_##i: \
    cur += i + 1; \
    goto digi_frac_end;
    repeat_in_1_18(expr_stop)
#undef expr_stop
    
digi_intg_more: /* read more digits in integral part */
    if (digi_is_digit(*cur)) {
        if (!digi_is_digit_or_fp(cur[1])) {
            /* this number is an integer with 20 digits */
            num = (u64)(*cur - '0');
            if ((sig < (UINT64_MAX / 10)) ||
                (sig == (UINT64_MAX / 10) && num <= (UINT64_MAX % 10))) {
                sig = num + sig * 10;
                cur++;
                return_f64(sig);
            }
        }
    }
    
    if (digi_is_exp(*cur)) {
        dot_pos = cur;
        goto digi_exp_more;
    }
    
    if (*cur == '.') {
        dot_pos = cur++;
        if (!digi_is_digit(*cur)) return_err();
    }
    
digi_frac_more: /* read more digits in fraction part */
    sig_cut = cur; /* too large to fit in u64, excess digits need to be cut */
    sig += (*cur >= '5'); /* round */
    while (digi_is_digit(*++cur));
    if (!dot_pos) {
        dot_pos = cur;
        if (*cur == '.') {
            if (!digi_is_digit(*++cur)) return_err();
            while (digi_is_digit(*cur)) cur++;
        }
    }
    exp_sig = (i64)(dot_pos - sig_cut);
    exp_sig += (dot_pos < sig_cut);
    
    /* ignore trailing zeros */
    tmp = cur - 1;
    while (*tmp == '0' || *tmp == '.') tmp--;
    if (tmp < sig_cut) {
        sig_cut = NULL;
    } else {
        sig_end = cur;
    }
    
    if (digi_is_exp(*cur)) goto digi_exp_more;
    goto digi_exp_finish;
    
digi_frac_end: /* fraction part end */
    if (unlikely(dot_pos + 1 == cur)) return_err();
    sig_end = cur;
    exp_sig = -(i64)((u64)(cur - dot_pos) - 1);
    if (likely(!digi_is_exp(*cur))) {
        if (unlikely(exp_sig < F64_MIN_DEC_EXP - 19)) {
            return_f64(0); /* underflow */
        }
        exp = (i32)exp_sig;
        goto digi_finish;
    } else {
        goto digi_exp_more;
    }
    
digi_exp_more: /* read exponent part */
    exp_sign = (*++cur == '-');
    cur += digi_is_sign(*cur);
    if (unlikely(!digi_is_digit(*cur))) return_err();
    while (*cur == '0') cur++;
    
    /* read exponent literal */
    tmp = cur;
    while (digi_is_digit(*cur)) {
        exp_lit = (*cur++ - '0') + (u64)exp_lit * 10;
    }
    if (unlikely(cur - tmp >= U64_SAFE_DIG)) {
        if (exp_sign) {
            return_f64(0); /* underflow */
        } else {
            return_f64(INFINITY); /* overflow */
        }
    }
    exp_sig += exp_sign ? -exp_lit : exp_lit;
    
digi_exp_finish: /* validate exponent value */
    if (unlikely(exp_sig < F64_MIN_DEC_EXP - 19)) {
        return_f64(0); /* underflow */
    }
    if (unlikely(exp_sig > F64_MAX_DEC_EXP)) {
        return_f64(INFINITY); /* overflow */
    }
    exp = (i32)exp_sig;
    
digi_finish: /* all digit read finished */
    
    /* Fast return (0-2 ulp error) */
    {
        f64 ret = (f64)sig;
        if (likely(exp >= -F64_POW10_EXP_MAX)) {
            if (likely(exp < 0)) {
                ret = ret / f64_pow10_table[-exp];
            } else {
                ret = ret * f64_pow10_table[exp];
            }
        } else {
            ret = ret / f64_pow10_table[F64_MAX_DEC_EXP];
            ret = ret / f64_pow10_table[-(exp + F64_MAX_DEC_EXP)];
        }
        return_f64(ret);
    }
    
#undef return_err
#undef return_f64
}
