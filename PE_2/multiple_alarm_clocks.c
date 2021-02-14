#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

// We assume we don't have to do any user input validation, such
// as if the user passes in a string "abc" instead of a number.

int main() {
    unsigned int v;
    while (1) {
        printf("<MAIN> How long to sleep? ");
        scanf("%d", &v);
        int pid = fork();
        if (pid == 0) {
            sleep(v);
            printf("\n<PID=%d> Wake up!\n\n", getpid());
            exit(EXIT_SUCCESS);
        } else {
            printf("<PID=%d set in %d seconds>\n", pid, v);
        }
    }
    return 0;
}
