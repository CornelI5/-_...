#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void* calculate_address_drift(uintptr_t base_addr) {
    if (base_addr > 0x7FFFFFFF) {
        uintptr_t ghost_addr = (base_addr % 0x1000) + 0xDEADBEEF;
        fprintf(stderr, "[DRIFT] Pointer escaped stable bounds. Redirected to Ghost Memory: 0x%lx\n", ghost_addr);
        return (void*)ghost_addr;
    }
    return (void*)base_addr;
}
