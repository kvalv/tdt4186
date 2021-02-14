#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    unsigned int v, n;
    pid_t exit_pid;

    while (1) {
        printf("<MAIN, PID=%d> sleep time (seconds): ", getpid());
        n = scanf("%u", &v);
        if (n != 1) {
            // doesn't sett ernno if we're unable to parse properly,
            // so in this case we'll just use fprintf.
            fprintf(stderr, "Could not parse to a number.\n");
            exit(EXIT_FAILURE);
        }
        int pid = fork();
        if (pid == -1) {
            perror("Could not fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            unsigned int t_left = v;
            while((t_left = sleep(t_left)));
            printf("<PID=%d> Wake up!\n", getpid());
            exit(EXIT_SUCCESS);
        } else {
            printf("<PID=%d set in %d seconds>\n", pid, v);

            while ((exit_pid = waitpid(-1, NULL, WNOHANG))) {
                if (exit_pid == -1) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }
                printf("<MAIN> child with PID=%d exited.\n", exit_pid);
            }
        }
    }
    return 0;
}
