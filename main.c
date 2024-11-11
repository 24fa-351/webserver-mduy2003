#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "server.h"

#define DEFAULT_PORT 50001
#define LISTEN_BACKLOG 5

int main(int argc, char *argv[])
{
    int port = DEFAULT_PORT;

    if (argc == 3 && strcmp(argv[1], "-p") == 0) // Checks if correct amount of arguments are given and if port is given
    {
        port = atoi(argv[2]);
    }
    else
    {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in socket_address;
    memset(&socket_address, '\0', sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(port);

    int returnval;
    returnval = bind(socket_fd, (struct sockaddr *)&socket_address, sizeof(socket_address));
    returnval = listen(socket_fd, LISTEN_BACKLOG);

    while (1) // While loop to accept multiple connections using threads
    {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(socket_fd, (struct sockaddr *)&client_address, &client_address_len);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handleConnection, client_fd);
        pthread_detach(thread_id);
    }

    close(socket_fd);
    return 0;
}