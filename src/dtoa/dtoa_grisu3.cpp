/*
 Code from https://github.com/abolz/Drachennest
 */

#include "grisu3.h"

extern "C" {
char *dtoa_grisu3(double val, char *buf) {
    buf = grisu3::Dtoa(buf, val); // need 64 bytes
    *buf = '\0';
    return buf;
}
}
