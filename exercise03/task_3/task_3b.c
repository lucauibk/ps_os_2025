#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_pthread/_pthread_t.h>

typedef struct {
    char *filename;
    int id;
    long sum;
} ThreadData;

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        return 1;
    }
    int N = argc - 1;
    pthread_t threads[N];
    ThreadData thread_data[N];
    
}