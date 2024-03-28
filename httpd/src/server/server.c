#include "server.h"

#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../daemon/daemon.h"
#include "../http/request.h"
#include "../http/response.h"
#include "../utils/variables/variables.h"

/*
 * @brief: build the response by concatenating it to the version field of
 * the struct response (for example here)
 */
static char *__respond(struct response *response)
{
    ssize_t nwrite = 0;
    char *buffer = malloc(BUFFERSIZE);
    nwrite += sprintf(buffer + nwrite, "%s %d %s\nDate: %s\n", response->version, response->status_code, response->phrase, response->date);
    if (response->status_code == VALID)
        nwrite += sprintf(buffer + nwrite, "Content_length: %s\n",
                response->content_length);
    sprintf(buffer + nwrite, "Connection: %s\n", response->connection);

    return buffer;
}

/*
 * @brief: parse the request emmited by the client, and send him the ressources
 * asked if this was a GET request
 *
 * @param client_fd: the client file descriptor
 * @param buffer: the request as a string
 * @param bytes: the length of the request
 * @param config: the config of the actual server
 */
static void respond(int client_fd, char *buffer, size_t bytes,
                    struct config *config)
{
    struct request *request = parse_request(buffer, bytes);
    struct response *response = create_response(request, config);
    char *rep = __respond(response);
    send(client_fd, rep, strlen(rep), MSG_NOSIGNAL);
    free(rep);

    if (response->status_code == VALID && request->method == GET)
    {
        send(client_fd, "\n", 1, MSG_NOSIGNAL);

        char pathname[BUFFERSIZE] = { 0 };
        char *root_dir = config->servers[0].root_dir;
        strcat(pathname, root_dir);
        strcat(pathname, request->target->data);

        int fd = open(pathname, O_RDONLY);
        ssize_t nsent;
        int total = 0;
        while (total != atoi(response->content_length))
        {
            nsent = sendfile(client_fd, fd, NULL, 512);
            if (nsent < 0)
            {
                perror(NULL);
                close(fd);
                request_destroy(request);
                response_destroy(response);
                return;
            }
            total += nsent;
        }
        close(fd);
    }
    request_destroy(request);
    response_destroy(response);
}

static void communicate(int client_fd, struct config *config)
{
    char buffer[BUFFERSIZE];
    size_t bytes = 0;
    bytes = recv(client_fd, buffer, BUFFERSIZE, 0);
    respond(client_fd, buffer, bytes, config);
}

static void start_server(int server_socket, struct config *config)
{
    if (listen(server_socket, SOMAXCONN) == -1)
        return;
    while (return_run())
    {
        int client_fd = accept(server_socket, NULL, NULL);
        if (client_fd != -1)
        {
            fprintf(stderr, "client connected\n");
            communicate(client_fd, config);
            close(client_fd);
            fprintf(stderr, "client disconnected\n");
        }
    }
    fprintf(stderr, "Have you freed all the ressources ?\n");
}

static int create_and_bind(const char *node, const char *service)
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    struct addrinfo *res = NULL;
    if (getaddrinfo(node, service, &hints, &res) == -1)
    {
        fprintf(stderr, "couldn't find a struct addrinfo with such hints\n");
        return -1;
    }
    int sock = -1;
    for (struct addrinfo *p = res; p; p = p->ai_next)
    {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1)
            continue;
        if (bind(sock, p->ai_addr, p->ai_addrlen) != -1)
            break;
        close(sock);
        sock = -1;
    }
    free(res);
    return sock;
}

/*
 * The signal handler function for the basic launch
 *
 * @param signum: the number corresponding to the signal we received.
 */
void bhandler(int signum)
{
    switch (signum)
    {
    case SIGINT:
        // STOP
        unset();
        break;
    default:
        break;
    }
}

int basic_launch(struct config *config)
{
    int server_socket =
        create_and_bind(config->servers[0].ip, config->servers[0].port);
    if (server_socket == -1)
    {
        fprintf(stderr, "could not create the server socket\n");
        return -1;
    }
    struct sigaction bsa;
    bsa.sa_flags = 0;
    bsa.sa_handler = bhandler;
    if (sigemptyset(&bsa.sa_mask) < 0)
    {
        fprintf(stderr, "error of the signal catcher\n");
        return -1;
    }
    if (sigaction(SIGINT, &bsa, NULL) < 0)
    {
        fprintf(stderr, "error of the signal catcher\n");
        return -1;
    }

    start_server(server_socket, config);

    close(server_socket);

    return 0;
}

int daemonize_launch(struct config *config)
{
    int cpid = daemonize();
    if (!cpid) // We are in the daemon
    {
        int server_socket =
            create_and_bind(config->servers[0].ip, config->servers[0].port);
        if (server_socket == -1)
        {
            fprintf(stderr, "could not create the server socket\n");
            return -1;
        }

        start_server(server_socket, config);
        close(server_socket);

        return 0;
    }
    else
        return cpid;
}
