#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef SERVER_H
#define SERVER_H

#include "../config/config.h"

int daemonize_launch(struct config *config);
int basic_launch(struct config *config);

#endif /*!SERVER_H*/
