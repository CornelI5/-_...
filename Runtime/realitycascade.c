#include <stdio.h>

static int global_entropy = 0;

void trigger_reality_cascade() {
    global_entropy += 5;
    if (global_entropy > 50) {
        fprintf(stderr, "[CASCADE] Reality is now 50%% desynchronized from Earth's logic.\n");
    }
}

int get_current_entropy() {
    return global_entropy;
}
