#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

typedef char* (*transform_func)(const char*);

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <string> <plugin1.so> [plugin2.so] ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* input = argv[1];
    char* result = strdup(input);
    if (!result) {
        perror("strdup");
        return EXIT_FAILURE;
    }

    for (int i = 2; i < argc; i++) {
        void* handle = dlopen(argv[i], RTLD_NOW); // Load the shared library
        if (!handle) {
            fprintf(stderr, "dlopen error: %s\n", dlerror());
            free(result);
            return EXIT_FAILURE;
        }

        dlerror(); // Clear existing errors
        transform_func transform = (transform_func)dlsym(handle, "transform"); // Get the transform function

        char* err = dlerror();
        if (err != NULL) {
            fprintf(stderr, "dlsym error: %s\n", err);
            dlclose(handle);
            free(result);
            return EXIT_FAILURE;
        }

        char* new_result = transform(result);
        printf("%s: %s\n", argv[i], new_result);

        free(result);
        result = new_result;

        dlclose(handle);
    }

    free(result);
    return EXIT_SUCCESS;
}
