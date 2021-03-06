/*
 Source: https://github.com/miloyip/itoa-benchmark/blob/master/src/unnamed.cpp
 License: https://github.com/miloyip/itoa-benchmark/blob/master/license.txt
 
 Code is modified for benchmark.
 */

#include <stdint.h>
#include <limits.h>
#include <string.h>

char *itoa_u32_unnamed(uint32_t value, char* buffer) {
    if (1000000000UL <= value) { *buffer++ = (char)((value / 1000000000UL) % 10 + '0'); }
    if ( 100000000UL <= value) { *buffer++ = (char)((value /  100000000UL) % 10 + '0'); }
    if (  10000000UL <= value) { *buffer++ = (char)((value /   10000000UL) % 10 + '0'); }
    if (   1000000UL <= value) { *buffer++ = (char)((value /    1000000UL) % 10 + '0'); }
    if (    100000UL <= value) { *buffer++ = (char)((value /     100000UL) % 10 + '0'); }
    if (     10000UL <= value) { *buffer++ = (char)((value /      10000UL) % 10 + '0'); }
    if (      1000UL <= value) { *buffer++ = (char)((value /       1000UL) % 10 + '0'); }
    if (       100UL <= value) { *buffer++ = (char)((value /        100UL) % 10 + '0'); }
    if (        10UL <= value) { *buffer++ = (char)((value /         10UL) % 10 + '0'); }

    *buffer++ = (char)(value % 10 + '0');
    /* *buffer = '\0'; */
    return buffer;
}

char *itoa_i32_unnamed(int32_t value, char* buffer) {
    if (value == INT32_MIN) {
        memcpy(buffer, "-2147483648", 11);
        return buffer + 11;
    }

    if (value < 0) {
        *buffer++ = '-';
        value = -value;
    }

    if (1000000000L <= value) { *buffer++ = (char)((value / 1000000000L) % 10 + '0'); }
    if ( 100000000L <= value) { *buffer++ = (char)((value /  100000000L) % 10 + '0'); }
    if (  10000000L <= value) { *buffer++ = (char)((value /   10000000L) % 10 + '0'); }
    if (   1000000L <= value) { *buffer++ = (char)((value /    1000000L) % 10 + '0'); }
    if (    100000L <= value) { *buffer++ = (char)((value /     100000L) % 10 + '0'); }
    if (     10000L <= value) { *buffer++ = (char)((value /      10000L) % 10 + '0'); }
    if (      1000L <= value) { *buffer++ = (char)((value /       1000L) % 10 + '0'); }
    if (       100L <= value) { *buffer++ = (char)((value /        100L) % 10 + '0'); }
    if (        10L <= value) { *buffer++ = (char)((value /         10L) % 10 + '0'); }

    *buffer++ = (char)(value % 10 + '0');
    /* *buffer = '\0'; */
    return buffer;
}

char *itoa_u64_unnamed(uint64_t value, char* buffer) {
  if ((value >> 32) == 0) {
      return itoa_u32_unnamed((uint32_t)(value), buffer);
  }

  if (10000000000000000000ULL <= value) { *buffer++ = (char)((value / 10000000000000000000ULL) % 10 + '0'); }
  if ( 1000000000000000000ULL <= value) { *buffer++ = (char)((value /  1000000000000000000ULL) % 10 + '0'); }
  if (  100000000000000000ULL <= value) { *buffer++ = (char)((value /   100000000000000000ULL) % 10 + '0'); }
  if (   10000000000000000ULL <= value) { *buffer++ = (char)((value /    10000000000000000ULL) % 10 + '0'); }
  if (    1000000000000000ULL <= value) { *buffer++ = (char)((value /     1000000000000000ULL) % 10 + '0'); }
  if (     100000000000000ULL <= value) { *buffer++ = (char)((value /      100000000000000ULL) % 10 + '0'); }
  if (      10000000000000ULL <= value) { *buffer++ = (char)((value /       10000000000000ULL) % 10 + '0'); }
  if (       1000000000000ULL <= value) { *buffer++ = (char)((value /        1000000000000ULL) % 10 + '0'); }
  if (        100000000000ULL <= value) { *buffer++ = (char)((value /         100000000000ULL) % 10 + '0'); }
  if (         10000000000ULL <= value) { *buffer++ = (char)((value /          10000000000ULL) % 10 + '0'); }
  if (          1000000000ULL <= value) { *buffer++ = (char)((value /           1000000000ULL) % 10 + '0'); }
  if (           100000000ULL <= value) { *buffer++ = (char)((value /            100000000ULL) % 10 + '0'); }
  if (            10000000ULL <= value) { *buffer++ = (char)((value /             10000000ULL) % 10 + '0'); }
  if (             1000000ULL <= value) { *buffer++ = (char)((value /              1000000ULL) % 10 + '0'); }
  if (              100000ULL <= value) { *buffer++ = (char)((value /               100000ULL) % 10 + '0'); }
  if (               10000ULL <= value) { *buffer++ = (char)((value /                10000ULL) % 10 + '0'); }
  if (                1000ULL <= value) { *buffer++ = (char)((value /                 1000ULL) % 10 + '0'); }
  if (                 100ULL <= value) { *buffer++ = (char)((value /                  100ULL) % 10 + '0'); }
  if (                  10ULL <= value) { *buffer++ = (char)((value /                   10ULL) % 10 + '0'); }

  *buffer++ = (char)(value % 10 + '0');
  //*buffer = '\0';
  return buffer;
}

