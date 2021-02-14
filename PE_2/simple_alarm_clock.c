#include <stdio.h>
#include <unistd.h>

// We assume we don't have to do any user input validation, such
// as if the user passes in a string "abc" instead of a number.

int main() {
    unsigned int v;
    while (1) {
        printf("how long to sleep? ");
        scanf("%d", &v);
        sleep(v);
        printf("Wake up!\n\n");
    }
    return 0;
}
