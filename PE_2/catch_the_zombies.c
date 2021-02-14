#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

// We assume we don't have to do any user input validation, such
// as if the user passes in a string "abc" instead of a number.

int main() {
    unsigned int v;
    pid_t exit_pid;
    while (1) {
        printf("<MAIN, PID=%d> sleep time (seconds): ", getpid());
        scanf("%d", &v);
        int pid = fork();
        if (pid == 0) {
            sleep(v);
            printf("\n<PID=%d> Wake up!\n\n", getpid());
            exit(EXIT_SUCCESS);
        }
        printf("<PID=%d set in %u seconds>\n", pid, v);

        while ((exit_pid = waitpid(-1, NULL, WNOHANG))) {
            printf("<MAIN> child with PID=%d exited.\n", exit_pid);
        }
    }
    return 0;
}
