# Exercise Sheet 3

## Task 1

Create a program that registers a handler for the signals `SIGINT`, `SIGSTOP`, `SIGCONT`, `SIGKILL`, `SIGUSR1`, and `SIGUSR2` using `sigaction` (you **must not** use `signal`).
When a signal is received, it outputs a message indicating which signal was received.

Additionally, implement the following behavior:

- When `SIGUSR1` is received, use `sigaction` to ignore `SIGUSR2` from now on.
- When `SIGUSR2` is received, use `sigaction` to ignore `SIGUSR1` from now on.
- When `SIGINT` is received, use `sigaction` to handle `SIGUSR1` and `SIGUSR2` again.

Check every `sigaction` call for errors.

Use the following template to get started:

```C
// TODO: include necessary header files

// TODO: define signal handler

int main(void) {
    // TODO: for each signal, use sigaction to register signal handler

    const time_t work_seconds = 1;q

    struct timespec work_duration = {
        .tv_sec = work_seconds,
    };

    struct timespec remaining = {0};

    while (true) {
        // simulate real workload
        if (nanosleep(&work_duration, &remaining) == -1 && errno == EINTR) {
            work_duration = remaining;
            continue;
        }

        // restore work_duration
        work_duration.tv_sec = work_seconds;

        // TODO: more code (only if needed)
    }

    return EXIT_SUCCESS;
}
```

To make this work, you can define the `-D_DEFAULT_SOURCE` feature test macro (see `feature_test_macros(7)`).

