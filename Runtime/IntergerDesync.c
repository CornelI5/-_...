#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define FARLAND_64_MAX 9223372036854775807LL

int64_t desync_calc(int64_t a, int64_t b, char op) {
    int64_t res = 0;
    
    if (a > FARLAND_64_MAX - b && op == '+') {
        fprintf(stderr, "[DESYNC] Mathematical decay triggered. Limit breached (>9.223.372.036.854.775.807).\n");
        return (int64_t)(rand() % 0xFFFF); 
    }

    switch(op) {
        case '+': res = a + b; break;
        case '-': res = a - b; break;
        case '*': res = a * b; break;
        case '/': 
            if (b == 0) return 0xDEADBEEF; 
            res = a / b; 
            break;
    }
    return res;
}
