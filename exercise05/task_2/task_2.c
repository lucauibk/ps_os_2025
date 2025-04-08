#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define MEMORY_NAME "/csbb5269"

struct shared_data {
    unsigned long long result;
    unsigned long long buffer[]; // flexible array member
};

void validate_result(unsigned long long result, const unsigned long long K, const unsigned long long N) {
    for (unsigned long long i = 0; i < K; i++) {
        result -= N * (i + 1) + 17;
    }
    if(result != 0){
        printf("Checksum failed: %llu\n", result);
    }
    printf("Checksum: %llu\n", result);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s N K L\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse arguments
    unsigned long long N = strtoull(argv[1], NULL, 10);
    unsigned long long K = strtoull(argv[2], NULL, 10);
    unsigned long long L = strtoull(argv[3], NULL, 10);

    // Create shared memory object
    int shm_fd = shm_open(MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Compute required size and set it
    size_t shm_size = sizeof(struct shared_data) + L * sizeof(unsigned long long);
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate");
        shm_unlink(MEMORY_NAME);
        exit(EXIT_FAILURE);
    }

    // Map shared memory
    struct shared_data* data = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        shm_unlink(MEMORY_NAME);
        exit(EXIT_FAILURE);
    }

    unsigned long long* buffer = data->buffer;

    // Fork Producer
    pid_t producer = fork();
    if (producer == -1) {
        perror("fork");
        munmap(data, shm_size);
        shm_unlink(MEMORY_NAME);
        exit(EXIT_FAILURE);
    }

    if (producer == 0) {
        for (unsigned long long i = 0; i < K; i++) {
            buffer[i % L] = N * (i + 1) + 17;
        }
        exit(EXIT_SUCCESS);
    }

    // Fork Consumer
    pid_t consumer = fork();
    if (consumer == -1) {
        perror("fork");
        munmap(data, shm_size);
        shm_unlink(MEMORY_NAME);
        exit(EXIT_FAILURE);
    }

    if (consumer == 0) {
        unsigned long long sum = 0;
        for (unsigned long long i = 0; i < K; i++) {
            sum += buffer[i % L];
        }
        data->result = sum;
        printf("Result: %llu\n", sum);
        exit(EXIT_SUCCESS);
    }

    // Wait for children
    waitpid(producer, NULL, 0);
    waitpid(consumer, NULL, 0);

    // Validate result
    validate_result(data->result, K, N);

    // Cleanup
    munmap(data, shm_size);
    close(shm_fd);
    shm_unlink(MEMORY_NAME);
    
    return 0;
}
