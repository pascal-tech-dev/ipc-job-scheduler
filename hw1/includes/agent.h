#ifndef AGENT_H
#define AGENT_H

#include "config.h"

/* Runs in the child after fork. Takes ownership of client_fd. Does not return. */
void agent_run(int client_fd, const Facility *facility);

#endif /* AGENT_H */