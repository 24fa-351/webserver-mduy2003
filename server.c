#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "server.h"

void send_response(int client_fd, const char *header, const char *content_type, const char *body, size_t body_len)
{
}
void handle_static(int client_fd, const char *filepath)
{
}
void handle_stats(int client_fd)
{
}
void handle_calc(int client_fd, const char *query)
{
}

void *handleConnection(void *a_client)
{
    int client_fd = *((int *)a_client);
    free(a_client);

    char buffer[BUFFER_SIZE];
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0)
    {
        close(client_fd);
        return NULL;
    }
    buffer[bytes_read] = '\0';

    pthread_mutex_lock(&stats_mutex);
    request_count++;
    bytes_received += bytes_read;
    pthread_mutex_unlock(&stats_mutex);

    char method[16], path[256], protocol[16];
    sscanf(buffer, "%15s %255s %15s", method, path, protocol);

    if (strcmp(method, "GET") != 0)
    {
        const char *method_not_allowed = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        send_response(client_fd, "HTTP/1.1 405 Method Not Allowed", "text/html", method_not_allowed, strlen(method_not_allowed));
        close(client_fd);
        return NULL;
    }

    if (strncmp(path, "/static", 7) == 0)
    {
        handle_static(client_fd, path + 7);
    }
    else if (strcmp(path, "/stats") == 0)
    {
        handle_stats(client_fd);
    }
    else if (strncmp(path, "/calc", 5) == 0)
    {
        char *query = strchr(path, '?');
        if (query)
        {
            handle_calc(client_fd, query + 1);
        }
        else
        {
            const char *bad_request = "<html><body><h1>400 Bad Request</h1></body></html>";
            send_response(client_fd, "HTTP/1.1 400 Bad Request", "text/html", bad_request, strlen(bad_request));
        }
    }
    else
    {
        const char *not_found = "<html><body><h1>404 Not Found</h1></body></html>";
        send_response(client_fd, "HTTP/1.1 404 Not Found", "text/html", not_found, strlen(not_found));
    }

    close(client_fd);
    return NULL;
}
