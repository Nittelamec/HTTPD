#include "response.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../utils/variables/variables.h"

/*
 * Initialisation of the response structure. Each field is set to default values
 */
static struct response *response_init(void)
{
    struct response *res = malloc(sizeof(struct response));
    if (res)
    {
        res->version = NULL;
        res->status_code = VALID;
        res->phrase = NULL;
        res->date = NULL;
        res->content_length = NULL;
        res->connection = my_strdup("close");
    }
    return res;
}

/*
 * @brief: return a string of the formatted date
 */
static char *str_time(void)
{
    time_t t = time(NULL);
    struct tm *tmp;

    tmp = localtime(&t);
    if (tmp == NULL)
    {
        perror("localtime");
        return NULL;
    }

    char *s = malloc(BUFFERSIZE);
    if (strftime(s, BUFFERSIZE, "%a, %d %b %G %T %Z", tmp) == 0)
    {
        fprintf(stderr, "strftime returned 0");
        free(s);
        return NULL;
    }
    return s;
}

/*
 * @brief: return the response to a valid HTTP request
 *
 * @param req: the request structure to answer
 * @param config: the config file of the server
 * @param client_fd: the client socket
 */
struct response *create_response(struct request *req, struct config *config)
{
    struct response *res = response_init();
    if (!req)
        res->status_code = BAD_REQUEST;

    res->date = str_time();
    res->version = my_strdup("HTTP/1.1");
    if (req)
    {
        char pathname[BUFFERSIZE] = { 0 };
        char *root_dir = config->servers[0].root_dir;
        strcat(pathname, root_dir);
        strcat(pathname, req->target->data);

        int fd;
        if ((fd = open(pathname, O_RDONLY)) < 0)
        {
            if (errno == EACCES)
            {
                res->status_code = FORBIDDEN;
                res->phrase = my_strdup("access denied");
            }
            else if (errno == ENOENT)
            {
                res->status_code = NOT_FOUND;
                res->phrase = my_strdup("not found");
            }
            else
            {
                res->status_code = ERROR;
                res->phrase = my_strdup("a general error occured");
            }
        }
        else
        {
            struct stat statbuf;
            if (fstat(fd, &statbuf) < 0)
            {
                res->status_code = ERROR;
                res->phrase = my_strdup("a general error occured");
            }
            res->content_length = malloc(32);
            sprintf(res->content_length, "%ld", statbuf.st_size);
            res->phrase = my_strdup("ok");
            close(fd);
        }
    }
    return res;
}

/*
 * @brief: destroy the response structure
 *
 * @param res: the structure to be free
 */
void response_destroy(struct response *res)
{
    if (res)
    {
        free(res->version);
        free(res->phrase);
        free(res->date);
        free(res->content_length);
        free(res->connection);
        free(res);
    }
}
