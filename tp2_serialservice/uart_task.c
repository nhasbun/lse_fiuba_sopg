#include "uart_task.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "SerialManager.h"
#include "tcp_task.h"


uint8_t run_uart_loop = 1;


/**
 * @brief Serial listener for incoming hardware commands.
 * 
 * @param args unused
 * @return void* unused
 */
void * uart_task(void * args) {

    printf("Starting uart thread... \r\n");

    while(run_uart_loop) {

        char buf[128];
        char temp;
        int res = serial_receive(buf, 20);

        if (res > 0) {
            buf[res] = 0;
            printf("Reception bytes: %d with info: %s \r\n", res, buf);

            // Delivering hw commands incoming from controller to tcp client
            int link_socket = get_link_socket();
            if (write (link_socket, buf, res) == -1) {
      			perror("Error escribiendo mensaje en socket. Skipping.");
    		}
        }

        usleep(100*1000);
    }
}

void uart_init() {
    
    int res = serial_open(0, 0);

    if (res != 0) {
    	perror("Error opening serial port");
    	exit(-1);
    }
}

void uart_on_destroy() {
    serial_close();
    run_uart_loop = 0;
}