#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "echo.h"

void *handleConnection(void *a_client)
{
    int client_fd = *((int *)a_client);
    free(a_client);

    char buffer[BUFFER_SIZE];
    while (1) // While loop to constantly receive and echo messages from one connection
    {
        int bytes_read = read(client_fd, buffer, sizeof(buffer));
        if (bytes_read <= 0) // If the read fails or breaks, connection is closed
        {
            break;
        }
        buffer[bytes_read] = '\0';
        printf("Received: %s", buffer);       
        write(client_fd, buffer, bytes_read);
    }

    close(client_fd);
    return NULL;
}