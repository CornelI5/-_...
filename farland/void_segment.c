#include <stdio.h>
#include <stdlib.h>
#include "../include/farland.h"

// Void Segment: Memory that is 'freed' but persists as corrupted energy
void* void_free(void* ptr) {
    if (ptr == NULL) return NULL;
    
    // In Farland, free() doesn't return memory to the OS.
    // It dumps it into the Void Segment, which is a global buffer
    // that slowly overwrites other memory addresses (Address Drift).
    corrupt_neighboring_memory(ptr); 
    
    // Log the corruption for the compiler's debugger
    printf("[VOID SEGMENT] Address %p collapsed into the Void.\n", ptr);
    return NULL;
}
