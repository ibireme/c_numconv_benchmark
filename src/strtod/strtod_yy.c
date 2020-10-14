/*
 Code from https://github.com/ibireme/yyjson
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

/** Returns the number of significant bits in value (should not be 0). */
static_inline u32 u64_sig_bits(u64 v) {
    return (u32)64 - u64_lz_bits(v) - u64_tz_bits(v);
}



/*==============================================================================
 * 128-bit Integer Utils
 *============================================================================*/

/** Multiplies two 64-bit unsigned integers (a * b),
 returns the 128-bit result as 'hi' and 'lo'. */
static_inline void u128_mul(u64 a, u64 b, u64 *hi, u64 *lo) {
#if HAS_INT128
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
#if HAS_INT128
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

/** Minimum valid bits in f64_bits_to_pow10_exp_table. */
#define F64_BITS_TO_POW10_MIN 3

/**
 Maximum pow10 exponent value which can fit in the bits.
 For example:

    10^4 = binary [1001110001]0000, significant bit is 10.
    10^5 = binary [110000110101]00000, significant bit is 12.
    table[10] = 4.
    table[12] = 5.
    
 */
static const i32 f64_bit_to_pow10_exp_table[F64_SIG_FULL_BITS + 1] = {
    -1, 0, 0, 1, 1, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 6, 6, 7, 7, 8, 8,
    9, 9, 9, 10, 10, 11, 11, 12, 12, 12, 13, 13, 14, 14, 15, 15, 15,
    16, 16, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 21, 21, 22, 22
};



/*==============================================================================
 * Diy Floating Point
 *============================================================================*/

/** Minimum decimal exponent in sig table (F64_MIN_DEC_EXP + 1 - 20). */
#define POW10_SIG_TABLE_MIN_EXP -343

/** Maximum decimal exponent in sig table (F64_MAX_DEC_EXP). */
#define POW10_SIG_TABLE_MAX_EXP 308

/** Normalized significant bits table for pow10 (5.1KB).
    (generate with misc/make_tables.c) */
static const u64 pow10_sig_table[] = {
    U64(0xBF29DCAB, 0xA82FDEAE), U64(0xEEF453D6, 0x923BD65A),
    U64(0x9558B466, 0x1B6565F8), U64(0xBAAEE17F, 0xA23EBF76),
    U64(0xE95A99DF, 0x8ACE6F54), U64(0x91D8A02B, 0xB6C10594),
    U64(0xB64EC836, 0xA47146FA), U64(0xE3E27A44, 0x4D8D98B8),
    U64(0x8E6D8C6A, 0xB0787F73), U64(0xB208EF85, 0x5C969F50),
    U64(0xDE8B2B66, 0xB3BC4724), U64(0x8B16FB20, 0x3055AC76),
    U64(0xADDCB9E8, 0x3C6B1794), U64(0xD953E862, 0x4B85DD79),
    U64(0x87D4713D, 0x6F33AA6C), U64(0xA9C98D8C, 0xCB009506),
    U64(0xD43BF0EF, 0xFDC0BA48), U64(0x84A57695, 0xFE98746D),
    U64(0xA5CED43B, 0x7E3E9188), U64(0xCF42894A, 0x5DCE35EA),
    U64(0x818995CE, 0x7AA0E1B2), U64(0xA1EBFB42, 0x19491A1F),
    U64(0xCA66FA12, 0x9F9B60A7), U64(0xFD00B897, 0x478238D1),
    U64(0x9E20735E, 0x8CB16382), U64(0xC5A89036, 0x2FDDBC63),
    U64(0xF712B443, 0xBBD52B7C), U64(0x9A6BB0AA, 0x55653B2D),
    U64(0xC1069CD4, 0xEABE89F9), U64(0xF148440A, 0x256E2C77),
    U64(0x96CD2A86, 0x5764DBCA), U64(0xBC807527, 0xED3E12BD),
    U64(0xEBA09271, 0xE88D976C), U64(0x93445B87, 0x31587EA3),
    U64(0xB8157268, 0xFDAE9E4C), U64(0xE61ACF03, 0x3D1A45DF),
    U64(0x8FD0C162, 0x06306BAC), U64(0xB3C4F1BA, 0x87BC8697),
    U64(0xE0B62E29, 0x29ABA83C), U64(0x8C71DCD9, 0xBA0B4926),
    U64(0xAF8E5410, 0x288E1B6F), U64(0xDB71E914, 0x32B1A24B),
    U64(0x892731AC, 0x9FAF056F), U64(0xAB70FE17, 0xC79AC6CA),
    U64(0xD64D3D9D, 0xB981787D), U64(0x85F04682, 0x93F0EB4E),
    U64(0xA76C5823, 0x38ED2622), U64(0xD1476E2C, 0x07286FAA),
    U64(0x82CCA4DB, 0x847945CA), U64(0xA37FCE12, 0x6597973D),
    U64(0xCC5FC196, 0xFEFD7D0C), U64(0xFF77B1FC, 0xBEBCDC4F),
    U64(0x9FAACF3D, 0xF73609B1), U64(0xC795830D, 0x75038C1E),
    U64(0xF97AE3D0, 0xD2446F25), U64(0x9BECCE62, 0x836AC577),
    U64(0xC2E801FB, 0x244576D5), U64(0xF3A20279, 0xED56D48A),
    U64(0x9845418C, 0x345644D7), U64(0xBE5691EF, 0x416BD60C),
    U64(0xEDEC366B, 0x11C6CB8F), U64(0x94B3A202, 0xEB1C3F39),
    U64(0xB9E08A83, 0xA5E34F08), U64(0xE858AD24, 0x8F5C22CA),
    U64(0x91376C36, 0xD99995BE), U64(0xB5854744, 0x8FFFFB2E),
    U64(0xE2E69915, 0xB3FFF9F9), U64(0x8DD01FAD, 0x907FFC3C),
    U64(0xB1442798, 0xF49FFB4B), U64(0xDD95317F, 0x31C7FA1D),
    U64(0x8A7D3EEF, 0x7F1CFC52), U64(0xAD1C8EAB, 0x5EE43B67),
    U64(0xD863B256, 0x369D4A41), U64(0x873E4F75, 0xE2224E68),
    U64(0xA90DE353, 0x5AAAE202), U64(0xD3515C28, 0x31559A83),
    U64(0x8412D999, 0x1ED58092), U64(0xA5178FFF, 0x668AE0B6),
    U64(0xCE5D73FF, 0x402D98E4), U64(0x80FA687F, 0x881C7F8E),
    U64(0xA139029F, 0x6A239F72), U64(0xC9874347, 0x44AC874F),
    U64(0xFBE91419, 0x15D7A922), U64(0x9D71AC8F, 0xADA6C9B5),
    U64(0xC4CE17B3, 0x99107C23), U64(0xF6019DA0, 0x7F549B2B),
    U64(0x99C10284, 0x4F94E0FB), U64(0xC0314325, 0x637A193A),
    U64(0xF03D93EE, 0xBC589F88), U64(0x96267C75, 0x35B763B5),
    U64(0xBBB01B92, 0x83253CA3), U64(0xEA9C2277, 0x23EE8BCB),
    U64(0x92A1958A, 0x7675175F), U64(0xB749FAED, 0x14125D37),
    U64(0xE51C79A8, 0x5916F485), U64(0x8F31CC09, 0x37AE58D3),
    U64(0xB2FE3F0B, 0x8599EF08), U64(0xDFBDCECE, 0x67006AC9),
    U64(0x8BD6A141, 0x006042BE), U64(0xAECC4991, 0x4078536D),
    U64(0xDA7F5BF5, 0x90966849), U64(0x888F9979, 0x7A5E012D),
    U64(0xAAB37FD7, 0xD8F58179), U64(0xD5605FCD, 0xCF32E1D7),
    U64(0x855C3BE0, 0xA17FCD26), U64(0xA6B34AD8, 0xC9DFC070),
    U64(0xD0601D8E, 0xFC57B08C), U64(0x823C1279, 0x5DB6CE57),
    U64(0xA2CB1717, 0xB52481ED), U64(0xCB7DDCDD, 0xA26DA269),
    U64(0xFE5D5415, 0x0B090B03), U64(0x9EFA548D, 0x26E5A6E2),
    U64(0xC6B8E9B0, 0x709F109A), U64(0xF867241C, 0x8CC6D4C1),
    U64(0x9B407691, 0xD7FC44F8), U64(0xC2109436, 0x4DFB5637),
    U64(0xF294B943, 0xE17A2BC4), U64(0x979CF3CA, 0x6CEC5B5B),
    U64(0xBD8430BD, 0x08277231), U64(0xECE53CEC, 0x4A314EBE),
    U64(0x940F4613, 0xAE5ED137), U64(0xB9131798, 0x99F68584),
    U64(0xE757DD7E, 0xC07426E5), U64(0x9096EA6F, 0x3848984F),
    U64(0xB4BCA50B, 0x065ABE63), U64(0xE1EBCE4D, 0xC7F16DFC),
    U64(0x8D3360F0, 0x9CF6E4BD), U64(0xB080392C, 0xC4349DED),
    U64(0xDCA04777, 0xF541C568), U64(0x89E42CAA, 0xF9491B61),
    U64(0xAC5D37D5, 0xB79B6239), U64(0xD77485CB, 0x25823AC7),
    U64(0x86A8D39E, 0xF77164BD), U64(0xA8530886, 0xB54DBDEC),
    U64(0xD267CAA8, 0x62A12D67), U64(0x8380DEA9, 0x3DA4BC60),
    U64(0xA4611653, 0x8D0DEB78), U64(0xCD795BE8, 0x70516656),
    U64(0x806BD971, 0x4632DFF6), U64(0xA086CFCD, 0x97BF97F4),
    U64(0xC8A883C0, 0xFDAF7DF0), U64(0xFAD2A4B1, 0x3D1B5D6C),
    U64(0x9CC3A6EE, 0xC6311A64), U64(0xC3F490AA, 0x77BD60FD),
    U64(0xF4F1B4D5, 0x15ACB93C), U64(0x99171105, 0x2D8BF3C5),
    U64(0xBF5CD546, 0x78EEF0B7), U64(0xEF340A98, 0x172AACE5),
    U64(0x9580869F, 0x0E7AAC0F), U64(0xBAE0A846, 0xD2195713),
    U64(0xE998D258, 0x869FACD7), U64(0x91FF8377, 0x5423CC06),
    U64(0xB67F6455, 0x292CBF08), U64(0xE41F3D6A, 0x7377EECA),
    U64(0x8E938662, 0x882AF53E), U64(0xB23867FB, 0x2A35B28E),
    U64(0xDEC681F9, 0xF4C31F31), U64(0x8B3C113C, 0x38F9F37F),
    U64(0xAE0B158B, 0x4738705F), U64(0xD98DDAEE, 0x19068C76),
    U64(0x87F8A8D4, 0xCFA417CA), U64(0xA9F6D30A, 0x038D1DBC),
    U64(0xD47487CC, 0x8470652B), U64(0x84C8D4DF, 0xD2C63F3B),
    U64(0xA5FB0A17, 0xC777CF0A), U64(0xCF79CC9D, 0xB955C2CC),
    U64(0x81AC1FE2, 0x93D599C0), U64(0xA21727DB, 0x38CB0030),
    U64(0xCA9CF1D2, 0x06FDC03C), U64(0xFD442E46, 0x88BD304B),
    U64(0x9E4A9CEC, 0x15763E2F), U64(0xC5DD4427, 0x1AD3CDBA),
    U64(0xF7549530, 0xE188C129), U64(0x9A94DD3E, 0x8CF578BA),
    U64(0xC13A148E, 0x3032D6E8), U64(0xF18899B1, 0xBC3F8CA2),
    U64(0x96F5600F, 0x15A7B7E5), U64(0xBCB2B812, 0xDB11A5DE),
    U64(0xEBDF6617, 0x91D60F56), U64(0x936B9FCE, 0xBB25C996),
    U64(0xB84687C2, 0x69EF3BFB), U64(0xE65829B3, 0x046B0AFA),
    U64(0x8FF71A0F, 0xE2C2E6DC), U64(0xB3F4E093, 0xDB73A093),
    U64(0xE0F218B8, 0xD25088B8), U64(0x8C974F73, 0x83725573),
    U64(0xAFBD2350, 0x644EEAD0), U64(0xDBAC6C24, 0x7D62A584),
    U64(0x894BC396, 0xCE5DA772), U64(0xAB9EB47C, 0x81F5114F),
    U64(0xD686619B, 0xA27255A3), U64(0x8613FD01, 0x45877586),
    U64(0xA798FC41, 0x96E952E7), U64(0xD17F3B51, 0xFCA3A7A1),
    U64(0x82EF8513, 0x3DE648C5), U64(0xA3AB6658, 0x0D5FDAF6),
    U64(0xCC963FEE, 0x10B7D1B3), U64(0xFFBBCFE9, 0x94E5C620),
    U64(0x9FD561F1, 0xFD0F9BD4), U64(0xC7CABA6E, 0x7C5382C9),
    U64(0xF9BD690A, 0x1B68637B), U64(0x9C1661A6, 0x51213E2D),
    U64(0xC31BFA0F, 0xE5698DB8), U64(0xF3E2F893, 0xDEC3F126),
    U64(0x986DDB5C, 0x6B3A76B8), U64(0xBE895233, 0x86091466),
    U64(0xEE2BA6C0, 0x678B597F), U64(0x94DB4838, 0x40B717F0),
    U64(0xBA121A46, 0x50E4DDEC), U64(0xE896A0D7, 0xE51E1566),
    U64(0x915E2486, 0xEF32CD60), U64(0xB5B5ADA8, 0xAAFF80B8),
    U64(0xE3231912, 0xD5BF60E6), U64(0x8DF5EFAB, 0xC5979C90),
    U64(0xB1736B96, 0xB6FD83B4), U64(0xDDD0467C, 0x64BCE4A1),
    U64(0x8AA22C0D, 0xBEF60EE4), U64(0xAD4AB711, 0x2EB3929E),
    U64(0xD89D64D5, 0x7A607745), U64(0x87625F05, 0x6C7C4A8B),
    U64(0xA93AF6C6, 0xC79B5D2E), U64(0xD389B478, 0x79823479),
    U64(0x843610CB, 0x4BF160CC), U64(0xA54394FE, 0x1EEDB8FF),
    U64(0xCE947A3D, 0xA6A9273E), U64(0x811CCC66, 0x8829B887),
    U64(0xA163FF80, 0x2A3426A9), U64(0xC9BCFF60, 0x34C13053),
    U64(0xFC2C3F38, 0x41F17C68), U64(0x9D9BA783, 0x2936EDC1),
    U64(0xC5029163, 0xF384A931), U64(0xF64335BC, 0xF065D37D),
    U64(0x99EA0196, 0x163FA42E), U64(0xC06481FB, 0x9BCF8D3A),
    U64(0xF07DA27A, 0x82C37088), U64(0x964E858C, 0x91BA2655),
    U64(0xBBE226EF, 0xB628AFEB), U64(0xEADAB0AB, 0xA3B2DBE5),
    U64(0x92C8AE6B, 0x464FC96F), U64(0xB77ADA06, 0x17E3BBCB),
    U64(0xE5599087, 0x9DDCAABE), U64(0x8F57FA54, 0xC2A9EAB7),
    U64(0xB32DF8E9, 0xF3546564), U64(0xDFF97724, 0x70297EBD),
    U64(0x8BFBEA76, 0xC619EF36), U64(0xAEFAE514, 0x77A06B04),
    U64(0xDAB99E59, 0x958885C5), U64(0x88B402F7, 0xFD75539B),
    U64(0xAAE103B5, 0xFCD2A882), U64(0xD59944A3, 0x7C0752A2),
    U64(0x857FCAE6, 0x2D8493A5), U64(0xA6DFBD9F, 0xB8E5B88F),
    U64(0xD097AD07, 0xA71F26B2), U64(0x825ECC24, 0xC8737830),
    U64(0xA2F67F2D, 0xFA90563B), U64(0xCBB41EF9, 0x79346BCA),
    U64(0xFEA126B7, 0xD78186BD), U64(0x9F24B832, 0xE6B0F436),
    U64(0xC6EDE63F, 0xA05D3144), U64(0xF8A95FCF, 0x88747D94),
    U64(0x9B69DBE1, 0xB548CE7D), U64(0xC24452DA, 0x229B021C),
    U64(0xF2D56790, 0xAB41C2A3), U64(0x97C560BA, 0x6B0919A6),
    U64(0xBDB6B8E9, 0x05CB600F), U64(0xED246723, 0x473E3813),
    U64(0x9436C076, 0x0C86E30C), U64(0xB9447093, 0x8FA89BCF),
    U64(0xE7958CB8, 0x7392C2C3), U64(0x90BD77F3, 0x483BB9BA),
    U64(0xB4ECD5F0, 0x1A4AA828), U64(0xE2280B6C, 0x20DD5232),
    U64(0x8D590723, 0x948A535F), U64(0xB0AF48EC, 0x79ACE837),
    U64(0xDCDB1B27, 0x98182245), U64(0x8A08F0F8, 0xBF0F156B),
    U64(0xAC8B2D36, 0xEED2DAC6), U64(0xD7ADF884, 0xAA879177),
    U64(0x86CCBB52, 0xEA94BAEB), U64(0xA87FEA27, 0xA539E9A5),
    U64(0xD29FE4B1, 0x8E88640F), U64(0x83A3EEEE, 0xF9153E89),
    U64(0xA48CEAAA, 0xB75A8E2B), U64(0xCDB02555, 0x653131B6),
    U64(0x808E1755, 0x5F3EBF12), U64(0xA0B19D2A, 0xB70E6ED6),
    U64(0xC8DE0475, 0x64D20A8C), U64(0xFB158592, 0xBE068D2F),
    U64(0x9CED737B, 0xB6C4183D), U64(0xC428D05A, 0xA4751E4D),
    U64(0xF5330471, 0x4D9265E0), U64(0x993FE2C6, 0xD07B7FAC),
    U64(0xBF8FDB78, 0x849A5F97), U64(0xEF73D256, 0xA5C0F77D),
    U64(0x95A86376, 0x27989AAE), U64(0xBB127C53, 0xB17EC159),
    U64(0xE9D71B68, 0x9DDE71B0), U64(0x92267121, 0x62AB070E),
    U64(0xB6B00D69, 0xBB55C8D1), U64(0xE45C10C4, 0x2A2B3B06),
    U64(0x8EB98A7A, 0x9A5B04E3), U64(0xB267ED19, 0x40F1C61C),
    U64(0xDF01E85F, 0x912E37A3), U64(0x8B61313B, 0xBABCE2C6),
    U64(0xAE397D8A, 0xA96C1B78), U64(0xD9C7DCED, 0x53C72256),
    U64(0x881CEA14, 0x545C7575), U64(0xAA242499, 0x697392D3),
    U64(0xD4AD2DBF, 0xC3D07788), U64(0x84EC3C97, 0xDA624AB5),
    U64(0xA6274BBD, 0xD0FADD62), U64(0xCFB11EAD, 0x453994BA),
    U64(0x81CEB32C, 0x4B43FCF5), U64(0xA2425FF7, 0x5E14FC32),
    U64(0xCAD2F7F5, 0x359A3B3E), U64(0xFD87B5F2, 0x8300CA0E),
    U64(0x9E74D1B7, 0x91E07E48), U64(0xC6120625, 0x76589DDB),
    U64(0xF79687AE, 0xD3EEC551), U64(0x9ABE14CD, 0x44753B53),
    U64(0xC16D9A00, 0x95928A27), U64(0xF1C90080, 0xBAF72CB1),
    U64(0x971DA050, 0x74DA7BEF), U64(0xBCE50864, 0x92111AEB),
    U64(0xEC1E4A7D, 0xB69561A5), U64(0x9392EE8E, 0x921D5D07),
    U64(0xB877AA32, 0x36A4B449), U64(0xE69594BE, 0xC44DE15B),
    U64(0x901D7CF7, 0x3AB0ACD9), U64(0xB424DC35, 0x095CD80F),
    U64(0xE12E1342, 0x4BB40E13), U64(0x8CBCCC09, 0x6F5088CC),
    U64(0xAFEBFF0B, 0xCB24AAFF), U64(0xDBE6FECE, 0xBDEDD5BF),
    U64(0x89705F41, 0x36B4A597), U64(0xABCC7711, 0x8461CEFD),
    U64(0xD6BF94D5, 0xE57A42BC), U64(0x8637BD05, 0xAF6C69B6),
    U64(0xA7C5AC47, 0x1B478423), U64(0xD1B71758, 0xE219652C),
    U64(0x83126E97, 0x8D4FDF3B), U64(0xA3D70A3D, 0x70A3D70A),
    U64(0xCCCCCCCC, 0xCCCCCCCD), U64(0x80000000, 0x00000000),
    U64(0xA0000000, 0x00000000), U64(0xC8000000, 0x00000000),
    U64(0xFA000000, 0x00000000), U64(0x9C400000, 0x00000000),
    U64(0xC3500000, 0x00000000), U64(0xF4240000, 0x00000000),
    U64(0x98968000, 0x00000000), U64(0xBEBC2000, 0x00000000),
    U64(0xEE6B2800, 0x00000000), U64(0x9502F900, 0x00000000),
    U64(0xBA43B740, 0x00000000), U64(0xE8D4A510, 0x00000000),
    U64(0x9184E72A, 0x00000000), U64(0xB5E620F4, 0x80000000),
    U64(0xE35FA931, 0xA0000000), U64(0x8E1BC9BF, 0x04000000),
    U64(0xB1A2BC2E, 0xC5000000), U64(0xDE0B6B3A, 0x76400000),
    U64(0x8AC72304, 0x89E80000), U64(0xAD78EBC5, 0xAC620000),
    U64(0xD8D726B7, 0x177A8000), U64(0x87867832, 0x6EAC9000),
    U64(0xA968163F, 0x0A57B400), U64(0xD3C21BCE, 0xCCEDA100),
    U64(0x84595161, 0x401484A0), U64(0xA56FA5B9, 0x9019A5C8),
    U64(0xCECB8F27, 0xF4200F3A), U64(0x813F3978, 0xF8940984),
    U64(0xA18F07D7, 0x36B90BE5), U64(0xC9F2C9CD, 0x04674EDF),
    U64(0xFC6F7C40, 0x45812296), U64(0x9DC5ADA8, 0x2B70B59E),
    U64(0xC5371912, 0x364CE305), U64(0xF684DF56, 0xC3E01BC7),
    U64(0x9A130B96, 0x3A6C115C), U64(0xC097CE7B, 0xC90715B3),
    U64(0xF0BDC21A, 0xBB48DB20), U64(0x96769950, 0xB50D88F4),
    U64(0xBC143FA4, 0xE250EB31), U64(0xEB194F8E, 0x1AE525FD),
    U64(0x92EFD1B8, 0xD0CF37BE), U64(0xB7ABC627, 0x050305AE),
    U64(0xE596B7B0, 0xC643C719), U64(0x8F7E32CE, 0x7BEA5C70),
    U64(0xB35DBF82, 0x1AE4F38C), U64(0xE0352F62, 0xA19E306F),
    U64(0x8C213D9D, 0xA502DE45), U64(0xAF298D05, 0x0E4395D7),
    U64(0xDAF3F046, 0x51D47B4C), U64(0x88D8762B, 0xF324CD10),
    U64(0xAB0E93B6, 0xEFEE0054), U64(0xD5D238A4, 0xABE98068),
    U64(0x85A36366, 0xEB71F041), U64(0xA70C3C40, 0xA64E6C52),
    U64(0xD0CF4B50, 0xCFE20766), U64(0x82818F12, 0x81ED44A0),
    U64(0xA321F2D7, 0x226895C8), U64(0xCBEA6F8C, 0xEB02BB3A),
    U64(0xFEE50B70, 0x25C36A08), U64(0x9F4F2726, 0x179A2245),
    U64(0xC722F0EF, 0x9D80AAD6), U64(0xF8EBAD2B, 0x84E0D58C),
    U64(0x9B934C3B, 0x330C8577), U64(0xC2781F49, 0xFFCFA6D5),
    U64(0xF316271C, 0x7FC3908B), U64(0x97EDD871, 0xCFDA3A57),
    U64(0xBDE94E8E, 0x43D0C8EC), U64(0xED63A231, 0xD4C4FB27),
    U64(0x945E455F, 0x24FB1CF9), U64(0xB975D6B6, 0xEE39E437),
    U64(0xE7D34C64, 0xA9C85D44), U64(0x90E40FBE, 0xEA1D3A4B),
    U64(0xB51D13AE, 0xA4A488DD), U64(0xE264589A, 0x4DCDAB15),
    U64(0x8D7EB760, 0x70A08AED), U64(0xB0DE6538, 0x8CC8ADA8),
    U64(0xDD15FE86, 0xAFFAD912), U64(0x8A2DBF14, 0x2DFCC7AB),
    U64(0xACB92ED9, 0x397BF996), U64(0xD7E77A8F, 0x87DAF7FC),
    U64(0x86F0AC99, 0xB4E8DAFD), U64(0xA8ACD7C0, 0x222311BD),
    U64(0xD2D80DB0, 0x2AABD62C), U64(0x83C7088E, 0x1AAB65DB),
    U64(0xA4B8CAB1, 0xA1563F52), U64(0xCDE6FD5E, 0x09ABCF27),
    U64(0x80B05E5A, 0xC60B6178), U64(0xA0DC75F1, 0x778E39D6),
    U64(0xC913936D, 0xD571C84C), U64(0xFB587849, 0x4ACE3A5F),
    U64(0x9D174B2D, 0xCEC0E47B), U64(0xC45D1DF9, 0x42711D9A),
    U64(0xF5746577, 0x930D6501), U64(0x9968BF6A, 0xBBE85F20),
    U64(0xBFC2EF45, 0x6AE276E9), U64(0xEFB3AB16, 0xC59B14A3),
    U64(0x95D04AEE, 0x3B80ECE6), U64(0xBB445DA9, 0xCA61281F),
    U64(0xEA157514, 0x3CF97227), U64(0x924D692C, 0xA61BE758),
    U64(0xB6E0C377, 0xCFA2E12E), U64(0xE498F455, 0xC38B997A),
    U64(0x8EDF98B5, 0x9A373FEC), U64(0xB2977EE3, 0x00C50FE7),
    U64(0xDF3D5E9B, 0xC0F653E1), U64(0x8B865B21, 0x5899F46D),
    U64(0xAE67F1E9, 0xAEC07188), U64(0xDA01EE64, 0x1A708DEA),
    U64(0x884134FE, 0x908658B2), U64(0xAA51823E, 0x34A7EEDF),
    U64(0xD4E5E2CD, 0xC1D1EA96), U64(0x850FADC0, 0x9923329E),
    U64(0xA6539930, 0xBF6BFF46), U64(0xCFE87F7C, 0xEF46FF17),
    U64(0x81F14FAE, 0x158C5F6E), U64(0xA26DA399, 0x9AEF774A),
    U64(0xCB090C80, 0x01AB551C), U64(0xFDCB4FA0, 0x02162A63),
    U64(0x9E9F11C4, 0x014DDA7E), U64(0xC646D635, 0x01A1511E),
    U64(0xF7D88BC2, 0x4209A565), U64(0x9AE75759, 0x6946075F),
    U64(0xC1A12D2F, 0xC3978937), U64(0xF209787B, 0xB47D6B85),
    U64(0x9745EB4D, 0x50CE6333), U64(0xBD176620, 0xA501FC00),
    U64(0xEC5D3FA8, 0xCE427B00), U64(0x93BA47C9, 0x80E98CE0),
    U64(0xB8A8D9BB, 0xE123F018), U64(0xE6D3102A, 0xD96CEC1E),
    U64(0x9043EA1A, 0xC7E41393), U64(0xB454E4A1, 0x79DD1877),
    U64(0xE16A1DC9, 0xD8545E95), U64(0x8CE2529E, 0x2734BB1D),
    U64(0xB01AE745, 0xB101E9E4), U64(0xDC21A117, 0x1D42645D),
    U64(0x899504AE, 0x72497EBA), U64(0xABFA45DA, 0x0EDBDE69),
    U64(0xD6F8D750, 0x9292D603), U64(0x865B8692, 0x5B9BC5C2),
    U64(0xA7F26836, 0xF282B733), U64(0xD1EF0244, 0xAF2364FF),
    U64(0x8335616A, 0xED761F1F), U64(0xA402B9C5, 0xA8D3A6E7),
    U64(0xCD036837, 0x130890A1), U64(0x80222122, 0x6BE55A65),
    U64(0xA02AA96B, 0x06DEB0FE), U64(0xC83553C5, 0xC8965D3D),
    U64(0xFA42A8B7, 0x3ABBF48D), U64(0x9C69A972, 0x84B578D8),
    U64(0xC38413CF, 0x25E2D70E), U64(0xF46518C2, 0xEF5B8CD1),
    U64(0x98BF2F79, 0xD5993803), U64(0xBEEEFB58, 0x4AFF8604),
    U64(0xEEAABA2E, 0x5DBF6785), U64(0x952AB45C, 0xFA97A0B3),
    U64(0xBA756174, 0x393D88E0), U64(0xE912B9D1, 0x478CEB17),
    U64(0x91ABB422, 0xCCB812EF), U64(0xB616A12B, 0x7FE617AA),
    U64(0xE39C4976, 0x5FDF9D95), U64(0x8E41ADE9, 0xFBEBC27D),
    U64(0xB1D21964, 0x7AE6B31C), U64(0xDE469FBD, 0x99A05FE3),
    U64(0x8AEC23D6, 0x80043BEE), U64(0xADA72CCC, 0x20054AEA),
    U64(0xD910F7FF, 0x28069DA4), U64(0x87AA9AFF, 0x79042287),
    U64(0xA99541BF, 0x57452B28), U64(0xD3FA922F, 0x2D1675F2),
    U64(0x847C9B5D, 0x7C2E09B7), U64(0xA59BC234, 0xDB398C25),
    U64(0xCF02B2C2, 0x1207EF2F), U64(0x8161AFB9, 0x4B44F57D),
    U64(0xA1BA1BA7, 0x9E1632DC), U64(0xCA28A291, 0x859BBF93),
    U64(0xFCB2CB35, 0xE702AF78), U64(0x9DEFBF01, 0xB061ADAB),
    U64(0xC56BAEC2, 0x1C7A1916), U64(0xF6C69A72, 0xA3989F5C),
    U64(0x9A3C2087, 0xA63F6399), U64(0xC0CB28A9, 0x8FCF3C80),
    U64(0xF0FDF2D3, 0xF3C30B9F), U64(0x969EB7C4, 0x7859E744),
    U64(0xBC4665B5, 0x96706115), U64(0xEB57FF22, 0xFC0C795A),
    U64(0x9316FF75, 0xDD87CBD8), U64(0xB7DCBF53, 0x54E9BECE),
    U64(0xE5D3EF28, 0x2A242E82), U64(0x8FA47579, 0x1A569D11),
    U64(0xB38D92D7, 0x60EC4455), U64(0xE070F78D, 0x3927556B),
    U64(0x8C469AB8, 0x43B89563), U64(0xAF584166, 0x54A6BABB),
    U64(0xDB2E51BF, 0xE9D0696A), U64(0x88FCF317, 0xF22241E2),
    U64(0xAB3C2FDD, 0xEEAAD25B), U64(0xD60B3BD5, 0x6A5586F2),
    U64(0x85C70565, 0x62757457), U64(0xA738C6BE, 0xBB12D16D),
    U64(0xD106F86E, 0x69D785C8), U64(0x82A45B45, 0x0226B39D),
    U64(0xA34D7216, 0x42B06084), U64(0xCC20CE9B, 0xD35C78A5),
    U64(0xFF290242, 0xC83396CE), U64(0x9F79A169, 0xBD203E41),
    U64(0xC75809C4, 0x2C684DD1), U64(0xF92E0C35, 0x37826146),
    U64(0x9BBCC7A1, 0x42B17CCC), U64(0xC2ABF989, 0x935DDBFE),
    U64(0xF356F7EB, 0xF83552FE), U64(0x98165AF3, 0x7B2153DF),
    U64(0xBE1BF1B0, 0x59E9A8D6), U64(0xEDA2EE1C, 0x7064130C),
    U64(0x9485D4D1, 0xC63E8BE8), U64(0xB9A74A06, 0x37CE2EE1),
    U64(0xE8111C87, 0xC5C1BA9A), U64(0x910AB1D4, 0xDB9914A0),
    U64(0xB54D5E4A, 0x127F59C8), U64(0xE2A0B5DC, 0x971F303A),
    U64(0x8DA471A9, 0xDE737E24), U64(0xB10D8E14, 0x56105DAD),
    U64(0xDD50F199, 0x6B947519), U64(0x8A5296FF, 0xE33CC930),
    U64(0xACE73CBF, 0xDC0BFB7B), U64(0xD8210BEF, 0xD30EFA5A),
    U64(0x8714A775, 0xE3E95C78), U64(0xA8D9D153, 0x5CE3B396),
    U64(0xD31045A8, 0x341CA07C), U64(0x83EA2B89, 0x2091E44E),
    U64(0xA4E4B66B, 0x68B65D61), U64(0xCE1DE406, 0x42E3F4B9),
    U64(0x80D2AE83, 0xE9CE78F4), U64(0xA1075A24, 0xE4421731),
    U64(0xC94930AE, 0x1D529CFD), U64(0xFB9B7CD9, 0xA4A7443C),
    U64(0x9D412E08, 0x06E88AA6), U64(0xC491798A, 0x08A2AD4F),
    U64(0xF5B5D7EC, 0x8ACB58A3), U64(0x9991A6F3, 0xD6BF1766),
    U64(0xBFF610B0, 0xCC6EDD3F), U64(0xEFF394DC, 0xFF8A948F),
    U64(0x95F83D0A, 0x1FB69CD9), U64(0xBB764C4C, 0xA7A44410),
    U64(0xEA53DF5F, 0xD18D5514), U64(0x92746B9B, 0xE2F8552C),
    U64(0xB7118682, 0xDBB66A77), U64(0xE4D5E823, 0x92A40515),
    U64(0x8F05B116, 0x3BA6832D), U64(0xB2C71D5B, 0xCA9023F8),
    U64(0xDF78E4B2, 0xBD342CF7), U64(0x8BAB8EEF, 0xB6409C1A),
    U64(0xAE9672AB, 0xA3D0C321), U64(0xDA3C0F56, 0x8CC4F3E9),
    U64(0x88658996, 0x17FB1871), U64(0xAA7EEBFB, 0x9DF9DE8E),
    U64(0xD51EA6FA, 0x85785631), U64(0x8533285C, 0x936B35DF),
    U64(0xA67FF273, 0xB8460357), U64(0xD01FEF10, 0xA657842C),
    U64(0x8213F56A, 0x67F6B29C), U64(0xA298F2C5, 0x01F45F43),
    U64(0xCB3F2F76, 0x42717713), U64(0xFE0EFB53, 0xD30DD4D8),
    U64(0x9EC95D14, 0x63E8A507), U64(0xC67BB459, 0x7CE2CE49),
    U64(0xF81AA16F, 0xDC1B81DB), U64(0x9B10A4E5, 0xE9913129),
    U64(0xC1D4CE1F, 0x63F57D73), U64(0xF24A01A7, 0x3CF2DCD0),
    U64(0x976E4108, 0x8617CA02), U64(0xBD49D14A, 0xA79DBC82),
    U64(0xEC9C459D, 0x51852BA3), U64(0x93E1AB82, 0x52F33B46),
    U64(0xB8DA1662, 0xE7B00A17), U64(0xE7109BFB, 0xA19C0C9D),
    U64(0x906A617D, 0x450187E2), U64(0xB484F9DC, 0x9641E9DB),
    U64(0xE1A63853, 0xBBD26451), U64(0x8D07E334, 0x55637EB3),
    U64(0xB049DC01, 0x6ABC5E60), U64(0xDC5C5301, 0xC56B75F7),
    U64(0x89B9B3E1, 0x1B6329BB), U64(0xAC2820D9, 0x623BF429),
    U64(0xD732290F, 0xBACAF134), U64(0x867F59A9, 0xD4BED6C0),
    U64(0xA81F3014, 0x49EE8C70), U64(0xD226FC19, 0x5C6A2F8C),
    U64(0x83585D8F, 0xD9C25DB8), U64(0xA42E74F3, 0xD032F526),
    U64(0xCD3A1230, 0xC43FB26F), U64(0x80444B5E, 0x7AA7CF85),
    U64(0xA0555E36, 0x1951C367), U64(0xC86AB5C3, 0x9FA63441),
    U64(0xFA856334, 0x878FC151), U64(0x9C935E00, 0xD4B9D8D2),
    U64(0xC3B83581, 0x09E84F07), U64(0xF4A642E1, 0x4C6262C9),
    U64(0x98E7E9CC, 0xCFBD7DBE), U64(0xBF21E440, 0x03ACDD2D),
    U64(0xEEEA5D50, 0x04981478), U64(0x95527A52, 0x02DF0CCB),
    U64(0xBAA718E6, 0x8396CFFE), U64(0xE950DF20, 0x247C83FD),
    U64(0x91D28B74, 0x16CDD27E), U64(0xB6472E51, 0x1C81471E),
    U64(0xE3D8F9E5, 0x63A198E5), U64(0x8E679C2F, 0x5E44FF8F)
};

/** "Do It Yourself Floating Point" struct. */
typedef struct diy_fp {
    u64 sig; /* significand */
    i32 exp; /* exponent, base 2 */
} diy_fp;

/** Get cached diy_fp with 10^x. The input value must in range
    [POW10_SIG_TABLE_MIN_EXP, POW10_SIG_TABLE_MAX_EXP]. */
static_inline diy_fp diy_fp_get_cached_pow10(i32 pow10) {
    u64 sig = pow10_sig_table[pow10 - POW10_SIG_TABLE_MIN_EXP];
    i32 exp = (pow10 * 217706 - 4128768) >> 16; /* exponent base 2 */
    diy_fp fp;
    fp.sig = sig;
    fp.exp = exp;
    return fp;
}

/** Returns fp * fp2. */
static_inline diy_fp diy_fp_mul(diy_fp fp, diy_fp fp2) {
    u64 hi, lo;
    u128_mul(fp.sig, fp2.sig, &hi, &lo);
    fp.sig = hi + (lo >> 63);
    fp.exp += fp2.exp + 64;
    return fp;
}

/** Convert diy_fp to IEEE-754 raw value. */
static_inline u64 diy_fp_to_ieee_raw(diy_fp fp) {
    u64 sig = fp.sig;
    i32 exp = fp.exp;
    u32 lz_bits;
    if (unlikely(fp.sig == 0)) return 0;
    
    lz_bits = u64_lz_bits(sig);
    sig <<= lz_bits;
    sig >>= F64_BITS - F64_SIG_FULL_BITS;
    exp -= lz_bits;
    exp += F64_BITS - F64_SIG_FULL_BITS;
    exp += F64_SIG_BITS;
    
    if (unlikely(exp >= F64_MAX_BIN_EXP)) {
        /* overflow */
        return F64_RAW_INF;
    } else if (likely(exp >= F64_MIN_BIN_EXP - 1)) {
        /* normal */
        exp += F64_EXP_BIAS;
        return ((u64)exp << F64_SIG_BITS) | (sig & F64_SIG_MASK);
    } else if (likely(exp >= F64_MIN_BIN_EXP - F64_SIG_FULL_BITS)) {
        /* subnormal */
        return sig >> (F64_MIN_BIN_EXP - exp - 1);
    } else {
        /* underflow */
        return 0;
    }
}



/*==============================================================================
 * BigInt For Floating Point Number Reader
 *============================================================================*/

/** Maximum exponent of exact pow10 */
#define U64_POW10_MAX_EXP 19

/** Table: [ 10^0, ..., 10^19 ] (generate with misc/make_tables.c) */
static const u64 u64_pow10_table[U64_POW10_MAX_EXP + 1] = {
    U64(0x00000000, 0x00000001), U64(0x00000000, 0x0000000A),
    U64(0x00000000, 0x00000064), U64(0x00000000, 0x000003E8),
    U64(0x00000000, 0x00002710), U64(0x00000000, 0x000186A0),
    U64(0x00000000, 0x000F4240), U64(0x00000000, 0x00989680),
    U64(0x00000000, 0x05F5E100), U64(0x00000000, 0x3B9ACA00),
    U64(0x00000002, 0x540BE400), U64(0x00000017, 0x4876E800),
    U64(0x000000E8, 0xD4A51000), U64(0x00000918, 0x4E72A000),
    U64(0x00005AF3, 0x107A4000), U64(0x00038D7E, 0xA4C68000),
    U64(0x002386F2, 0x6FC10000), U64(0x01634578, 0x5D8A0000),
    U64(0x0DE0B6B3, 0xA7640000), U64(0x8AC72304, 0x89E80000)
};

/** Maximum numbers of chunks used by a bigint (58 is enough here). */
#define BIGINT_MAX_CHUNKS 64

/** Unsigned arbitrarily large integer */
typedef struct bigint {
    u32 used; /* used chunks count, should not be 0 */
    u64 bits[BIGINT_MAX_CHUNKS]; /* chunks */
} bigint;

/**
 Evaluate 'big += val'.
 @param big A big number (can be 0).
 @param val An unsigned integer (can be 0).
 */
static_inline void bigint_add_u64(bigint *big, u64 val) {
    u32 idx, max;
    u64 num = big->bits[0];
    u64 add = num + val;
    big->bits[0] = add;
    if (likely((add >= num) || (add >= val))) return;
    for ((void)(idx = 1), max = big->used; idx < max; idx++) {
        if (likely(big->bits[idx] != U64_MAX)) {
            big->bits[idx] += 1;
            return;
        }
        big->bits[idx] = 0;
    }
    big->bits[big->used++] = 1;
}

/**
 Evaluate 'big *= val'.
 @param big A big number (can be 0).
 @param val An unsigned integer (cannot be 0).
 */
static_inline void bigint_mul_u64(bigint *big, u64 val) {
    u32 idx = 0, max = big->used;
    u64 hi, lo, carry = 0;
    for (; idx < max; idx++) {
        if (big->bits[idx]) break;
    }
    for (; idx < max; idx++) {
        u128_mul_add(big->bits[idx], val, carry, &hi, &lo);
        big->bits[idx] = lo;
        carry = hi;
    }
    if (carry) big->bits[big->used++] = carry;
}

/**
 Evaluate 'big *= 2^exp'.
 @param big A big number (can be 0).
 @param exp An exponent integer (can be 0).
 */
static_inline void bigint_mul_pow2(bigint *big, u32 exp) {
    u32 shft = exp % 64;
    u32 move = exp / 64;
    u32 idx = big->used;
    if (unlikely(shft == 0)) {
        for (; idx > 0; idx--) {
            big->bits[idx + move - 1] = big->bits[idx - 1];
        }
        big->used += move;
        while (move) big->bits[--move] = 0;
    } else {
        big->bits[idx] = 0;
        for (; idx > 0; idx--) {
            u64 num = big->bits[idx] << shft;
            num |= big->bits[idx - 1] >> (64 - shft);
            big->bits[idx + move] = num;
        }
        big->bits[move] = big->bits[0] << shft;
        big->used += move + (big->bits[big->used + move] > 0);
        while (move) big->bits[--move] = 0;
    }
}

/**
 Evaluate 'big *= 10^exp'.
 @param big A big number (can be 0).
 @param exp An exponent integer (cannot be 0).
 */
static_inline void bigint_mul_pow10(bigint *big, i32 exp) {
    for (; exp >= U64_POW10_MAX_EXP; exp -= U64_POW10_MAX_EXP) {
        bigint_mul_u64(big, u64_pow10_table[U64_POW10_MAX_EXP]);
    }
    if (exp) {
        bigint_mul_u64(big, u64_pow10_table[exp]);
    }
}

/**
 Compare two bigint.
 @return -1 if 'a < b', +1 if 'a > b', 0 if 'a == b'.
 */
static_inline i32 bigint_cmp(bigint *a, bigint *b) {
    u32 idx = a->used;
    if (a->used < b->used) return -1;
    if (a->used > b->used) return +1;
    while (idx --> 0) {
        u64 av = a->bits[idx];
        u64 bv = b->bits[idx];
        if (av < bv) return -1;
        if (av > bv) return +1;
    }
    return 0;
}

/**
 Evaluate 'big = val'.
 @param big A big number (can be 0).
 @param val An unsigned integer (can be 0).
 */
static_inline void bigint_set_u64(bigint *big, u64 val) {
    big->used = 1;
    big->bits[0] = val;
}

/** Set a bigint with floating point number string. */
static void bigint_set_buf(bigint *big, u64 sig, i32 *exp,
                           const u8 *sig_cut, const u8 *sig_end, const u8 *dot_pos) {
    
    if (unlikely(!sig_cut)) {
        /* no digit cut, set significant part only */
        bigint_set_u64(big, sig);
        return;
        
    } else {
        /* some digits was cut, read them from 'sig_cut' to 'sig_end' */
        const u8 *hdr = sig_cut;
        const u8 *cur = hdr;
        u32 len = 0;
        u64 val = 0;
        bool dig_big_cut = false;
        bool has_dot = (hdr < dot_pos) & (dot_pos < sig_end);
        u32 dig_len_total = U64_SAFE_DIG + (u32)(sig_end - hdr) - has_dot;
        
        sig -= (*sig_cut >= '5'); /* sig was rounded before */
        if (dig_len_total > F64_MAX_DEC_DIG) {
            dig_big_cut = true;
            sig_end -= dig_len_total - (F64_MAX_DEC_DIG + 1);
            sig_end -= (dot_pos + 1 == sig_end);
            dig_len_total = (F64_MAX_DEC_DIG + 1);
        }
        *exp -= (i32)dig_len_total - U64_SAFE_DIG;
        
        big->used = 1;
        big->bits[0] = sig;
        while (cur < sig_end) {
            if (likely(cur != dot_pos)) {
                val = val * 10 + (*cur++ - '0');
                len++;
                if (unlikely(cur == sig_end && dig_big_cut)) {
                    /* The last digit must be non-zero,    */
                    /* set it to '1' for correct rounding. */
                    val = val - (val % 10) + 1;
                }
                if (len == U64_SAFE_DIG || cur == sig_end) {
                    bigint_mul_pow10(big, len);
                    bigint_add_u64(big, val);
                    val = 0;
                    len = 0;
                }
            } else {
                cur++;
            }
        }
    }
}



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

double strtod_yy(const char *str, size_t len, char **end) {
    
#define return_err() do { \
    *end = (char *)str; \
    return 0.0; \
} while(false)
    
#define return_f64(_v) do { \
    *end = (char *)cur; \
    return sign ? -(f64)(_v) : (f64)(_v); \
} while(false)
    
