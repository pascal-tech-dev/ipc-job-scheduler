#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

#define MAX_JOBS 10000

typedef struct {
    int m;         /* Number of processor types. */
    int n;         /* Number of nodes. */
    int *capacity; /* Owned, row-major array: capacity[node * m + type]. */
} Facility;

/*
 * Load positive dimensions and nonnegative capacities from in.
 * Returns 0 on success, -1 on failure; leaves out empty on failure.
 * out must point to an empty Facility (initialize it with {0}).
 * Release a successfully loaded facility with facility_free().
 */
int facility_load(FILE *in, Facility *out);

/* Free capacity and reset all fields to zero. Accepts NULL. */
void facility_free(Facility *facility);

/* Return the configured capacity, or -1 for an invalid facility/index. */
int facility_at(const Facility *facility, int node, int type);

#endif /* CONFIG_H */
