#include <stdlib.h>
#include <string.h>

char* transform(const char* input) {
    size_t len = strlen(input);
    char* result = malloc(len + 1);

    for (size_t i = 0; i < len; i++) {
        result[i] = input[len - i - 1];
    }
    result[len] = '\0';

    return result;
}
