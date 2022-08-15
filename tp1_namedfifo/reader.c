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


const char * data_prefix = "DATA";
const char * signal_prefix = "SIGN";


int main(void)
{
	uint8_t inputBuffer[BUFFER_SIZE];
	int32_t bytesRead, returnCode, fd;
    bytesRead = 1;
    
    /* Create named fifo. -1 means already exists so no action if already exists */
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1  )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    // Opening writing files Sign.txt and Log.txt
    FILE * sign_file=fopen("Sign.txt", "a");
    if (sign_file == NULL) perror("Error opening Sign file.");

    FILE * log_file=fopen("Log.txt", "a");
    if (log_file == NULL) perror("Error opening Log file.");
    
    /* Open named fifo. Blocks until other process opens it */
	printf("waiting for writers...\n");
	if ( (fd = open(FIFO_NAME, O_RDONLY) ) < 0 )
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }
    
    /* open syscalls returned without error -> other process attached to named fifo */
	printf("got a writer\n");

    /* Loop until read syscall returns a value <= 0 */
	while (bytesRead > 0) {

        /* read data into local buffer */
		if ((bytesRead = read(fd, inputBuffer, BUFFER_SIZE)) == -1) {
			perror("read");
        
        } else {

			inputBuffer[bytesRead] = '\0';
			printf("reader: read %d bytes: \"%s\"\n", bytesRead, inputBuffer);

            if (strncmp(data_prefix, inputBuffer, 4) == 0) {
                fprintf(log_file, "%s\r\n", inputBuffer + 5);
                fflush(log_file);
            }

            if (strncmp(signal_prefix, inputBuffer, 4) == 0) {
                fprintf(sign_file, "%s\r\n", inputBuffer + 5);
                fflush(sign_file);
            }
		}
    }

	return 0;
}
