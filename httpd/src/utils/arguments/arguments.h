#ifndef ARGUMENTS_H
#define ARGUMENTS_H

enum my_options
{
    DEFAULT = -1,

    STOP = 0,
    START,
    RELOAD,
    RESTART
};

struct args
{
    int dry;
    int daemon;
    enum my_options option;
    char *config;

    char valid;
};

/*
 * @brief: parse the arguments given to the httpd binary
 *
 * @param args: a string of the arguments
 */
struct args *parse_arguments(int argc, char *argv[]);

/*
 * @brief: destroy the args strucuture
 */
void args_destroy(struct args *args);

#endif /*!ARGUMENTS_H*/
