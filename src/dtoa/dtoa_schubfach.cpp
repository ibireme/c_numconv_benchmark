/*
 Code from https://github.com/abolz/Drachennest
 */

#include "schubfach_64.h"

extern "C" {
char *dtoa_schubfach(double val, char *buf) {
    buf = schubfach::Dtoa(buf, val); // need 64 bytes
    *buf = '\0';
    return buf;
}
}
