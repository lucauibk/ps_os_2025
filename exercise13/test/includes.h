#ifndef COMMON_INCLUDES_H
#define COMMON_INCLUDES_H

#include <stdio.h>      // printf, fprintf, snprintf, FILE, stdin, stdout, stderr
#include <stdlib.h>     // malloc, free, exit, atoi, rand, qsort
#include <string.h>     // strcpy, strlen, strcmp, memcpy, memset, strcat
#include <stdbool.h>    // bool, true, false
#include <stdint.h>     // int32_t, uint64_t, intptr_t etc.
#include <ctype.h>      // isdigit, isalpha, tolower, toupper
#include <errno.h>      // errno, perror, error codes
#include <time.h>       // time, clock, struct tm, nanosleep
#include <math.h>       // sin, cos, sqrt, pow, fabs
#include <pthread.h>    // pthread_create, mutex, condition variables
#include <unistd.h>     // sleep, usleep, fork, pipe, getpid
#include <fcntl.h>      // open, O_CREAT, O_RDWR etc.
#include <sys/types.h>  // pid_t, ssize_t, off_t
#include <sys/stat.h>   // stat, fstat, chmod
#include <signal.h>     // signal handling, sigaction
#include <assert.h>     // assert()

#endif // COMMON_INCLUDES_H
