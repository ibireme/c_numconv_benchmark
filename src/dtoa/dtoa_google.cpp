#include "double-conversion/double-conversion.h"
using namespace double_conversion;

/* C wrapper */
extern "C" {
char *dtoa_google(double val, char *buf) {
    StringBuilder sb(buf, 32);
    DoubleToStringConverter::EcmaScriptConverter().ToShortest(val, &sb);
    return buf + sb.position();
}
}
