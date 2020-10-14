#include "double-conversion/double-conversion.h"

using namespace double_conversion;

extern "C"
double strtod_google(const char *str, size_t len, char **endptr) {
    static StringToDoubleConverter converter(StringToDoubleConverter::ALLOW_CASE_INSENSITIVITY,
                                             0.0, 1.0, "infinity", "nan");
    int processed = 0;
    double val = converter.StringToDouble(str, (int)len, &processed);
    *endptr = (char *)(str + processed);
    return val;
}
