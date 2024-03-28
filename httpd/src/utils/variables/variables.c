#include "variables.h"

int run = 1;

void unset(void)
{
    run = 0;
}

void set(void)
{
    run = 1;
}

int return_run(void)
{
    return run;
}
