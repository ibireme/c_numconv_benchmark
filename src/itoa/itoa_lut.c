/*
 Source: https://github.com/miloyip/itoa-benchmark/blob/master/src/lut.cpp
 License: https://github.com/miloyip/itoa-benchmark/blob/master/license.txt
 
 Code is modified for benchmark.
 */



#include <stdint.h>

static const char gDigitsLut[200] = {
    '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
    '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
    '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
    '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
    '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
    '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
    '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
    '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
    '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
    '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
};



// Use lookup table of two digits

char *itoa_u32_lut(uint32_t value, char* buffer) {
    char temp[10];
    char* p = temp;
    
    while (value >= 100) {
        const unsigned i = (value % 100) << 1;
        value /= 100;
        *p++ = gDigitsLut[i + 1];
        *p++ = gDigitsLut[i];
    }

    if (value < 10)
        *p++ = (char)(value) + '0';
    else {
        const unsigned i = value << 1;
        *p++ = gDigitsLut[i + 1];
        *p++ = gDigitsLut[i];
    }

    do {
        *buffer++ = *--p;
    } while (p != temp);

    // *buffer = '\0';
    return buffer;
}

char *itoa_i32_lut(int32_t value, char* buffer) {
    uint32_t u = (uint32_t)(value);
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1;
    }
    return itoa_u32_lut(u, buffer);
}

char *itoa_u64_lut(uint64_t value, char* buffer) {
    char temp[20];
    char* p = temp;
    
    while (value >= 100) {
        const unsigned i = (unsigned)(value % 100) << 1;
        value /= 100;
        *p++ = gDigitsLut[i + 1];
        *p++ = gDigitsLut[i];
    }

    if (value < 10)
        *p++ = (char)(value) + '0';
    else {
        const unsigned i = (unsigned)(value) << 1;
        *p++ = gDigitsLut[i + 1];
        *p++ = gDigitsLut[i];
    }

    do {
        *buffer++ = *--p;
    } while (p != temp);

    //*buffer = '\0';
    return buffer;
}

char *itoa_i64_lut(int64_t value, char* buffer) {
    uint64_t u = (uint64_t)(value);
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1;
    }
    return itoa_u64_lut(u, buffer);
}

/* benckmark config */
int itoa_lut_available_32 = 1;
int itoa_lut_available_64 = 1;
