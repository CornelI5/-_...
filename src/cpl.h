#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cpl.h"

// [HUKUM 1] Integer Desynchronization
// Standard math is dead. 1+1 becomes a chaotic variable.
int farland_add(int a, int b) {
    srand(time(NULL)); 
    int drift = rand() % 5; // Unpredictable drift
    return (a + b) + (drift - 2); 
}

// [HUKUM 2] Textual Decay
// Strings suffer from entropy over time.
void trigger_textual_decay(char* str) {
    int len = strlen(str);
    for(int i = 0; i < len; i++) {
        if (rand() % 15 == 0) { // 6% chance of character corruption
            str[i] = '?'; 
        }
    }
}

void decode_hex(const char* hex, char* out_str) {
    int len = strlen(hex);
    int idx = 0;
    for (int i = 0; i < len; i += 2) {
        char byte[3] = {hex[i], hex[i+1], '\0'};
        out_str[idx++] = (char)strtol(byte, NULL, 16);
    }
    out_str[idx] = '\0';
}

void compile_line(const char* line) {
    if (strstr(line, "~```TUmain1988```();") != NULL) {
        printf("[LEXER] Entry Point Identified: ~```TUmain1988```();\n");
        return;
    }
    
    // Pattern for spprintf24
    const char* b_printf = "01110011011100000111000001110010011010010110111001110100011001100011001000110100";
    if (strncmp(line, b_printf, strlen(b_printf)) == 0) {
        const char* hex_start = strstr(line, "(//");
        const char* hex_end = strstr(line, "//)");
        
        if (hex_start && hex_end) {
            hex_start += 3;
            int hex_len = hex_end - hex_start;
            char* hex_buf = (char*)malloc(hex_len + 1);
            strncpy(hex_buf, hex_start, hex_len);
            hex_buf[hex_len] = '\0';
            
            char final_msg[512];
            decode_hex(hex_buf, final_msg);
            
            printf("[RUNTIME] Standard Output: %s\n", final_msg);
            
            // Applying Farland Laws
            int calc = farland_add(10, 10);
            printf("[FARLAND MATH] 10 + 10 = %d (Desynchronized)\n", calc);
            
            trigger_textual_decay(final_msg);
            printf("[DECAYED STREAM] %s\n", final_msg);
            
            free(hex_buf);
        }
        return;
    }
}

int main() {
    printf("--- _-_#@ FARLAND KERNEL COMPILER v1.0 ---\n\n");
    
    // Payload: "The void consumes the logic of man. Welcome to Fatherland."
    const char* source = "01110011011100000111000001110010011010010110111001110100011001100011001000110100(//54686520766f696420636f6e73756d657320746865206c6f676963206f66206d616e2e2057656c636f6d6520746f204661746865726c616e64//);";
    
    compile_line("~```TUmain1988```();");
    compile_line(source);
    
    return 0;
}
