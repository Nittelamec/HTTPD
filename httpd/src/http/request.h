#ifndef REQUEST_H
#define REQUEST_H

#include "../utils/string/string.h"

enum method
{
    GET = 0,
    HEAD,
    OTHER
};

struct request
{
    enum method method;
    struct string *target;
    struct string *version;
    struct string *content_length;
    struct string *host;
};

/*
 * Parser of the request
 *
 * @param str: the string of the request
 */
void request_destroy(struct request *request);

struct request *parse_request(char *str, size_t size);

#endif /*!REQUEST_H*/
