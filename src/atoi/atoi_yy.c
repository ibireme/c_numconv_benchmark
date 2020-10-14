/*
 Code from https://github.com/ibireme/yyjson
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "benchmark.h"


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

#define repeat_in_1_8(x) { x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) }

#define repeat_in_1_17(x) { x(1) x(2) x(3) x(4) x(5) x(6) x(7) \
                            x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) \
                            x(16) x(17) }

#define repeat_in_1_18(x) { x(1) x(2) x(3) x(4) x(5) x(6) x(7) \
                            x(8) x(9) x(10) x(11) x(12) x(13) x(14) x(15) \
                            x(16) x(17) x(18) }

/** Digit type */
typedef uint8_t digi_type;

/** Digit: '0'. */
static const digi_type DIGI_TYPE_ZERO       = 1 << 0;

/** Digit: [1-9]. */
static const digi_type DIGI_TYPE_NONZERO    = 1 << 1;

/** Minus sign (negative): '-'. */
static const digi_type DIGI_TYPE_NEG        = 1 << 3;


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
static yy_inline bool digi_is_type(uint8_t d, digi_type type) {
    return (digi_table[d] & type) != 0;
}

/** Match a none zero digit: [1-9] */
static yy_inline bool digi_is_nonzero(uint8_t d) {
    return digi_is_type(d, DIGI_TYPE_NONZERO);
}

/** Match a digit: [0-9] */
static yy_inline bool digi_is_digit(uint8_t d) {
    return digi_is_type(d, DIGI_TYPE_ZERO | DIGI_TYPE_NONZERO);
}

uint32_t atoi_u32_yy(const char *str, size_t len, char **endptr, atoi_result *res) {
    if (yy_unlikely(!digi_is_nonzero(*str))) {
        if (*str == '0' && !digi_is_digit(str[1])) {
            *endptr = (char *)str + 1;
            *res = atoi_result_suc;
        } else {
            *endptr = (char *)str;
            *res = atoi_result_fail;
        }
        return 0;
    }
    
    const char *cur = str;
    uint32_t val = (uint32_t)(*cur - '0'), add;
    *res = atoi_result_suc;
    
#define expr_int(i) \
    if (yy_likely((add = (uint32_t)(cur[i] - '0')) <= 9)) val = add + val * 10; \
    else goto digi_end_##i;
    repeat_in_1_8(expr_int);
#undef expr_int
    goto digi_more;
    
#define expr_end(i) \
    digi_end_##i: *endptr = (char *)cur + i; return val;
    repeat_in_1_8(expr_end)
#undef expr_end
    
digi_more:
    cur += 9;
    if (digi_is_digit(*cur)) {
        add = *cur++ - '0';
        if ((val > UINT32_MAX / 10) ||
            ((val == UINT32_MAX / 10) && (add > UINT32_MAX % 10)) ||
            digi_is_digit(*cur)) {
            while (digi_is_digit(*cur)) cur++;
            *res = atoi_result_overflow;
            *endptr = (char *)cur;
            return UINT32_MAX;;
        } else {
            *endptr = (char *)cur;
            return val * 10 + add;
        }
    } else {
        *endptr = (char *)cur;
        return val;
    }
}

int32_t atoi_i32_yy(const char *str, size_t len, char **endptr, atoi_result *res) {
    bool sign = (*str == '-');
    const char *cur = str + sign;
    
    if (yy_unlikely(!digi_is_nonzero(*cur))) {
        if (*cur == '0' && !digi_is_digit(cur[1])) {
            *endptr = (char *)cur + 1;
            *res = atoi_result_suc;
        } else {
            *endptr = (char *)cur;
            *res = atoi_result_fail;
        }
        return 0;
    }
    
    uint32_t val = (uint32_t)(*cur - '0'), add;
    *res = atoi_result_suc;
    
#define expr_int(i) \
    if (yy_likely((add = (uint32_t)(cur[i] - '0')) <= 9)) val = add + val * 10; \
    else goto digi_end_##i;
    repeat_in_1_8(expr_int);
#undef expr_int
    goto digi_more;
    
#define expr_end(i) \
    digi_end_##i: *endptr = (char *)cur + i; return (sign ? -(int32_t)val : (int32_t)val);
    repeat_in_1_8(expr_end)
#undef expr_end
    
digi_more:
    cur += 9;
    if (digi_is_digit(*cur)) {
        add = *cur++ - '0';
        if ((val > (uint32_t)INT32_MAX / 10) ||
            ((val == (uint32_t)INT32_MAX / 10) && (add > ((uint32_t)INT32_MAX % 10) + sign)) ||
            digi_is_digit(*cur)) {
            while (digi_is_digit(*cur)) cur++;
            *res = atoi_result_overflow;
            *endptr = (char *)cur;
            return sign ? INT32_MIN : INT32_MAX;
        } else {
            val = val * 10 + add;
            *endptr = (char *)cur;
            return sign ? -(int32_t)val : (int32_t)val;
        }
    } else {
        *endptr = (char *)cur;
        return sign ? -(int32_t)val : (int32_t)val;
    }
}

