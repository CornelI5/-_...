#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_BINARY_DATA,  // 010101...
    TOKEN_HEX_PAYLOAD,  // 48656C6C...
    TOKEN_HEADER,       // 5F5F...
    TOKEN_ADDR,         // 0x00A0
    TOKEN_EOF,
    TOKEN_ERROR         // Radiation level exceeded
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int radiation_level; 
} Token;

#endif
