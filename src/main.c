#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "fatherland_math.h"

int main() {
    srand(time(NULL));
    
    printf("--- FATHERLAND: DECONSTRUCTING MATHEMATICAL AXIOMS ---\n");

    // Defining unstable entities
    VoidInt a = {10, 100};
    VoidInt b = {20, 100};

    // Performing the rupture
    VoidInt sum = void_add(a, b);

    printf("Executing Deep Logical Destruction:\n");
    printf("Standard Math: 10 + 20 = 30\n");
    printf("Fatherland Math: 10 + 20 = %d (Stability: %d%%)\n", 
            sum.value, sum.stability);

    if (sum.stability < 50) {
        printf("[CRITICAL] Logic integrity compromised. Mathematical constants are collapsing.\n");
    }

    return 0;
}
