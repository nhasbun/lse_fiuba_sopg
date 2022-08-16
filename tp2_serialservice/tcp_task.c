#include "tcp_task.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "SerialManager.h"


#define IP_LISTEN "127.0.0.1"
#define IP_PORT 10000

static socklen_t addr_len;
static struct sockaddr_in clientaddr;
static struct sockaddr_in serveraddr;
static char buffer[128];
/* client link socket defaults to stdout before assignment */
static int link_socket_fd = -1;
static int socket_fd;
extern pthread_mutex_t lock;

// File scope fn declaration
static void socket_init();
static void on_new_message(char * msg, int len);


/**
 * @brief TCP server for incoming messages from InterfaceService.
 * 
 * @param args unused
 * @return void* unused
 */
void * tcp_task(void * args) {

    printf("Starting tcp thread... \r\n");

    socket_init();

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    // socket accept connection loop
    while(1)
    {
        pthread_mutex_lock(&lock); {
            // accept()
            addr_len = sizeof(struct sockaddr_in);
            if ( (link_socket_fd = accept(socket_fd, (struct sockaddr *)&clientaddr,&addr_len)) == -1)
                {
                perror("error en accept");
                exit(1);
            }
        }
        pthread_mutex_unlock(&lock);
        
        // Extract client address
        char ipClient[32];
        inet_ntop(AF_INET, &(clientaddr.sin_addr), ipClient, sizeof(ipClient));
        printf  ("server:  conexion desde:  %s\n",ipClient);

        uint32_t in_bytes = 1;
        while (in_bytes > 0) {
            /* We exit when connection drops and move to accept new connections. */
            if( (in_bytes = read(link_socket_fd,buffer,128)) == -1 ) {
                perror("Error leyendo mensaje en socket");
                exit(1);
            }

            buffer[in_bytes]=0x00;

            // Handling incoming message
            if (in_bytes > 0) {
                on_new_message(buffer, in_bytes);
            }
        }

        pthread_mutex_lock(&lock); {
            printf("Client connection lost...\r\n");
            close(link_socket_fd);
            link_socket_fd = -1;
        }
        pthread_mutex_unlock(&lock);
    }
}

/**
 * @brief Return current tcp socket with client.
 * 
 * @return int 
 */
int get_link_socket() {
    return link_socket_fd;
}

void tcp_on_destroy() {
    if (link_socket_fd != -1)
        close(link_socket_fd);
    
    close(socket_fd);
}


/**
 * File scope function implementations
 */

static void socket_init() {
    // Creamos socket
    socket_fd = socket(AF_INET,SOCK_STREAM, 0);

    // Cargamos datos de IP:PORT del server
    bzero((char*) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(IP_PORT);
    if(inet_pton(AF_INET, IP_LISTEN, &(serveraddr.sin_addr))<=0) {
        fprintf(stderr,"ERROR invalid server IP\r\n");
        exit(1);
    }

    // Abrimos puerto con bind()
    if (bind(socket_fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
        close(socket_fd);
        perror("listener: bind");
        exit(1);
    }

    // Seteamos socket en modo Listening
    if (listen (socket_fd, 10) == -1) {
        perror("error en listen");
        exit(1);
    }
}

static void on_new_message(char * msg, int len) {
    printf("Recibi %d bytes.:%s\n", len, msg);
    serial_send(msg, len);
}