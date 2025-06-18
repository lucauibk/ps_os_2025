//# Terminal A
//mkfifo myfifo
//./writer
//
//# Terminal B
//./reader
// writer.c
#include <fcntl.h>
#include <unistd.h>
int main() {
    int fd = open("myfifo", O_WRONLY);
    write(fd, "Hello", 5);
    close(fd);
}
// reader.c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
int main() {
    char buf[100];
    int fd = open("myfifo", O_RDONLY);
    int n = read(fd, buf, 100);
    write(1, buf, n); // Schreibe direkt auf stdout
    close(fd);
}