#define return_f64_raw(_v) do { \
    f64_uni uni; \
    uni.u = ((u64)sign << 63) | (u64)(_v); \
    *end = (char *)cur; \
    return uni.f; \
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
    
    /* read integral part */
#define expr_intg(i) \
    if (likely((num = (u64)(cur[i] - (u8)'0')) <= 9)) sig = num + sig * 10; \
    else goto digi_sepr_##i;
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
#define expr_frac(i) \
    digi_frac_##i: \
    if (likely((num = (u64)(cur[i + 1] - (u8)'0')) <= 9)) \
        sig = num + sig * 10; \
    else goto digi_stop_##i;
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
    
    /*
     Fast path (accurate), requirements:
     1. The input floating-point number does not lose precision.
     2. The floating-point number calculation is accurate.
     3. Correct rounding is performed (FE_TONEAREST).
     */
#if YYJSON_DOUBLE_MATH_CORRECT
    if (likely(!sig_cut &&
               exp >= -F64_POW10_EXP_MAX_EXACT &&
               exp <= +F64_POW10_EXP_MAX_EXACT * 2)) {
        u32 bits = u64_sig_bits(sig);
        if (bits <= F64_SIG_FULL_BITS) {
            if (exp < 0) {
                f64 dbl = (f64)sig;
                dbl /= f64_pow10_table[-exp];
                return_f64(dbl);
            }
            if (exp <= F64_POW10_EXP_MAX_EXACT) {
                f64 dbl = (f64)sig;
                dbl *= f64_pow10_table[exp];
                return_f64(dbl);
            }
            if (F64_SIG_FULL_BITS - bits >= F64_BITS_TO_POW10_MIN) {
                i32 exp1 = f64_bit_to_pow10_exp_table[F64_SIG_FULL_BITS - bits];
                i32 exp2 = exp - exp1;
                if (exp2 <= F64_POW10_EXP_MAX_EXACT) {
                    f64 dbl = (f64)sig;
                    dbl *= f64_pow10_table[exp1];
                    dbl *= f64_pow10_table[exp2];
                    return_f64(dbl);
                }
            }
        }
    }
