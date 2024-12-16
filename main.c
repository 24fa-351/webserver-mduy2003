#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"

#define DEFAULT_PORT 80
#define LISTEN_BACKLOG 5

int main(int argc, char* argv[])
{
    int port = DEFAULT_PORT;
    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        port = atoi(argv[2]);
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr*)&socket_address,
            sizeof(socket_address))
        < 0) {
        perror("Bind failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(socket_fd, LISTEN_BACKLOG) < 0) {
        perror("Listen failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Port: %d\n", port);
    printf("Command format: GET <command> HTTP/1.1\n");

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        int* client_fd = malloc(sizeof(int));
        *client_fd = accept(
            socket_fd, (struct sockaddr*)&client_address, &client_address_len);
        if (*client_fd < 0) {
            perror("Accept failed");
            free(client_fd);
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_connection, client_fd);
        pthread_detach(thread_id);
    }

    close(socket_fd);
    return 0;
}
