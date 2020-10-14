/*
 Code from https://github.com/jk-jeon/dragonbox
 */
#include <stdio.h>
#include "dragonbox/dragonbox_to_chars.h"

extern "C" {

char *dtoa_dragonbox(double val, char *buf) {
    return jkj::dragonbox::to_chars(val, buf); // null-terminate
    // return jkj::dragonbox::to_chars_n(val, buf); // no null-terminate
}

}