#endif
    
    /*
     Slow path (accurate):
     1. Use cached diy-fp to get an approximation value.
     2. Use bigcomp to check the approximation value if needed.
     
     This algorithm refers to google's double-conversion project:
     https://github.com/google/double-conversion
     */
    {
        const i32 ERR_ULP_LOG = 3;
        const i32 ERR_ULP = 1 << ERR_ULP_LOG;
        const i32 ERR_CACHED_POW = ERR_ULP / 2;
        const i32 ERR_MUL_FIXED = ERR_ULP / 2;
        const i32 DIY_SIG_BITS = 64;
        const i32 EXP_BIAS = F64_EXP_BIAS + F64_SIG_BITS;
        const i32 EXP_SUBNORMAL = -EXP_BIAS + 1;
        
        u64 fp_err;
        u32 bits;
        i32 order_of_magnitude;
        i32 effective_significand_size;
        i32 precision_digits_count;
        u64 precision_bits;
        u64 half_way;
        
        u64 raw;
        diy_fp fp, fp_upper;
        bigint big_full, big_comp;
        i32 cmp;
        
        fp.sig = sig;
        fp.exp = 0;
        fp_err = sig_cut ? (u64)(ERR_ULP / 2) : (u64)0;
        
        /* normalize */
        bits = u64_lz_bits(fp.sig);
        fp.sig <<= bits;
        fp.exp -= bits;
        fp_err <<= bits;
        
        /* multiply and add error */
        fp = diy_fp_mul(fp, diy_fp_get_cached_pow10(exp));
        fp_err += (u64)ERR_CACHED_POW + (fp_err != 0) + (u64)ERR_MUL_FIXED;
        
        /* normalize */
        bits = u64_lz_bits(fp.sig);
        fp.sig <<= bits;
        fp.exp -= bits;
        fp_err <<= bits;
        
        /* effective significand */
        order_of_magnitude = DIY_SIG_BITS + fp.exp;
        if (likely(order_of_magnitude >= EXP_SUBNORMAL + F64_SIG_FULL_BITS)) {
            effective_significand_size = F64_SIG_FULL_BITS;
        } else if (order_of_magnitude <= EXP_SUBNORMAL) {
            effective_significand_size = 0;
        } else {
            effective_significand_size = order_of_magnitude - EXP_SUBNORMAL;
        }
        
        /* precision digits count */
        precision_digits_count = DIY_SIG_BITS - effective_significand_size;
        if (unlikely(precision_digits_count + ERR_ULP_LOG >= DIY_SIG_BITS)) {
            i32 shr = (precision_digits_count + ERR_ULP_LOG) - DIY_SIG_BITS + 1;
            fp.sig >>= shr;
            fp.exp += shr;
            fp_err = (fp_err >> shr) + 1 + ERR_ULP;
            precision_digits_count -= shr;
        }
        
        /* half way */
        precision_bits = fp.sig & (((u64)1 << precision_digits_count) - 1);
        precision_bits *= ERR_ULP;
        half_way = (u64)1 << (precision_digits_count - 1);
        half_way *= ERR_ULP;
        
        /* rounding */
        fp.sig >>= precision_digits_count;
        fp.sig += (precision_bits >= half_way + fp_err);
        fp.exp += precision_digits_count;
        
        /* get IEEE double raw value */
        raw = diy_fp_to_ieee_raw(fp);
        if (unlikely(raw == F64_RAW_INF)) return_f64(INFINITY);
        if (likely(precision_bits <= half_way - fp_err ||
                   precision_bits >= half_way + fp_err)) {
            return_f64_raw(raw); /* number is accurate */
        }
        
        /* upper boundary */
        if (raw & F64_EXP_MASK) {
            fp_upper.sig = (raw & F64_SIG_MASK) + ((u64)1 << F64_SIG_BITS);
            fp_upper.exp = (i32)((raw & F64_EXP_MASK) >> F64_SIG_BITS);
        } else {
            fp_upper.sig = (raw & F64_SIG_MASK);
            fp_upper.exp = 1;
        }
        fp_upper.exp -= F64_EXP_BIAS + F64_SIG_BITS;
        fp_upper.sig <<= 1;
        fp_upper.exp -= 1;
        fp_upper.sig += 1; /* add half ulp */
        
        /* bigint compare */
        bigint_set_buf(&big_full, sig, &exp, sig_cut, sig_end, dot_pos);
        bigint_set_u64(&big_comp, fp_upper.sig);
        if (exp >= 0) {
            bigint_mul_pow10(&big_full, +exp);
        } else {
            bigint_mul_pow10(&big_comp, -exp);
        }
        if (fp_upper.exp > 0) {
            bigint_mul_pow2(&big_comp, (u32)+fp_upper.exp);
        } else {
            bigint_mul_pow2(&big_full, (u32)-fp_upper.exp);
        }
        cmp = bigint_cmp(&big_full, &big_comp);
        if (likely(cmp != 0)) {
            /* round down or round up */
            raw += (cmp > 0);
        } else {
            /* falls midway, round to even */
            raw += (raw & 1);
        }
        
        if (unlikely(raw == F64_RAW_INF)) return_f64(INFINITY);
        return_f64_raw(raw);
    }
    
#undef return_err
#undef return_f64
#undef return_f64_raw
}
