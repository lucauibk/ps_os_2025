#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int is_vowel(char c) {
    c = tolower(c);
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
}

char* transform(const char* input) {
    size_t len = strlen(input);
    char* result = malloc(len + 1);
    size_t j = 0;

    for (size_t i = 0; i < len; i++) {
        if (!is_vowel(input[i])) {
            result[j++] = input[i];
        }
    }
    result[j] = '\0';
    return result;
}
