/*
 Code from https://github.com/jk-jeon/Grisu-Exact
 */
 
#include <stdio.h>
#include "grisu_exact.h"
#include "fp_to_chars.h"


/* C wrapper */
/* buf need 32 bytes, return end pointer */
extern "C" {
char *dtoa_grisu_exact(double val, char *buf) {
    return jkj::fp_to_chars(val, buf,
                            jkj::grisu_exact_rounding_modes::nearest_to_even{},
                            jkj::grisu_exact_correct_rounding::tie_to_even{});
}
}
