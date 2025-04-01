#include <stdio.h>
#include <stdlib.h>

typedef enum{
    ERR_NO_ARGS = 13,
    ERR_TOO_MANY_ARGS = 7,
    ERR_INVALID_OFFSET = 99,
    ERR_EXACT_ARGS = 5
} ExitCode;

int is_number(const char *str){
    while(*str){
        if(*str < '0' || *str > '9'){
            return 0;
        }
        str++;
    }
    return 1;
}

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        exit(ERR_NO_ARGS);
    }
    if(argc > 11){
        fprintf(stderr, "Too many arguments\n");
        exit(ERR_TOO_MANY_ARGS);
    }
    if((argc - 1) == 5){
        fprintf(stderr, "Exact number of arguments: %d not allowed\n", argc - 1);
        exit(ERR_EXACT_ARGS);
    }
    //setenv("OFFSET", "2", 1);
    char *offset_env = getenv("OFFSET");
    int offset = 0;
    if (offset_env) {
        if (!is_number(offset_env)) {
            fprintf(stderr, "Error: OFFSET contains invalid data.\n");
            return ERR_INVALID_OFFSET;
        }
        offset = atoi(offset_env);
    }
    int arguments = (argc - 1) + offset;
    printf("Result: %d\n", arguments);
    return EXIT_SUCCESS;
}
