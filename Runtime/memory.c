#include "../include/NoPanic.h"
#include <stdlib.h>

void* farland_alloc(size_t size) {
    void* ptr = malloc(size);
    
    TRY_RECOVER(ptr != NULL);
    
    return ptr ? ptr : (void*)0xDEADBEEF; 
}
