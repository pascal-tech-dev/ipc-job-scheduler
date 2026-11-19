#include "agent.h"

#include <stdion.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void agent_run(int client_fd, const Facility *facility) {
    (void)facility;

    char buf[64];
    int n = snprintf(buf, sizeof(buf), "%d\n", (int)getpid());
    if (n > 0)
        (void)write(client_fd, buf, (size_t)n);
    

    /* */
}