#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 300

#define HEADER_MSG "DATA:"
#define SIGUSR1_MSG "SIGN:1"
#define SIGUSR2_MSG "SIGN:2"

// File Descriptor de la named fifo
int32_t fd; 

void sigusr1_handler(int sig)
{
    write(fd, SIGUSR1_MSG, sizeof(SIGUSR1_MSG));
}

void sigusr2_handler(int sig)
{
   write(fd, SIGUSR2_MSG, sizeof(SIGUSR2_MSG));
}

int main(void)
{

    char outputBuffer[BUFFER_SIZE];
    uint32_t bytesWrote;
    int32_t returnCode;

    // Struct de las signals
    struct sigaction signal_usr1, signal_usr2;

    // Configuracion SIGUSR1
    signal_usr1.sa_handler = sigusr1_handler;
    signal_usr1.sa_flags = 0; //SA_RESTART;
    sigemptyset(&signal_usr1.sa_mask);

    if (sigaction(SIGUSR1, &signal_usr1, NULL) == -1)
    {
        perror("Error en sigaction!!");
        exit(1);
    }

        // Configuracion SIGUSR2
    signal_usr2.sa_handler = sigusr2_handler;
    signal_usr2.sa_flags = 0; //SA_RESTART;
    sigemptyset(&signal_usr2.sa_mask);

    if (sigaction(SIGUSR2, &signal_usr2, NULL) == -1)
    {
        perror("Error en sigaction!!");
        exit(1);
    }

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ((returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < -1)
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
    printf("waiting for readers...\n");
    if ((fd = open(FIFO_NAME, O_WRONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("got a reader--type some stuff\n");

    /* Loop forever */
    while (1)
    {
        /* Get some text from console */

        if (fgets(&outputBuffer[strlen(HEADER_MSG)], BUFFER_SIZE, stdin))
        {
            memcpy(outputBuffer, HEADER_MSG, strlen(HEADER_MSG));

            /* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
            if ((bytesWrote = write(fd, outputBuffer, strlen(outputBuffer) - 1)) == -1)
            {
                perror("write");
            }
            else
            {
                printf("writer: wrote %d bytes\n", bytesWrote);
                strcpy(outputBuffer, "");
            }
        }
    }
    return 0;
}
