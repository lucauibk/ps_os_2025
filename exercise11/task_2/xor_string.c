#include <stdlib.h>
#include <string.h>

char* transform(const char* input) {
    size_t len = strlen(input);
    char* result = strdup(input);

    for (size_t i = 0; i < len; i++) {
        result[i] = input[i] ^ 0x20;
    }

    return result;
}
