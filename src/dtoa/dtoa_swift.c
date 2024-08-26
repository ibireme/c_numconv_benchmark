/*
 Code from https://github.com/apple/swift/blob/main/stdlib/public/runtime/SwiftDtoa.cpp
 */
#include "SwiftDtoa.h"

char *dtoa_swift(double val, char *buf) {
    size_t len = swift_dtoa_optimal_double(val, buf, 32);
    return buf + len;
}
