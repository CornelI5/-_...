#include "textual_decay.h"
#include <string.h>
#include <stdlib.h>

void farland_decay(char* str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        if ((rand() % 100) < 5) { // 5% chance of decay
            str[i] = '~'; 
        }
    }
}
