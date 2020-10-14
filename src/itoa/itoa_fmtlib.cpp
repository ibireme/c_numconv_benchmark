/*
 Project: https://github.com/fmtlib/fmt
 */

#define FMT_HEADER_ONLY 1
#include "fmt/core.h"
#include "fmt/compile.h"

/* C wrap */
extern "C" {
char *itoa_u32_fmtlib(uint32_t val, char* buf) {
    return fmt::format_to(buf, FMT_COMPILE("{}"), val);
}
char *itoa_i32_fmtlib(int32_t val, char* buf) {
    return fmt::format_to(buf, FMT_COMPILE("{}"), val);
}
char *itoa_u64_fmtlib(uint64_t val, char* buf) {
    return fmt::format_to(buf, FMT_COMPILE("{}"), val);
}
char *itoa_i64_fmtlib(int64_t val, char* buf) {
    return fmt::format_to(buf, FMT_COMPILE("{}"), val);
}

/* benckmark config */
int itoa_fmtlib_available_32 = 1;
int itoa_fmtlib_available_64 = 1;
}
