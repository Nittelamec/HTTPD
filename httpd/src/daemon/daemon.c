#include "daemon.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * The signal handler function.
 *
 * @param signum: the number corresponding to the signal we received.
 */
void handler(int signum)
{
    switch (signum)
    {
        case SIGUSR1:
        // do something to make the server understand to SIGINT
        break;
        case SIGUSR2:
        // RELOAD
        break;
    }
}

/*
 * Daemonize the server to run it in background
 */
int daemonize(void)
{
    pid_t cpid;
    cpid = fork();

    if (cpid == -1)
    {
        // Failed to create new process
        return -1;
    }
    else if (!cpid)
    {
        // We are in the daemon
        struct sigaction sa;
        sa.sa_flags = 0; // Nothing special to do
        sa.sa_handler = handler;

        // Initialize mask
        if (sigemptyset(&sa.sa_mask) < 0)
        {
            return -1;
        }
        if (sigaction(SIGUSR1, &sa, NULL) < 0)
        {
            return -1;
        }
        if (sigaction(SIGUSR2, &sa, NULL) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        // We are in the parent
        return cpid;
    }
}
