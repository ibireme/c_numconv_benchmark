#define FMT_HEADER_ONLY 1
#include "fmt/core.h"
#include "fmt/compile.h"

/* C wrapper */
extern "C" {
char *dtoa_fmtlib(double val, char *buf) {
    buf = fmt::format_to(buf, FMT_COMPILE("{}"), val);
    *buf = '\0';
    return buf;
}
}
