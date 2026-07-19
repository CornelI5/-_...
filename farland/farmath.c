#include "../include/NoPanic.h"
#include "../include/64limit.h"

int64_t farland_div(int64_t a, int64_t b) {
    TRY_RECOVER(b != 0);
    
    if (b == 0) return 0; 
    return enforce_64_limit(a / b);
}
