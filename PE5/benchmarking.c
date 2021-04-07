#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

// Global variables
// Used inside SIGALRM-handler
ssize_t bytes_read_this_alarm = 0;
ssize_t bytes_read_total = 0;
int seconds = 0;

void compute_bandwidth(int signum)
{
    //printf("Bandwidth:\t %luB/s\n", bytes_read_this_alarm);

    bytes_read_total += bytes_read_this_alarm;
    bytes_read_this_alarm = 0;
    alarm(1);
    seconds++;

    //
    //  Lines underneath prints mean, used to compare bandwith by block sizes
    //
    // if ((seconds % 5) == 0)
    // {
    //     long mean = bytes_read_total / seconds;
    //     printf("Mean:\t\t %lu\n", mean);
    // }
}

void read_cummulative(int read_descriptor, int block_size)
{
    char *read_chars;
    ssize_t read_bytes = 0;
    signal(SIGALRM, compute_bandwidth);

    alarm(1);
    while (1)
    {
        read_bytes = read(read_descriptor, &read_chars, block_size);
        if (read_bytes > 0)
        {
            bytes_read_this_alarm += read_bytes;
            // Task a) Print cummulative bytes
            // printf("Bytes read:\t%lu\n", bytes_read_this_alarm);
        }
        else if (read_bytes == 0)
        {
            printf("EOF\n");
        }
        else if (read_bytes < 0)
        {
            perror("Failed reading from fd[0]: ");
        }
    }
}

int main(int argc, char const *argv[])
{
    // File descriptors used in pipe
    int fd[2]; // fd[0] - read,    fd[1] - write

    // Sets new blocksize - if provided
    long block_size = 1024;
    if (argc > 1)
    {
        block_size = atol(argv[1]);
    }
    printf("Block size: %lu\n", block_size);

    // Create unnamed pipe
    if (pipe(fd) == -1)
    {
        perror("Error opening pipe: ");
    }

    int id = fork();

    if (id == 0)
    {
        while (1)
        {
            write(fd[1], "", block_size);
        }
    }
    else
    {
        read_cummulative(fd[0], block_size);
    }

    return EXIT_SUCCESS;
}
