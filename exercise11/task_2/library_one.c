#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* transform(const char* input) {
    size_t len = strlen(input);
    char* result = strdup(input);

    for (size_t i = 0; i < len; i++) {
        if (isupper(result[i]))
            result[i] = tolower(result[i]);
        else if (islower(result[i]))
            result[i] = toupper(result[i]);
    }

    return result;
}
