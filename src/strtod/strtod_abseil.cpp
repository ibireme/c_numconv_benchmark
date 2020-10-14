#include "absl/strings/charconv.h"
#include "absl/strings/numbers.h"

extern "C"
double strtod_abseil(const char *str, size_t len, char **endptr) {
    double val;
    auto res = absl::from_chars(str, str + len, val);
    *endptr = (char *)res.ptr;
    return val;
}