uint64_t atoi_u64_yy(const char *str, size_t len, char **endptr, atoi_result *res) {
    if (yy_unlikely(!digi_is_nonzero(*str))) {
        if (*str == '0' && !digi_is_digit(str[1])) {
            *endptr = (char *)str + 1;
            *res = atoi_result_suc;
        } else {
            *endptr = (char *)str;
            *res = atoi_result_fail;
        }
        return 0;
    }
    
    uint64_t num;
    const char *cur = str;
    uint64_t val = (uint64_t)(*cur - '0'), add;
    *res = atoi_result_suc;
    
#define expr_int(i) \
    if (yy_likely((add = (uint64_t)(cur[i] - '0')) <= 9)) val = add + val * 10; \
    else goto digi_end_##i;
    repeat_in_1_18(expr_int);
#undef expr_int
    goto digi_more;
    
#define expr_end(i) \
    digi_end_##i: *endptr = (char *)cur + i; return val;
    repeat_in_1_18(expr_end)
#undef expr_end
    
digi_more:
    cur += 19;
    if (digi_is_digit(*cur)) {
        add = *cur++ - '0';
        if ((val > UINT64_MAX / 10) ||
            ((val == UINT64_MAX / 10) && (add > UINT64_MAX % 10)) ||
            digi_is_digit(*cur)) {
            while (digi_is_digit(*cur)) cur++;
            *res = atoi_result_overflow;
            *endptr = (char *)cur;
            return UINT64_MAX;;
        } else {
            *endptr = (char *)cur;
            return val * 10 + add;
        }
    } else {
        *endptr = (char *)cur;
        return val;
    }
}

int64_t atoi_i64_yy(const char *str, size_t len, char **endptr, atoi_result *res) {
    bool sign = (*str == '-');
    const char *cur = str + sign;
    
    if (yy_unlikely(!digi_is_nonzero(*cur))) {
        if (*cur == '0' && !digi_is_digit(cur[1])) {
            *endptr = (char *)cur + 1;
            *res = atoi_result_suc;
        } else {
            *endptr = (char *)cur;
            *res = atoi_result_fail;
        }
        return 0;
    }
    
    uint64_t val = (uint64_t)(*cur - '0'), add;
    *res = atoi_result_suc;
    
#define expr_int(i) \
    if (yy_likely((add = (uint64_t)(cur[i] - '0')) <= 9)) val = add + val * 10; \
    else goto digi_end_##i;
    repeat_in_1_17(expr_int);
#undef expr_int
    goto digi_more;
    
#define expr_end(i) \
    digi_end_##i: *endptr = (char *)cur + i; return (sign ? -(int64_t)val : (int64_t)val);
    repeat_in_1_17(expr_end)
#undef expr_end
    
digi_more:
    cur += 18;
    if (digi_is_digit(*cur)) {
        add = *cur++ - '0';
        if ((val > (uint64_t)INT64_MAX / 10) ||
            ((val == (uint64_t)INT64_MAX / 10) && (add > ((uint64_t)INT64_MAX % 10) + sign)) ||
            digi_is_digit(*cur)) {
            while (digi_is_digit(*cur)) cur++;
            *res = atoi_result_overflow;
            *endptr = (char *)cur;
            return sign ? INT64_MIN : INT64_MIN;
        } else {
            val = val * 10 + add;
            *endptr = (char *)cur;
            return sign ? -(int64_t)val : (int64_t)val;
        }
    } else {
        *endptr = (char *)cur;
        return sign ? -(int64_t)val : (int64_t)val;
    }
}
