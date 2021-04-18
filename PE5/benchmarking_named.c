#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

ssize_t bytes_read_this_alarm = 0;
ssize_t bytes_read_total = 0;
int seconds = 0;

// Sighandler for USR1-signal
void handle_USR1(int signum)
{
    printf("Total bytes received:\t %ld \n", bytes_read_total);
}

// Computes bandwith, aka. bytes sent since last alarm
void compute_bandwidth(int signum)
{
    printf("Bandwith: \t\t %lu B/s\n", bytes_read_this_alarm);
    bytes_read_total += bytes_read_this_alarm;
    bytes_read_this_alarm = 0;
    alarm(1);
    seconds++;
}

// Reads from fd[0] continously
int read_cummulative(int read_descriptor, int block_size)
{
    char buffer[block_size];
    ssize_t read_bytes = 0;

    alarm(1);
    while (1)
    {
        read_bytes = read(read_descriptor, buffer, block_size);
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

    unlink("named_pipe");
    if (mkfifo("named_pipe", 0777) == -1)
    {
        perror("Could not create fifo file.\n");
        return EXIT_FAILURE;
    }

    // Sets new blocksize - if provided
    long block_size = 1024;
    if (argc > 1)
    {
        block_size = atol(argv[1]);
    }
    // printf("Block size: %lu\n", block_size);
    printf("Parent PID: %d\n", getpid());
    int id = fork();

    if (id == 0)
    {
        int fd = open("named_pipe", O_WRONLY);
        while (1)
        {
            write(fd, "", block_size);
        }
    }
    else
    {
        signal(SIGALRM, compute_bandwidth);
        signal(SIGUSR1, handle_USR1);
        int fd = open("named_pipe", O_RDONLY);
        read_cummulative(fd, block_size);
    }
}
