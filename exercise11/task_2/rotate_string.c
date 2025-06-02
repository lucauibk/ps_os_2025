#include <stdlib.h>
#include <string.h>

char* transform(const char* input) {
    size_t len = strlen(input);
    size_t shift = (len + 1) / 2;
    char* result = malloc(len + 1);

    for (size_t i = 0; i < len; i++) {
        result[i] = input[(i + shift) % len];
    }
    result[len] = '\0';

    return result;
}
