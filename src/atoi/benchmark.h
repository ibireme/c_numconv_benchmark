#ifndef benchmark_h
#define benchmark_h

#include <stdint.h>
#include <stddef.h>

/**
 Function prototypes to convert string to integer.
 The input parameters should not be NULL.
 If thr result is not suc, return value is undefined.
 */

typedef enum {
    atoi_result_suc = 0,
    atoi_result_fail = 1,
    atoi_result_overflow = 2,
} atoi_result;

static inline const char *atoi_result_desc(atoi_result res) {
    switch (res) {
        case atoi_result_suc: return "suc";
        case atoi_result_fail: return "fail";
        case atoi_result_overflow: return "overflow";
        default: return "unknown";
    }
}

typedef uint32_t (*atoi_u32_func)(const char *str, size_t len, char **endptr, atoi_result *res);
typedef int32_t (*atoi_i32_func)(const char *str, size_t len, char **endptr, atoi_result *res);
typedef uint64_t (*atoi_u64_func)(const char *str, size_t len, char **endptr, atoi_result *res);
typedef int64_t (*atoi_i64_func)(const char *str, size_t len, char **endptr, atoi_result *res);


#endif /* benchmark_h */
