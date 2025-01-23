/*
 Code from https://github.com/jk-jeon/dragonbox
 
 This is the dragonbox algorithm author's code.
 But the output is always in scientific notation.
 */
#include "dragonbox/dragonbox_to_chars.h"
extern "C" {
char *dtoa_dragonbox_jk(double val, char *buf) {
    return jkj::dragonbox::to_chars(val, buf); // null-terminate
    // return jkj::dragonbox::to_chars_n(val, buf); // no null-terminate
}
}



/*
 Code from https://github.com/abolz/Drachennest
 */
#include "dragonbox.h"
extern "C" {
char *dtoa_dragonbox(double val, char *buf) {
    buf = dragonbox::Dtoa(buf, val); // need 64 bytes
    *buf = '\0';
    return buf;
}
}
