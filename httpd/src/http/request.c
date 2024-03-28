#include "request.h"

#include <stdio.h>

/*
 * Initialisation of the request structure, each field is set to NULL
 */
static struct request *request_init(void)
{
    struct request *req = malloc(sizeof(struct request));
    if (req)
    {
        req->method = 0;
        req->target = NULL;
        req->version = NULL;
        req->content_length = NULL;
        req->host = NULL;
    }
    return req;
}

/*
 * Print all the request fields
 *
 * @param request: the request structure

void request_print(struct request *request)
{
    if (request->method == 0)
        printf("method: GET\n");
    else if (request->method == 1)
        printf("method: HEAD\n");
    else
        printf("method: INVALID\n");

    if (request->target)
        printf("target: %s\n", request->target);

    if (request->version)
        printf("version: %s\n", request->version);

    if (request->host)
        printf("Host: %s\n", request->host);

    if (request->content_length)
        printf("Content-Length: %s\n", request->content_length);
}
*/

/*
 * Auxiliary function to fill the request struct's fields corresponding
 * to the request line
 *
 * @param token: the token to match obtained with parse_request_line()
 * @param req: the request structure of the fields to fill (its address)
 */
static void __parse_request_line(struct string *token, struct request ***req,
                                 int c)
{
    if (c == 0)
    {
        if (!string_compare_n_str(token, "GET", 3))
            (**req)->method = GET;
        else if (!string_compare_n_str(token, "HEAD", 4))
            (**req)->method = HEAD;
        else
            (**req)->method = OTHER;
    }
    else if (c == 1)
        (**req)->target = string_create(token->data, token->size);
    else if (c == 2)
        (**req)->version = string_create(token->data, token->size);
}

/*
 * Parse the request line particularly
 *
 * @param req: the request struct
 * @param line: the line of the request being parsed
 */
static void parse_request_line(struct request **req, struct string *line)
{
    struct string *saveptr = NULL;
    struct string *space = string_create(" ", 1);
    struct string *token = string_tok(line, space, &saveptr);
    int c = 0;

    while (token)
    {
        __parse_request_line(token, &req, c);
        string_destroy(token);
        token = string_tok(NULL, space, &saveptr);
        c++;
    }
    string_destroy(space);
}

/*
 * Parse the headers particularly
 *
 * @param req: the request struct
 * @param line: the line of the request being parsed
 */
static void parse_headers(struct request **req, struct string *line)
{
    struct string *saveptr = NULL;
    struct string *column = string_create(":", 1);
    struct string *space = string_create(" ", 1);

    struct string *key = string_tok(line, column, &saveptr);
    struct string *value = string_tok(NULL, space, &saveptr);

    if (key && value && !string_compare_n_str(key, "Host", 4))
        (*req)->host = string_create(value->data, value->size);
    else if (key && value && !string_compare_n_str(key, "Content-Length", 14))
        (*req)->content_length = string_create(value->data, value->size);
    else
    {
        while (value)
        {
            string_destroy(value);
            value = string_tok(NULL, space, &saveptr);
        }
        string_destroy(space);
        string_destroy(column);
        string_destroy(key);
        return;
    }

    while (value)
    {
        string_destroy(value);
        value = string_tok(NULL, space, &saveptr);
    }

    string_destroy(space);
    string_destroy(column);
    string_destroy(key);
}

/*
 * Parser of the string request
 *
 * @param str: the string of the request
 */
struct request *parse_request(char *str, size_t size)
{
    struct request *res = request_init();
    if (!res)
        return NULL;

    struct string *strequest = string_create(str, size);
    struct string *pattern = string_create("\r\n", 2);
    struct string *saveptr = NULL;

    struct string *line = string_tok_pattern(strequest, pattern, &saveptr);
    if (!line)
    {
        string_destroy(strequest);
        string_destroy(pattern);
        return NULL;
    }

    parse_request_line(&res, line);
    string_destroy(line);

    while ((line = string_tok_pattern(NULL, pattern, &saveptr)))
    {
        parse_headers(&res, line);
        string_destroy(line);
    }
    string_destroy(saveptr);

    string_destroy(strequest);
    string_destroy(pattern);

    return res;
}

/*
 * Destroy the request structure
 *
 * @param request: the struct request to destroy
 */
void request_destroy(struct request *request)
{
    if (request)
    {
        string_destroy(request->target);
        string_destroy(request->version);
        string_destroy(request->host);
        string_destroy(request->content_length);
        free(request);
    }
}
