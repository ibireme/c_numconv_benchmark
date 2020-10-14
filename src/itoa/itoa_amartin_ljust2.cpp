/*
 Source: https://github.com/amdn/itoa-benchmark/blob/master/src/itoa_ljust_impl.h
 License: https://github.com/amdn/itoa_ljust/blob/master/LICENSE

 Code is modified for benchmark.
 Require C++11.
 
 MSVC cannot inline the `p10` constexpr, which makes the function very slow,
 tested in vs2017 and vs2019.
 */


/*===----------------------------------------------------------------------===*
 * itoa_ljust_impl.h - Fast integer to ascii decimal conversion
 *
 * This file is meant to be included in only one .cpp file/compilation unit.
 * Uses recursive function templates, compile with -O3 for best performance.
 *
 * The implementation strives to perform well with random input values
 * where CPU branch prediction becomes ineffective:
 *
 *   1 ) reduce the number of conditional branches used to determine
 *       the number of digits and use uninterrupted sequence of
 *       instructions to generate multiple digits, this was inspired by
 *       the implementation of FastUInt32ToBufferLeft in
 *           https://github.com/google/protobuf/blob/master/
 *                 src/google/protobuf/stubs/strutil.cc
 *
 *   2 ) avoid branches altogether by allowing overwriting of characters
 *       in the output buffer when the difference is only one character
 *          a) minus sign
 *          b) alignment to even # digits
 *
 *   3 ) use hints to the compiler to indicate which conditional branches
 *       are likely to be taken so the compiler arranges the likely
 *       case to be the fallthrough, branch not taken
 *
 * Other Performance considerations
 *
 *   4 ) use a lookup table to convert binary numbers 0..99 into 2 characters
 *       This technique is used by all fast implementations.
 *       See Andrei Alexandrescu's engineering notes
 *           https://www.facebook.com/notes/facebook-engineering/
 *                 three-optimization-tips-for-c/10151361643253920/
 *
 *   5 ) use memcpy to store 2 digits at a time - most compilers treat
 *       memcpy as a builtin/intrinsic, this lets the compiler
 *       generate a 2-byte store instruction in platforms that support
 *       unaligned access
 *
 *   6 ) use explicit multiplicative inverse to perform integer division
 *       See Terje Mathisen's algoritm in Agner Fog's
 *           http://www.agner.org/optimize/optimizing_assembly.pdf
 *       By exploiting knowledge of the restricted domain of the dividend
 *       the multiplicative inverse factor is smaller and can fit in the
 *       immediate operand of x86 multiply instructions, resulting in
 *       fewer instructions
 *
 *   7 ) inline the recursive call to FastUInt64ToBufferLeft in the original
 *       Google Protocol Buffers, as in itoa-benchmark/src/unrolledlut.cpp
 *       by nyronium@genthree.io
 *
 * Correctness considerations
 *
 *   8 ) Avoids unary minus of signed types - undefined behavior if value
 *       is INT_MIN in platforms using two's complement representation
 *
 *===----------------------------------------------------------------------===*
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2017 Arturo Martin-de-Nicolas
 * arturomdn@gmail.com
 * https://github.com/amdn/itoa_ljust/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *     The above copyright notice and this permission notice shall be included
 *     in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *===----------------------------------------------------------------------===*/


#if (__cplusplus >= 201103) || (_MSC_VER >= 1910)

#if defined(__clang__)
#   define clang_attribute __has_attribute
#else
#   define clang_attribute(x) 0
#endif

#if !defined(force_inline)
#   if clang_attribute(always_inline) || (defined(__GNUC__) && __GNUC__ >= 4)
#       define force_inline __inline__ __attribute__((always_inline))
#   elif defined(__clang__) || defined(__GNUC__)
#       define force_inline __inline__
#   elif defined(_MSC_VER) && _MSC_VER >= 1200
#       define force_inline __forceinline
#   elif defined(_MSC_VER)
#       define force_inline __inline
#   elif defined(__cplusplus) || (defined(__STDC__) && __STDC__ && \
        defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#       define force_inline inline
#   else
#       define force_inline
#   endif
#endif

#include <stdint.h>
#include <cstdint>
#include <array>
#include <algorithm>
#include <cstring> // memcpy
#include <type_traits>


#if defined(__GNUC__) || defined(__clang__)
#define likely(expr)   __builtin_expect(static_cast<bool>(expr), 1) /* Note 3 */
#define unlikely(expr) __builtin_expect(static_cast<bool>(expr), 0)
#else
#define likely(expr)   (expr)
#define unlikely(expr) (expr)
#endif

namespace itoa_ljust2 {
    
    using u32 = uint32_t;
    using u64 = uint64_t;
    
