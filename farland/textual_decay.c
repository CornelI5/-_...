#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void apply_textual_decay(char* str, int decay_level) {
    if (decay_level <= 0) return;
    
    int len = strlen(str);
    char glitch_glyphs[] = "@#$_-^&*~+=?";
    
    for (int i = 0; i < len; i++) {
        if ((rand() % 100) < decay_level) {
            str[i] = glitch_glyphs[rand() % strlen(glitch_glyphs)];
        }
    }
}
