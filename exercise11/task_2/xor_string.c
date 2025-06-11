#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* transform(const char* input) {
    size_t len = strlen(input);
    char* result = strdup(input);

    for (size_t i = 0; i < len; i++) {
        if (isalpha((unsigned char)input[i])) {
            result[i] = input[i] ^ 0x20;
        }
    }

    return result;
}
