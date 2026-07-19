#ifndef CPL_H
#define CPL_H

// Token definitions for the _-_#@ architecture
typedef enum {
    TOKEN_MAIN_ENTRY,   // ~```TUmain1988```();
    TOKEN_TYPE_PRINT,   // 01110011... (spprintf24 binary variant)
    TOKEN_TYPE_RETURN,  // 01100100... (dreturn42 binary variant)
    TOKEN_TYPE_FLOW,    // 01111001... (yes,homp binary variant)
    TOKEN_HEX_MESSAGE,  // Hexadecimal wrapped inside (// ... //)
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

// Core compiler pipeline definitions
void decode_binary(const char* binary, char* out_str);
void decode_hex(const char* hex, char* out_str);
void compile_line(const char* line);

#endif // CPL_H
