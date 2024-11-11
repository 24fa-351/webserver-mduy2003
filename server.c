#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "server.h"

int request_count = 0;
size_t bytes_received = 0;
size_t bytes_sent = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER; // Ensures access to global variables is thread-safe

void send_response(int client_fd, const char *header, const char *content_type, const char *body, size_t body_len)
{
    char response[BUFFER_SIZE];
    int header_len = snprintf(response, sizeof(response),
                              "%s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "\r\n",
                              header, content_type, body_len);
    write(client_fd, response, header_len);
    write(client_fd, body, body_len);

    pthread_mutex_lock(&stats_mutex);
    bytes_sent += header_len + body_len;
    pthread_mutex_unlock(&stats_mutex);
}

void handle_static(int client_fd, const char *file_path)
{
    char path[BUFFER_SIZE] = "./static";
    strncat(path, file_path, sizeof(path) - strlen(path) - 1);

    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        const char *not_found = "<html><body><h1>404 Not Found</h1></body></html>";
        send_response(client_fd, "HTTP/1.1 404 Not Found", "text/html", not_found, strlen(not_found));
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char *file_data = malloc(file_size);
    fread(file_data, 1, file_size, file);
    fclose(file);

    send_response(client_fd, "HTTP/1.1 200 OK", "application/octet-stream", file_data, file_size);
    free(file_data);
}

void handle_stats(int client_fd)
{
    char response_body[BUFFER_SIZE];
    pthread_mutex_lock(&stats_mutex);
    int body_len = snprintf(response_body, sizeof(response_body),
                            "<html><body><h1>Server Stats</h1>"
                            "<p>Requests: %d</p>"
                            "<p>Bytes Received: %zu</p>"
                            "<p>Bytes Sent: %zu</p>"
                            "</body></html>",
                            request_count, bytes_received, bytes_sent);
    pthread_mutex_unlock(&stats_mutex);

    send_response(client_fd, "HTTP/1.1 200 OK", "text/html", response_body, body_len);
}

void handle_calc(int client_fd, const char *query)
{
    int a = 0, b = 0;
    sscanf(query, "a=%d&b=%d", &a, &b);
    int result = a + b;

    char response_body[BUFFER_SIZE];
    int body_len = snprintf(response_body, sizeof(response_body),
                            "<html><body><h1>Calculation Result</h1>"
                            "<p>%d + %d = %d</p>"
                            "</body></html>",
                            a, b, result);

    send_response(client_fd, "HTTP/1.1 200 OK", "text/html", response_body, body_len);
}

void *handle_connection(void *client)
{
    int client_fd = *((int *)client);
    free(client);

    char buffer[BUFFER_SIZE];
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0)
    {
        close(client_fd);
        return NULL;
    }
    buffer[bytes_read] = '\0';

    // Updates the stats variables
    pthread_mutex_lock(&stats_mutex);
    request_count++;
    bytes_received += bytes_read;
    pthread_mutex_unlock(&stats_mutex);

    
    char method[METHOD_SIZE], path[PATH_SIZE], protocol[PROTOCOL_SIZE];
    sscanf(buffer, "%15s %255s %15s", method, path, protocol);

    // Have to use GET inorder to use the functions
    if (strcmp(method, "GET") != 0)
    {
        const char *method_not_allowed = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        send_response(client_fd, "HTTP/1.1 405 Method Not Allowed", "text/html", method_not_allowed, strlen(method_not_allowed));
        close(client_fd);
        return NULL;
    }

    if (strncmp(path, "/static", STATIC_LEN) == 0)
    {
        handle_static(client_fd, path + STATIC_LEN);
    }
    else if (strcmp(path, "/stats") == 0)
    {
        handle_stats(client_fd);
    }
    else if (strncmp(path, "/calc", CALC_LEN) == 0)
    {
        char *query = strchr(path, '?'); // Usage of calc: /calc?a=1&b=2
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
