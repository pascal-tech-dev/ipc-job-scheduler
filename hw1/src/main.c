#include "config.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s conn\n", argv[0]);
        return EXIT_FAILURE;
    }

    Facility facility = {0};
    if (facility_load(stdin, &facility) != 0) {
        fprintf(stderr, "Error: Invalid facility configuration.\n");
        return EXIT_FAILURE;
    }

    fprintf(stderr, "Loaded %d nodes, %d processor types.\n",
            facility.n, facility.m);

    /* Server startup using argv[1] will be added here. */

    facility_free(&facility);
    return EXIT_SUCCESS;
}
