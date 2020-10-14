/*
 Code from: https://github.com/erthink/erthink/blob/master/erthink_d2a.h
 See: https://erthink.github.io/dtoa-benchmark/
 */
#include "erthink_d2a.h"


extern "C" {

char *dtoa_erthink(double val, char *buf) {
    buf = erthink::d2a_accurate(val, buf);
    *buf = '\0';
    return buf;
}


}
