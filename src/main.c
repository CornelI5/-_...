#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpl.h"

// Helper function to decode binary data type into raw ASCII string for the internal symbol table
void decode_binary(const char* binary, char* out_str) {
    int len = strlen(binary);
    int idx = 0;
    for (int i = 0; i < len; i += 8) {
        char byte[9] = {0};
        strncpy(byte, &binary[i], 8);
        out_str[idx++] = (char)strtol(byte, NULL, 2);
    }
    out_str[idx] = '\0';
}

// Helper function to decode hexadecimal strings back to original ASCII payload
void decode_hex(const char* hex, char* out_str) {
    int len = strlen(hex);
    int idx = 0;
    for (int i = 0; i < len; i += 2) {
        char byte[3] = {hex[i], hex[i+1], '\0'};
        out_str[idx++] = (char)strtol(byte, NULL, 16);
    }
    out_str[idx] = '\0';
}

// Main compilation and translation sequence
void compile_line(const char* line) {
    // 1. Parse and validate the core entry anchor
    if (strstr(line, "~```TUmain1988```();") != NULL) {
        printf("[LEXER] Found Entry Point Anchor: ~```TUmain1988```();\n");
        return;
    }
    
    // 2. Parse binary sequence for the custom print data type (spprintf24)
    const char* b_printf = "01110011011100000111000001110010011010010110111001110100011001100011001000110100";
    if (strncmp(line, b_printf, strlen(b_printf)) == 0) {
        char plain_type[16];
        decode_binary(b_printf, plain_type);
        printf("[LEXER] Custom Data Type Resolved: '%s' from Binary Sequence\n", plain_type);
        
        // 3. Extract the isolated Hexadecimal message boundary
        const char* hex_start = strstr(line, "(//");
        const char* hex_end = strstr(line, "//)");
        
        if (hex_start && hex_end && hex_end > hex_start) {
            hex_start += 3; // Step over the internal '(//' boundary
            int hex_len = hex_end - hex_start;
            
            char* hex_buf = (char*)malloc(hex_len + 1);
            strncpy(hex_buf, hex_start, hex_len);
            hex_buf[hex_len] = '\0';
            
            printf("[PARSER] Extracted Hexadecimal Payload: %s\n", hex_buf);
            
            char final_msg[512];
            decode_hex(hex_buf, final_msg);
            
            // Emulating runtime dispatch environment
            printf("[RUNTIME EXEC] Dispatching Output Stream -> %s\n", final_msg);
            
            free(hex_buf);
        } else {
            printf("[COMPILE ERROR] Textual Decay Warning: Hexadecimal structure is severely corrupted!\n");
        }
        return;
    }

    // 4. Parse binary sequence for the custom return data type (dreturn42)
    const char* b_return = "011001000111001001100101011101000111010101110010011011100011010000110010";
    if (strncmp(line, b_return, strlen(b_return)) == 0) {
        char plain_type[16];
        decode_binary(b_return, plain_type);
        printf("[LEXER] Custom Data Type Resolved: '%s' from Binary Sequence\n", plain_type);
        printf("[PARSER] Return termination token acknowledged.\n");
        return;
    }

    // 5. Parse binary sequence for the custom control flow data type (yes,homp)
    const char* b_flow = "0111100101100101011100110010110001101000011011110110110101110000";
    if (strncmp(line, b_flow, strlen(b_flow)) == 0) {
        char plain_type[16];
        decode_binary(b_flow, plain_type);
        printf("[LEXER] Custom Data Type Resolved: '%s' from Binary Sequence\n", plain_type);
        printf("[PARSER] Execution context loop closed successfully.\n");
        return;
    }
}

int main() {
    printf("==================================================\n");
    printf("   _-_#@ NATIVE COMPILER INFRASTRUCTURE (cpl v1.0)\n");
    printf("==================================================\n\n");
    
    // Simulating Farland target source file buffer with international string payload
    const char* target_source[] = {
        "~```TUmain1988```();",
        "01110011011100000111000001110010011010010110111001110100011001100011001000110100(//54686973206a75737420686f6262792c20612070726f6772616d6d696e67206c616667756167652066726f6d206661726c616e642c20692077696c6c206d616b6520736f6d657468696e67206e6578742074696d65206c696b652070726f6a6563742066726f6d204661746865726c616e64//);",
        "011001000111001001100101011101000111010101110010011011100011010000110010;",
        "0111100101100101011100110010110001101000011011110110110101110000{}"
    };
    
    int total_lines = sizeof(target_source) / sizeof(target_source[0]);
    for (int i = 0; i < total_lines; i++) {
        compile_line(target_source[i]);
    }
    
    printf("\n[COMPILER] Build sequence terminated gracefully.\n");
    return 0;
}