Have a look at the man pages `sigaction(2)`, `signal(7)`, and `signal-safety(7)`.
Read also [SIG30-C](https://wiki.sei.cmu.edu/confluence/display/c/SIG30-C.+Call+only+asynchronous-safe+functions+within+signal+handlers) and [SIG31-C](https://wiki.sei.cmu.edu/confluence/display/c/SIG31-C.+Do+not+access+shared+objects+in+signal+handlers).

Note: it will be obvious from your implementation if you did not read these documents!

You can use the shell command `pkill <process_name> --signal <signal>` to send a signal to your process. Either use two terminal instances, or send your process to the background using `&`, e.g. `./signal_handler &`; the command `fg` brings it back to the foreground.

Answer the following questions in `task_1.txt` or `task_1.md`:

- What difference do you notice between `SIGINT` and `SIGKILL`?
- What can the signals `SIGSTOP` and `SIGCONT` be used for?
- What happens when you press `CTRL + c` while the program is running (as a foreground process)?
- What happens when you press `CTRL + z` while the program is running (as a foreground process)?
- Hypothetically, what would happen in your program if another signal arrives while your signal handler is still executing? Does it make a difference if it is the same signal or a different one?

## Task 2

In this task, we provide code that should do some string manipulation using `fork`.
However, this code is quite buggy.
Your task is to find the bugs in this program and fix them.
Describe the bugs you found as comments in the source code file.
Explain what was wrong and why the fixed version is correct.

In order to make presenting the results easier,
comment out the buggy lines and add the correct code,
for example:

```c
// FOUND do not compare strings with ==
// if (str1 == str2) {
if (strcmp(str1, str2) == 0) {
```

### Intended behavior

In order to fix a buggy program it is of course necessary to know what it should do.
The intended behavior is described in this section.

All command line arguments to the program (including `argv[0]`) should be processed individually.
First, all command line arguments should be copied into heap memory.
Then, a separate process is created for each command-line argument using `fork` (so in total `argc` forks).

Each fork receives a pointer to the copy of the argument it should process.
The job of the fork is shifting this argument to the right by two (2) positions (into a new array),
and converting the input string to upper case (in-place).
In order to make the shifted string accessible to the main process,
inter-process communication (such as pipes or shared memory) should be used to return the result.

Once the main process is done spawning the child processes, it waits for their completion
and for each completed process, the original argument it received, the upper-cased version,
and the shifted argument are printed.

### Debugging

The bugs in this program are mostly issues with memory accesses, such as accessing addresses that are not part of an allocation or leaking dynamically allocated memory.

One tool which can be very helpful for finding such issues is `valgrind`.
This tool can run a binary and check if it accesses any invalid memory addresses or leaks memory. To get more informative messages, make sure to pass the flag `-g` to the C compiler.

A run of `valgrind` on the corrected version of the program might look like this:

```text
$ valgrind ./task_2 some Arguments 4297
==3083991== Memcheck, a memory error detector
==3083991== Copyright (C) 2002-2022, and GNU GPL'd, by Julian Seward et al.
==3083991== Using Valgrind-3.21.0 and LibVEX; rerun with -h for copyright info
==3083991== Command: ./task_2 some Arguments 4297
==3083991== 
original argv[0]: ./task_2
uppercased: ./TASK_2
shifted: _2./task
original argv[1]: some
uppercased: SOME
shifted: meso
original argv[2]: Arguments
uppercased: ARGUMENTS
shifted: tsArgumen
original argv[3]: 4297
uppercased: 4297
shifted: 9742
Done.
==3083991== 
==3083991== HEAP SUMMARY:
==3083991==     in use at exit: 0 bytes in 0 blocks
==3083991==   total heap usage: 6 allocs, 6 frees, 1,060 bytes allocated
==3083991== 
==3083991== All heap blocks were freed -- no leaks are possible
==3083991== 
==3083991== For lists of detected and suppressed errors, rerun with: -s
==3083991== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
...
```

Note the absence of any error messages and memory leaks.
For debugging, it might be helpful to pass flags to `valgrind`,
in particular `--track-origins=yes`, `--leak-check=full`, and `--show-leak-kinds=all`.

It might also be helpful to use a debugger, either integrated in your editor/IDE or a standalone debugger like `gdb`.

## Task 3

Begin by skimming through `man pthreads`.
Take note of additional compiler flags that may be required.

### Task 3a

Write a program that initializes a global variable `accumulation` of type `int64_t` and initializes it to `0`.

The program begins by printing the value of `accumulation` and then creates a child process using fork.
The child accumulates the values from `1` to `N` (where `N` is an argument provided by the user) into `accumulation` and exits.
The parent waits for the child to exit and prints the value again.

Next, the program (now referred to as the _main thread_) spawns a single POSIX thread.
The thread does the same thing as the child process, i.e. accumulate the values from `1`to `N` (where `N` is an argument provided by the user) into the global variable and immediately exit.
The main thread waits for the thread to finish, and prints the value one more time.

What do you notice? Explain the behavior.

Make sure to check for possible errors that might occur in the program. Look at return codes of functions like `pthread_create` and print meaningful error messages to standard error using `fprintf`

### Task 3b

Write a program that receives an arbitrary number of files as arguments ([task_3](task_3/) contains such files).
Let `N` be the number of arguments provided by the user.

The program creates `N` threads, each of which is assigned an ID `i`, with `0 < i <= N`.
The `i`-th thread computes a sum of all numbers contained in the file of the `i`-th argument, i.e. the first thread computes the sum of the first file, the second thread computes the sum of the second file, and so on.

The main thread then waits for all threads to complete and prints their result _in order_ in the format `"sum <i> = <sum>"`. Lastly the main thread sums up all sums from each thread and prints the result. `"total sum = <sum>"`.

Investigate how you can pass multiple arguments to a thread function, as well as how to receive a result from it.
The program **must not** make use of any global variables.

Ensure that all threads compute their sums concurrently, not one after another.

Example output:

```sh
./task_3b ./random_one.txt ./random_two.txt ./random_three.txt ./random_four.txt ./random_five.txt ./random_six.txt ./random_seven.txt ./random_eight.txt ./random_nine.txt ./random_ten.txt 
sum 1 = 254861
sum 2 = 254608
sum 3 = 247470
sum 4 = 247841
sum 5 = 247870
sum 6 = 261918
sum 7 = 251835
sum 8 = 258556
sum 9 = 237643
sum 10 = 249088
total sum = 2511690
```

Again, make sure to check for possible errors that might occur in the program. Look at return codes of functions like `pthread_create` and print meaningful error messages.

_Hint_: If you are using memory in multiple threads, make sure it is available and doesn't run out of scope, while it might still be used in another thread.

--------------

Submit your solution as a zip archive via [OLAT](https://lms.uibk.ac.at/), structured as follows, where csXXXXXX is your UIBK login name. Your zip archive **must not** contain binaries.

```text
exc03_csXXXXXX.zip
├── Makefile             # optional
├── group.txt            # optional
├── task_1
│   ├── Makefile
│   ├── signal_handler.c
│   └── task_1.txt       # or .md
├── task_2
│   ├── Makefile
│   ├── task_2.c
└── task_3
    ├── Makefile
    ├── random_eight.txt
    ├── random_five.txt
    ├── random_four.txt
    ├── random_nine.txt
    ├── random_one.txt
    ├── random_seven.txt
    ├── random_six.txt
    ├── random_ten.txt
    ├── random_three.txt
    ├── random_two.txt
    ├── task_3a.c
    └── task_3b.c
```

Requirements

- [ ] Any implementation MUST NOT produce any additional output
- [ ] If you work in a group, create a `group.txt` file according to the format specified below
- [ ] Auto-format all source files
- [ ] Check your submission on ZID-GPL
- [ ] Check your file structure (and permissions!)
- [ ] Submit zip
- [ ] Mark solved exercises in OLAT

If you worked in a group, the `group.txt` file must be present
and have one line per student which contains the matriculation number
in the beginning, followed by a space and the student's name.
For example, if the group consists of Jane Doe,
who has matriculation number 12345678,
and Max Mustermann, who has matriculation number 87654321,
the `group.txt` file should look like this:

```text
12345678 Jane Doe
87654321 Max Mustermann
```
