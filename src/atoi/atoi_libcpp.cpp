/*
 Use C++17 std::from_chars()
 */

#include <charconv>
#include <cstring>
#include "benchmark.h"

extern "C" {

uint32_t atoi_u32_libcpp(const char *str, size_t len, char **endptr, atoi_result *res)  {
    uint32_t val;
    std::from_chars_result r = std::from_chars(str, str + strlen(str), val, 10);
    *endptr = (char *)r.ptr;
    switch (r.ec) {
        case std::errc::result_out_of_range: *res = atoi_result_overflow; break;
        case std::errc::invalid_argument:    *res = atoi_result_fail;     break;
        default:                             *res = atoi_result_suc;      break;
    }
    return val;
}

int32_t atoi_i32_libcpp(const char *str, size_t len, char **endptr, atoi_result *res)  {
    int32_t val;
    std::from_chars_result r = std::from_chars(str, str + strlen(str), val, 10);
    *endptr = (char *)r.ptr;
    switch (r.ec) {
        case std::errc::result_out_of_range: *res = atoi_result_overflow; break;
        case std::errc::invalid_argument:    *res = atoi_result_fail;     break;
        default:                             *res = atoi_result_suc;      break;
    }
    return val;
}

uint64_t atoi_u64_libcpp(const char *str, size_t len, char **endptr, atoi_result *res)  {
    uint64_t val;
    std::from_chars_result r = std::from_chars(str, str + strlen(str), val, 10);
    *endptr = (char *)r.ptr;
    switch (r.ec) {
        case std::errc::result_out_of_range: *res = atoi_result_overflow; break;
        case std::errc::invalid_argument:    *res = atoi_result_fail;     break;
        default:                             *res = atoi_result_suc;      break;
    }
    return val;
}

int64_t atoi_i64_libcpp(const char *str, size_t len, char **endptr, atoi_result *res)  {
    int64_t val;
    std::from_chars_result r = std::from_chars(str, str + strlen(str), val, 10);
    *endptr = (char *)r.ptr;
    switch (r.ec) {
        case std::errc::result_out_of_range: *res = atoi_result_overflow; break;
        case std::errc::invalid_argument:    *res = atoi_result_fail;     break;
        default:                             *res = atoi_result_suc;      break;
    }
    return val;
}

}
