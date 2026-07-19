#ifndef FARLAND_CONFIG_H
#define FARLAND_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

// ============================================
// -_... from farland
// "Not designed to be the hardest.
//  Designed for a world where computer
//  mathematics are fundamentally different."
// ============================================

#define FARLAND_VERSION "0.1.0-unstable"
#define FARLAND_EPOCH 12550821  // Koordinat Far Lands Minecraft

typedef struct {
    uint32_t seed;
    float decay_rate;       
    float paranoia_level;   
    bool math_is_optional;  
    int reality_integrity;  
} FarlandConfig;

extern FarlandConfig g_farland_config;

void farland_init_chaos(void);

bool is_reality_stable(void);

void degrade_reality(float amount);

int get_farland_mood(void);

#endif
