#define _GNU_SOURCE
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>

static void* (*real_malloc)(size_t) = NULL;

char* append_number(char* dst, size_t number, size_t len) {
    if (len == 0) {
        return dst;
    }

    if (number > 9) { // If the number has more than one digit, recursively append the higher digits first
        char* new_dst = append_number(dst, number / 10, len);
        if (*new_dst != '\0') {
            return new_dst;
        }

        len -= (new_dst - dst);
        dst = new_dst;
    }

    if (len-- > 0) { 
        *dst = '0' + number % 10;
    }

    if (len > 0) { //
        *(++dst) = '\0';
    }

    return dst;
}

void* malloc(size_t size) {
    if (!real_malloc) {
        real_malloc = dlsym(RTLD_NEXT, "malloc");
    }

    // Construct message: "allocating <size> bytes\n"
    char msg[64] = {0};
    char* ptr = msg;

    ptr = stpncpy(ptr, "allocating ", sizeof(msg));
    ptr = append_number(ptr, size, sizeof(msg) - (ptr - msg));
    ptr = stpncpy(ptr, " bytes\n", sizeof(msg) - (ptr - msg));

    write(STDOUT_FILENO, msg, strlen(msg));

    return real_malloc(size);
}
