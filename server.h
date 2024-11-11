#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>

#define BUFFER_SIZE 1024

// Global variables for handle_stats
extern int request_count; 
extern size_t bytes_received; 
extern size_t bytes_sent;

void send_response(int client_fd, const char *header, const char *content_type, const char *body, size_t body_len);
void handle_static(int client_fd, const char *filepath);
void handle_stats(int client_fd);
void handle_calc(int client_fd, const char *query);
void *handle_connection(void *client);

#endif // SERVER_H