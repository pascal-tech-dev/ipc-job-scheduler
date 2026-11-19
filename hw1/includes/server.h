#ifndef SERVER_H
#define SERVER_H

#include "config.h"

/* Listen forever on conn (@path or ip:port). Returns 0 on success, -1 on failure. */
int server_run(const char  *conn, const Facility *facility);

#endif /* SERVER_H */