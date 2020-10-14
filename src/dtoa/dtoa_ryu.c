/*
 Code from https://github.com/ulfjack/ryu
 */
#ifndef _MSC_VER

#include "ryu/ryu.h"

char *dtoa_ryu(double val, char *buf) {
    int idx = d2s_buffered_n(val, buf);
    buf[idx] = '\0';
    return buf + idx;
}

#endif