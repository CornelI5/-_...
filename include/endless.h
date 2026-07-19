#ifndef ENDLESS_H
#define ENDLESS_H

#include "64limit.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

static inline void trigger_endless_loop(int64_t drift_factor) {
    printf("[VOID] Entering Event Horizon at drift: %lld\n", drift_factor);
    
    while (1) {
        for (volatile int64_t i = 0; i < (drift_factor % 0xFFFFF); i++);
        
    }
}

#endif
