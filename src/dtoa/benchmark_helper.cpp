#include "benchmark_helper.h"
#include "double-conversion/double-conversion.h"
#include "yy_test_utils.h"
#include <inttypes.h>

using namespace double_conversion;

extern "C" {

int yy_uint_to_string(uint64_t val, char *buf) {
    char *end = buf + snprintf(buf, 21, "%" PRIu64, val);
    *end = '\0';
    return (int)(end - buf);
}

int yy_sint_to_string(int64_t val, char *buf) {
    char *end = buf + snprintf(buf, 21, "%" PRIi64, val);
    *end = '\0';
    return (int)(end - buf);
}

int google_double_to_string(double val, char *buf) {
    StringBuilder sb(buf, 32);
    DoubleToStringConverter::EcmaScriptConverter().ToShortest(val, &sb);
    return (int)sb.position();
}

int google_double_to_string_prec(double val, int prec, char *buf) {
    StringBuilder sb(buf, 32);
    DoubleToStringConverter::EcmaScriptConverter().ToPrecision(val, prec, &sb);
    return (int)sb.position();
}

double google_string_to_double(const char *str, int *len) {
    static StringToDoubleConverter converter(StringToDoubleConverter::ALLOW_CASE_INSENSITIVITY,
                                             0.0, 1.0, "infinity", "nan");
    int processed = 0;
    double val = converter.StringToDouble(str, (int)strlen(str), &processed);
    *len = processed;
    return val;
}

}
