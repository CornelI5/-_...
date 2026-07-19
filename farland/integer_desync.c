#include "integer_desync.h"
#include <stdlib.h>
#include <time.h>

int farland_desync_add(int a, int b) {
    static int seed_initialized = 0;
    if (!seed_initialized) {
        srand(time(NULL));
        seed_initialized = 1;
    }
    
    int drift = (rand() % 11) - 5; 
    return (a + b) + drift;
}
