#ifndef RESPONSE_H
#define RESPONSE_H

#include <stddef.h>

#include "../config/config.h"
#include "../utils/string/string.h"
#include "request.h"

enum my_status_code
{
    ERROR = 0,

    VALID = 200,

    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND,
    MNA,
    HVNS = 505
};

struct response
{
    char *version;
    enum my_status_code status_code;
    char *phrase;
    char *date;
    char *content_length;
    char *connection;
};

/*
 * @brief: return the response to a valid HTTP request
 *
 * @param req: the request structure to answer
 * @param config: the config file of the server
 * @param client_fd: the client socket
 */
struct response *create_response(struct request *req, struct config *config);

/*
 * @brief: destroy the response structure
 *
 * @param res: the structure to be free
 */
void response_destroy(struct response *res);

#endif /*!RESPONSE_H*/
