#include "config.h"

#include <stdint.h>
#include <stdlib.h>

int facility_load(FILE *in, Facility *out) {
    if (out == NULL) {
        return -1;
    }

    *out = (Facility) {0};

    if (in == NULL)
        return -1;

    Facility loaded = {0};

    if (fscanf(in, "%d %d", &loaded.m, &loaded.n) != 2 || 
        loaded.m <= 0 || loaded.n <= 0)
        return -1;

    if ((size_t)loaded.n > SIZE_MAX / sizeof(*loaded.capacity) ||
        (size_t)loaded.m > SIZE_MAX / sizeof(**loaded.capacity))
        return -1;

    loaded.capacity = calloc(
        (size_t)loaded.n,
        sizeof(*loaded.capacity)
    );

    if (loaded.capacity == NULL)
        return -1;

    for (int node = 0; node < loaded.n; ++node) {
        /* Allocate memory for the node's processor type capacities. */
        loaded.capacity[node] = malloc(
            (size_t)loaded.m * sizeof(*loaded.capacity[node])
        );

        if (loaded.capacity[node] == NULL) {
            facility_free(&loaded);
            return -1;
        }

        for (int type = 0; type < loaded.m; ++type) {
            if (fscanf(in, "%d",
                       &loaded.capacity[node][type]) != 1 ||
                loaded.capacity[node][type] < 0) {
                facility_free(&loaded);
                return -1;
            }
        }
    }

    *out = loaded;
    return 0;
}

void facility_free(Facility *facility) {
    if (facility == NULL)
        return;

    if (facility->capacity != NULL) {
        for (int node = 0; node < facility->n; node++) {
            free(facility->capacity[node]);
        }
        free(facility->capacity);
    }
    
    *facility = (Facility) {0};
}

int facility_at(const Facility *facility, int node, int type) {
    if (facility == NULL || facility->capacity == NULL ||
        node < 0 || node >= facility->n ||
        type < 0 || type >= facility->m ||
        facility->capacity[node] == NULL)
        return -1;
    
    return facility->capacity[node][type];
}
