#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* transform(const char* input) {
    size_t len = strlen(input);
    char* result = strdup(input);

    for (size_t i = 0; i < len; i++) {
        char c = input[i];
        if ('A' <= c && c <= 'Z')
            result[i] = 'A' + (c - 'A' + 13) % 26;
        else if ('a' <= c && c <= 'z')
            result[i] = 'a' + (c - 'a' + 13) % 26;
    }

    return result;
}
