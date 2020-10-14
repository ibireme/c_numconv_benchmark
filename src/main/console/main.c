/*
 * Copyright (c) 2018 YaoYuan <ibireme@gmail.com>.
 * Released under the MIT license (MIT).
 */

#include <stdio.h>
#include <string.h>

extern void benchmark(const char *path);

int main(int argc, const char *argv[]) {
    
    if (argc != 3 || strcmp(argv[1], "-o") != 0 || strlen(argv[2]) < 1) {
        printf("usage: -o report.html\n");
        return 0;
    }
    
    benchmark(argv[2]);
    return 0;
}
