#include "arguments.h"

#include <stdlib.h>
#include <string.h>

#include "../string/string.h"

/*
 * @brief: Initialize an args struct with default values
 */
static struct args *args_init(void)
{
    struct args *args = malloc(sizeof(struct args));
    if (args)
    {
        args->dry = -1;
        args->daemon = -1;
        args->option = DEFAULT;
        args->config = NULL;

        args->valid = 1;
    }
    return args;
}

/*
 * @bief: return -1 if the char belongs to the option, or its integer in the
 * my_option enum otherwise
 */
static int is_option(char *str)
{
    if (!strcmp(str, "stop"))
        return 0;
    else if (!strcmp(str, "start"))
        return 1;
    else if (!strcmp(str, "reload"))
        return 2;
    else if (!strcmp(str, "restart"))
        return 3;
    else
        return -1;
}

/*
 * @brief: fill the fields of the args structure
 *
 * @param argv: a list of arguments
 * @param i: the address of the index in argv
 * @param args: the address of the structure to fill
 */
static void __parse_arguments(char *argv[], int *i, struct args **args)
{
    int dry = (*args)->dry;
    int daemon = (*args)->daemon;
    enum my_options option = (*args)->option;
    char *config = (*args)->config;

    int isopt = is_option(argv[*i]);

    if (!strcmp(argv[*i], "--dry-run"))
    {
        if (dry > 0 || daemon > 0 || option != DEFAULT || config)
            *i = -1;
        else
            (*args)->dry = 1;
    }
    else if (!strcmp(argv[*i], "-a"))
    {
        if (daemon > 0 || option != DEFAULT || config)
            *i = -1;
        else
            (*args)->daemon = 1;
    }
    else if (isopt >= 0)
    {
        if (option != DEFAULT || daemon < 0 || config)
            *i = -1;
        else
            (*args)->option = isopt;
    }
    else
    {
        if (config)
            *i = -1;
        else
            (*args)->config = my_strdup(argv[*i]);
    }
}

/*
 * @brief: return 1 if the struct is well filled, 0 otherwise
 */
static int args_is_valid(struct args *args)
{
    int valid_conf = (args->config != NULL);
    int valid_daemon = ((args->daemon > 0 && args->option != DEFAULT)
                        || (args->daemon < 0 && args->option == DEFAULT));
    return valid_conf && valid_daemon;
}

/*
 * @brief: parse the arguments given to the httpd binary
 *
 * @param argc: number of arguments passed to the binary + 1
 * @param argv: a string of the arguments
 */
struct args *parse_arguments(int argc, char *argv[])
{
    struct args *args = args_init();
    if (!args)
        return NULL;

    int i = 1;
    while (i && i < argc)
    {
        __parse_arguments(argv, &i, &args);
        i++;
    }
    if (!i || !args_is_valid(args))
        args->valid = 0;

    return args;
}

/*
 * @brief: destroy the args strucuture
 */
void args_destroy(struct args *args)
{
    if (args)
    {
        free(args->config);
        free(args);
    }
}