char *itoa_i64_unnamed(int64_t value, char* buffer) {
    if (value == INT64_MIN) {
        memcpy(buffer, "-9223372036854775808", 20);
        return buffer + 20;
    }

    if (value >= 0 && (value >> 32) == 0) {
        return itoa_u32_unnamed((uint32_t)(value), buffer);
    }

    if (value < 0) {
        *buffer++ = '-';
        value = -value;

        if ((value >> 32) == 0) {
            return itoa_u32_unnamed((uint32_t)(value), buffer);
        }
    }

    if (1000000000000000000LL <= value) { *buffer++ = (char)((value / 1000000000000000000LL) % 10 + '0'); }
    if ( 100000000000000000LL <= value) { *buffer++ = (char)((value /  100000000000000000LL) % 10 + '0'); }
    if (  10000000000000000LL <= value) { *buffer++ = (char)((value /   10000000000000000LL) % 10 + '0'); }
    if (   1000000000000000LL <= value) { *buffer++ = (char)((value /    1000000000000000LL) % 10 + '0'); }
    if (    100000000000000LL <= value) { *buffer++ = (char)((value /     100000000000000LL) % 10 + '0'); }
    if (     10000000000000LL <= value) { *buffer++ = (char)((value /      10000000000000LL) % 10 + '0'); }
    if (      1000000000000LL <= value) { *buffer++ = (char)((value /       1000000000000LL) % 10 + '0'); }
    if (       100000000000LL <= value) { *buffer++ = (char)((value /        100000000000LL) % 10 + '0'); }
    if (        10000000000LL <= value) { *buffer++ = (char)((value /         10000000000LL) % 10 + '0'); }
    if (         1000000000LL <= value) { *buffer++ = (char)((value /          1000000000LL) % 10 + '0'); }
    if (          100000000LL <= value) { *buffer++ = (char)((value /           100000000LL) % 10 + '0'); }
    if (           10000000LL <= value) { *buffer++ = (char)((value /            10000000LL) % 10 + '0'); }
    if (            1000000LL <= value) { *buffer++ = (char)((value /             1000000LL) % 10 + '0'); }
    if (             100000LL <= value) { *buffer++ = (char)((value /              100000LL) % 10 + '0'); }
    if (              10000LL <= value) { *buffer++ = (char)((value /               10000LL) % 10 + '0'); }
    if (               1000LL <= value) { *buffer++ = (char)((value /                1000LL) % 10 + '0'); }
    if (                100LL <= value) { *buffer++ = (char)((value /                 100LL) % 10 + '0'); }
    if (                 10LL <= value) { *buffer++ = (char)((value /                  10LL) % 10 + '0'); }

    *buffer++ = (char)(value % 10 + '0');
    //*buffer = '\0';
    return buffer;
}

/* benckmark config */
int itoa_unnamed_available_32 = 1;
int itoa_unnamed_available_64 = 1;
