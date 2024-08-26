/*
 Code from: https://github.com/erthink/erthink/blob/master/erthink_d2a.h
 See: https://erthink.github.io/dtoa-benchmark/
 */
#include "erthink_d2a.h++"
#include "erthink_defs.h"


extern "C" {

char *dtoa_erthink(double val, char *buf) {
    buf = erthink::d2a<erthink::grisu::ieee754_default_printer<true>>(
            val, buf);
    *buf = '\0';
    return buf;
}


}
