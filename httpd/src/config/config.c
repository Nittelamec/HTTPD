#ifndef _POSIX_C_SOURCE
#    define _POSIX_C_SOURCE 200809L
#endif

#define _GNU_SOURCE

#include "config.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/variables/variables.h"

static struct config *config_init(void)
{
    struct config *res = malloc(sizeof(struct config));
    if (res)
    {
        res->pid_file = NULL;
        res->log_file = NULL;
        res->log = true;
        res->servers = NULL;
        res->nb_servers = 0;
    }
    return res;
}

void server_print(struct server_config server)
{
    if (server.server_name)
        printf("server_name: %s\n", server.server_name->data);
    if (server.ip)
        printf("ip: %s\n", server.ip);
    if (server.port)
        printf("port: %s\n", server.port);
    if (server.root_dir)
        printf("root_dir: %s\n", server.root_dir);
    if (server.default_file)
        printf("default_file: %s\n", server.default_file);
}

void config_print(struct config *config)
{
    if (config)
    {
        printf("[global]\n");
        printf("pid_file: %s\n", config->pid_file);
        if (config->log_file)
            printf("log_file: %s\n", config->log_file);
        (config->log) ? printf("log: true\n") : printf("log: false\n");
        printf("nb_servers: %ld\n", config->nb_servers);
        printf("\n");
        if (config->servers)
        {
            for (size_t i = 0; i < config->nb_servers; i++)
            {
                printf("[vhosts%ld]\n", i + 1);
                server_print(config->servers[i]);
                printf("\n");
            }
        }
    }
}

static size_t string_parse(char **start, char dest[BUFFERSIZE], char delim)
{
    while (**start == ' ')
        *start += 1;
    char *posdelim = strchr(*start, delim);
    size_t i = 0;
    while (*start != posdelim)
    {
        dest[i] = **start;
        *start += 1;
        i++;
    }
    dest[i] = '\0';
    *start += (delim != '\n') ? 1 : 0;
    return i;
}

static bool str_to_bool(char *str)
{
    if (!strcmp(str, "true"))
        return true;
    else if (!strcmp(str, "false"))
        return false;
    else
        return true;
}

static char *my_strndup(char *str, size_t n)
{
    char *res = malloc(n + 1);
    for (size_t i = 0; i <= n; i++)
        res[i] = str[i];
    return res;
}

static void parse_global(char *lineptr, struct config *config, int *err)
{
    char key[BUFFERSIZE];
    string_parse(&lineptr, key, ' ');

    lineptr = strchr(lineptr, ' ');

    char value[BUFFERSIZE];
    size_t len = string_parse(&lineptr, value, '\n');
    if (!strcmp(key, "pid_file"))
        config->pid_file = my_strndup(value, len);
    else if (!strcmp(key, "log_file"))
        config->log_file = my_strndup(value, len);
    else if (!strcmp(key, "log"))
        config->log = str_to_bool(value);
    else
        *err = 1;
}

static void init_serv(struct server_config *serv)
{
    serv->server_name = NULL;
    serv->port = NULL;
    serv->ip = NULL;
    serv->default_file = NULL;
    serv->root_dir = NULL;
}

static void parse_vhosts(char *lineptr, struct server_config *serv, int *err)
{
    char key[BUFFERSIZE];
    string_parse(&lineptr, key, ' ');

    lineptr = strchr(lineptr, ' ');

    char value[BUFFERSIZE];
    size_t len = string_parse(&lineptr, value, '\n');
    if (!strcmp(key, "server_name"))
    {
        struct string *str = string_create(value, len);
        serv->server_name = str;
    }
    else if (!strcmp(key, "port"))
        serv->port = my_strndup(value, len);
    else if (!strcmp(key, "ip"))
        serv->ip = my_strndup(value, len);
    else if (!strcmp(key, "root_dir"))
        serv->root_dir = my_strndup(value, len);
    else if (!strcmp(key, "default_file"))
        serv->default_file = my_strndup(value, len);
    else
        *err = 1;
}

static void are_servers_valid(struct config *config, int *err)
{
    for (size_t i = 0; i < config->nb_servers; i++)
    {
        struct server_config server = config->servers[i];
        if (!server.server_name || !server.ip || !server.port
            || !server.root_dir)
            *err = 1;
    }
}

static void is_config_valid(struct config *config, int *err)
{
    are_servers_valid(config, err);
    if (!config->pid_file || !config->servers)
        *err = 1;
}

struct config *parse_configuration(const char *path)
{
    struct config *res = config_init();
    FILE *f = fopen(path, "r");
    if (!res || !f)
        return NULL;
    char *lineptr = NULL;
    size_t n = 0;
    ssize_t line;
    int *err = malloc(sizeof(int));
    *err = 0;
    while ((line = getline(&lineptr, &n, f)) != -1)
    {
        if (!strcmp(lineptr, "[global]\n"))
        {
            line = getline(&lineptr, &n, f);
            while (line > 1)
            {
                parse_global(lineptr, res, err);
                line = getline(&lineptr, &n, f);
            }
        }
        else if (!strcmp(lineptr, "[[vhosts]]\n"))
        {
            line = getline(&lineptr, &n, f);
            res->nb_servers += 1;
            res->servers = realloc(
                res->servers, res->nb_servers * sizeof(struct server_config));
            init_serv(&(res->servers[res->nb_servers - 1]));
            while (line > 1)
            {
                parse_vhosts(lineptr, &(res->servers[res->nb_servers - 1]),
                             err);
                line = getline(&lineptr, &n, f);
            }
        }
    }
    is_config_valid(res, err);
    if (*err)
    {
        config_destroy(res);
        printf("invalid config file\n");
        res = NULL;
    }
    fclose(f);
    free(lineptr);
    free(err);
    return res;
}

static void server_config_destroy(struct server_config serv)
{
    if (serv.server_name)
        free(serv.server_name->data);
    free(serv.server_name);
    free(serv.port);
    free(serv.ip);
    free(serv.root_dir);
    free(serv.default_file);
}

void config_destroy(struct config *config)
{
    if (config)
    {
        free(config->pid_file);
        free(config->log_file);
        for (size_t i = 0; i < config->nb_servers; i++)
            server_config_destroy(config->servers[i]);
        free(config->servers);
        free(config);
    }
}
