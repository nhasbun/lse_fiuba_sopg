/**
 * @file main.c
 * @brief LSE FIUBA SOPG CESE 17co2022 TP2 SerialService solution
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "tcp_task.h"
#include "uart_task.h"


pthread_t tcp_thread, uart_thread;


static void block_sign();
static void unblock_sign();
static void configure_signals();
static void signal_handler(int signal);


/**
 * @brief Entry point implementation routine
 * 
 * At start we configure signal handler and start serial communication with
 * hardware.
 *  
 * Then we launch a thread for handling future communications with serial port and
 * another thread for handling tcp communications with InterfaceService.
 * 
 * Main thread keep running just waiting for linux incoming signals.
 * 
 * Also signals are properly blocked before launching threads and then inmediately 
 * released.
 */
int main(void) {
    
    printf("Inicio Serial Service\r\n");

    // Configure signal handling for process
    configure_signals();

    // Linking with hardware decive
    uart_init();

    // Blocking signals before starting threads
    block_sign();

    // Launching threads
    if (pthread_create (&tcp_thread, NULL, tcp_task, NULL) != 0) {
        perror("Error creating tcp_thread...");
        exit(-1);
    };
    
    if (pthread_create (&uart_thread, NULL, uart_task, NULL) != 0) {
        perror("Error creating uart_thread...");
        exit(-1);
    };

    // Unblocking signals
    unblock_sign();

    for(;;) {
        sleep(1);
    }
    
    exit(EXIT_SUCCESS);
    return 0;
}


// File scope functions
static void block_sign() {
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

static void unblock_sign() {
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

static void configure_signals() {
    struct sigaction sa;

	sa.sa_handler = signal_handler;
	sa.sa_flags = 0; //SA_RESTART;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) != 0) {
        perror("Error registering SIGINT...");
        exit(-1);
    }
	
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        perror("Error registering SIGTERM...");
        exit(-1);
    }
}

static void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        printf("SIGINT or SIGTERM signal: %d ... \r\n", signal);
        
        tcp_on_destroy();
        printf("TCP closed...\r\n");
        uart_on_destroy();
        printf("UART closed...\r\n");
        
        // TCP is blocked in reading socket. That means that waiting for join
        // could take a long time until client sends something on tcp socket.
        pthread_join(uart_thread, NULL);
        exit(0);
    }
}
