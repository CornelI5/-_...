#include "../include/Endless.h"

void execute_instruction(int64_t addr) {
    if (addr > FARLAND_64_MAX) {
        trigger_endless_loop(addr);
    }
}