    struct Digits { /* Note 4 */
        std::array<char,200> A;
        Digits() {
            int i,d,n=0;
            std::generate(A.begin(), A.end(), [&] () {
                return i=n++,d=i/2, '0' + (i%2 ? d%10 : d/10);
            });
        }
        char const* operator()(u32 d) const { return &A[2*d]; }
    } const digits;
    
    constexpr u32 p10(u32 e) { return e ? 10*p10(e-1) : 1; }
    
    template<u32 E> u32 q10(u32 u); /* Note 6 */
    template<> u32 q10<0>(u32 u) { return u; }
    template<> u32 q10<2>(u32 u) { return ((u64)u      *  5243U) >> 19; } // u < 10^4
    template<> u32 q10<4>(u32 u) { return ((1+(u64)u) * 858993U) >> 33; } // u < 10^6
    template<> u32 q10<6>(u32 u) { return ((1+(u64)u)* 8796093U) >> 43; } // u < 10^8
    template<> u32 q10<8>(u32 u) { return ((u64)u * 1441151881U) >> 57; } // u < 2^32
    template<u32 E> u64 q10(u64 u) { return u / p10(E); }
    
    template<typename T, u32 E, typename U = T> struct QR {
        T const q;
        T const r;
        explicit QR( U u ) : q(q10<E>(u)), r(u - q * p10(E)) {}
    };
    
    template<u32 E>
    typename std::enable_if<not (E%2), char*>::type cvt(char* out, u32 u) {
        QR<u32,E-2> d{u};
        memcpy(out, digits(d.q), 2); /* Note 5 */
        return cvt<E-2>(out+2, d.r);
    }
    template<> char* cvt<0>(char* out, u32) { /* *out = '\0'; */ return out; }
    template<u32 E>
    typename std::enable_if<E%2, char*>::type cvt(char* out, u32 u) {
        QR<u32,E-1> d{u};
        char const* src = digits(d.q);
        *out = *src++;
        out += d.q > 9; /* Note 2b */
        *out++ = *src;
        return cvt<E-1>(out, d.r);
    }
    
    static force_inline char* to_dec(char* out, u32 u) { /* Note 1 */
        if (unlikely(u >= p10(8)))    return cvt<9>(out, u);
        else if (likely(u <  p10(2))) return cvt<1>(out, u);
        else if (likely(u <  p10(4))) return cvt<3>(out, u);
        else if (likely(u <  p10(6))) return cvt<5>(out, u);
        else                          return cvt<7>(out, u);
    }
    
    static force_inline char* to_dec(char* out, u64 u) { /* Note 7 */
        u32 low = static_cast<u32>(u);
        if (likely(low == u)) return to_dec(out, low);
        QR<u64,8> mid{u};
        u32 mid32 = static_cast<u32>(mid.q);
        if (likely(mid32 == mid.q)) {
            out = to_dec(out, mid32);
            return cvt<8>(out, static_cast<u32>(mid.r));
        } else {
            QR<u32,8,u64> d{mid.q};
            out = d.q < p10(2) ? cvt<1>(out, d.q) : cvt<3>(out, d.q);
            out = cvt<8>(out, d.r);
            return cvt<8>(out, static_cast<u32>(mid.r));
        }
    }
    
    template<typename T,
    typename U = typename std::enable_if<std::is_signed<T>::value,
    typename std::make_unsigned<T>::type>::type>
    static force_inline char* to_dec(char* out, T v) {
        U mask = v < 0 ? ~(U)(0) : 0;          /* Note 2a */
        U u = ((2 * (U)(v)) & ~mask) - (U)(v); /* Note 8  */
        *out = '-';
        return to_dec(out + (mask&1), u);
    }
}

/* C wrap */
extern "C" {
    char *itoa_u32_amartin_ljust2(uint32_t v, char* out) { return itoa_ljust2::to_dec(out, v); }
    char *itoa_i32_amartin_ljust2(int32_t v, char* out) { return itoa_ljust2::to_dec(out, v); }
    char *itoa_u64_amartin_ljust2(uint64_t v, char* out) { return itoa_ljust2::to_dec(out, v); }
    char *itoa_i64_amartin_ljust2(int64_t v, char* out) { return itoa_ljust2::to_dec(out, v); }
    
    int itoa_amartin_ljust2_available_32 = 1;
    int itoa_amartin_ljust2_available_64 = 1;
}

#else
#include <stdint.h>
extern "C" {
    char *itoa_u32_amartin_ljust2(uint32_t value, char *buffer) { return buffer; }
    char *itoa_i32_amartin_ljust2(int32_t value, char *buffer) { return buffer; }
    char *itoa_u64_amartin_ljust2(uint64_t value, char *buffer) { return buffer; }
    char *itoa_i64_amartin_ljust2(int64_t value, char *buffer) { return buffer; }

    /* benckmark config */
    int itoa_amartin_ljust2_available_32 = 0;
    int itoa_amartin_ljust2_available_64 = 0;
}
#endif
