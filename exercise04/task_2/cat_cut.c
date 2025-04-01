#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage: %s <file1> <file2>\n", argv[0]);
        return 1;
    }

}