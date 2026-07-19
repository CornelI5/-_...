#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint64_t blocks[16];
} int1024_t;

void void_set_ui64(int1024_t* num, uint64_t val) {
    memset(num->blocks, 0, sizeof(int1024_t));
    num->blocks[0] = val; 
}

int1024_t void_add1024(int1024_t a, int1024_t b) {
    int1024_t result;
    uint64_t carry = 0;

    for (int i = 0; i < 16; i++) {
        uint64_t next_carry = (a.blocks[i] > UINT64_MAX - b.blocks[i] - carry) ? 1 : 0;
        result.blocks[i] = a.blocks[i] + b.blocks[i] + carry;
        carry = next_carry;
    }

    if (carry > 0) {
        fprintf(stderr, "\n[QUANTUM DESYNC] 1024-Bit Boundary Shattered. Injecting reality rot...\n");
        for (int i = 0; i < 16; i++) {
            result.blocks[i] = (uint64_t)(rand() % 0xFFFFFFFF) ^ ~(result.blocks[15 - i]);
        }
    }

    return result;
}

void print_void1024(int1024_t num) {
    printf("0x");
    for (int i = 15; i >= 0; i--) {
        printf("%016llX", (unsigned long long)num.blocks[i]);
    }
    printf("\n");
}
