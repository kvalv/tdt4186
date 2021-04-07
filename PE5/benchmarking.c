#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

// a. Pipeline functionality (3 points)
// Write the program using unnamed pipes (system call pipe(2)) and output
// the cumulative number of received bytes after each read call of
// the parent process

void read_cummulative(int read_descriptor, int block_size)
{
    char read_chars[2048];
    ssize_t read_bytes = 0;
    ssize_t cummulative = 0;
    while (1)
    {
        read_bytes = read(read_descriptor, &read_chars, block_size);
        if (read_bytes > 0)
        {
            cummulative += read_bytes;
            // Task a) Print cummulative bytes
            printf("Bytes read:\t%lu\n", cummulative);
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

    int block_size = 1024;
    if (argc > 1)
    {
        block_size = atoi(argv[1]);
    }
    printf("Block size: %d\n", block_size);

    if (pipe(fd) == -1)
    {
        perror("Error opening pipe: ");
    }

    int id = fork();
    ssize_t wrote = 0;

    if (id == 0)
    {
        while (1)
        {

            write(fd[1], "", block_size);
        }

        //printf("pid:\t%d\t i:\t%d\n", id, i);
    }
    else
    {
        read_cummulative(fd[0], block_size);
    }

    return EXIT_SUCCESS;
}
