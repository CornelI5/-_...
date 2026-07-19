#ifndef NOPANIC_H
#define NOPANIC_H

#include <stdio.h>

static inline void suppress_catastrophe() {
    fprintf(stderr, "[SYSTEM] Integrity breach detected, but Farland persists...\n");
}

#define TRY_RECOVER(expression) \
    do { \
        if (!(expression)) { \
            suppress_catastrophe(); \
            /* no happened */ \
        } \
    } while (0)

#endif
