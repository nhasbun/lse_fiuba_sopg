#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>  // unix signal c api


#define FIFO_NAME "tp1_fifo"
#define BUFFER_SIZE 300


uint8_t signal_1 = 0;
uint8_t signal_2 = 0;
int32_t fd = 0;
uint8_t fd_created = 0;


void signalHandler(int signalNum);
void fifo_write(char * output_str);


int main(void)
{
    // Setting signal handlers
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);

    char outputBuffer[BUFFER_SIZE];
    char outputBuffer2[BUFFER_SIZE+5];

    // Printing pid
    printf("Writer pid is: %d \r\n", getpid());

    /* Create named fifo. -1 means already exists so no action if already exists */
    int32_t returnCode = 0;
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1 )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
	printf("waiting for readers...\n");
	if ( (fd = open(FIFO_NAME, O_WRONLY) ) < 0 )
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }

    fd_created = 1;

    /* open syscalls returned without error -> other process attached to named fifo */
	printf("got a reader--type some stuff\n");

        /* Loop forever */
	while (1)
	{
        /* Get some text from console */
		if (fgets(outputBuffer, BUFFER_SIZE, stdin) == NULL) {
            perror("Error reading from console...");
            continue;
        }

        // Adding «DATA:» prefix
        sprintf(outputBuffer2, "DATA:%s", outputBuffer);

        fifo_write(outputBuffer2);
	}

    return 0;
}

/**
 * @brief Signal handler for SIGUSR1 (10) and SIGUSR2 (12)
 * 
 * @param signalNum posix signal id
 */
void signalHandler(int signalNum) {

    if (fd_created == 0) return;

    switch(signalNum) {
        
        case SIGUSR1:
            fifo_write("SIGN:1\n");
            break;

        case SIGUSR2:
            fifo_write("SIGN:2\n");
            break;
        
        default:
            break;
    }
}

/**
 * @brief Writing to fifo with str + "\n" format
 */
void fifo_write(char * output_str) {

        uint32_t bytesWrote;

        // Write buffer to named fifo.
		if ((bytesWrote = write(fd, output_str, strlen(output_str) - 1)) == -1)
        {
			perror("Error writing to named fifo...");
        }
        else
        {
            char msg[32] = "";
            
            if (sprintf(msg, "writer: wrote %d bytes\n", bytesWrote) == -1) {
                perror("Error using sprintf...");
            
            } else if (write(STDOUT_FILENO, msg, strlen(msg)) == -1) {
                perror("Error writing to stdout...");
            }
        }
}