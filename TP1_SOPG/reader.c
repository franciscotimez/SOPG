#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 300

#define HEADER_MSG "DATA:"
#define SIGUSR_MSG_HEADER "SIGN:"
#define SIGUSR1_MSG "SIGN:1"
#define SIGUSR2_MSG "SIGN:2"

#define LOG_FILENAME "Log.txt"
#define SIGNALS_FILENAME "Sign.txt"

int main(void)
{
    uint8_t inputBuffer[BUFFER_SIZE];
    int32_t bytesRead, returnCode, fd;

    time_t rawtime;
    struct tm *info;
    char buffer[80];

    FILE *file;

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ((returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < -1)
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
    printf("waiting for writers...\n");
    if ((fd = open(FIFO_NAME, O_RDONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("got a writer\n");

    /* Loop until read syscall returns a value <= 0 */
    do
    {
        /* read data into local buffer */
        if ((bytesRead = read(fd, inputBuffer, BUFFER_SIZE)) == -1)
        {
            perror("read");
        }
        else
        {
            inputBuffer[bytesRead] = '\0';

            printf("reader: read %d bytes: \"%s\"\n", bytesRead, inputBuffer);

            // Tomo la hora de sistema
            time(&rawtime);
            info = localtime(&rawtime);
            strftime(buffer, 80, "[%x - %X]", info);

            // Filtro Headers
            if (strstr(inputBuffer, HEADER_MSG))
            {
                // Se imprime por consola
                printf("%s << %s --> %s\n", LOG_FILENAME, buffer, &inputBuffer[strlen(HEADER_MSG)]);
                
                // Se imprime en el archivo
                file = fopen(LOG_FILENAME, "a");
                if (file)
                {
                    fprintf(file, "%s --> %s\n", buffer, &inputBuffer[strlen(HEADER_MSG)]);
                    fclose(file);
                }

            }
            else if (strstr(inputBuffer, SIGUSR_MSG_HEADER))
            {
                // Se imprime por consola
                printf("%s << %s --> %s\n", SIGNALS_FILENAME, buffer, inputBuffer);
                
                // Se imprime en el archivo
                file = fopen(SIGNALS_FILENAME, "a");
                if (file)
                {
                    fprintf(file, "%s --> %s\n", buffer, inputBuffer);
                    fclose(file);
                }
            }
        }
    } while (bytesRead > 0);

    return 0;
}
