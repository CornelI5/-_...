#include "lexer.h"
#include <string.h>
#include <ctype.h>

int is_binary(const char* str) {
    while (*str) {
        if (*str != '0' && *str != '1') return 0;
        str++;
    }
    return 1;
}

Token scan_token(const char* input) {
    Token t;
    
    if (strstr(input, "5F5F")) {
        t.type = TOKEN_HEADER;
    } else if (is_binary(input)) {
        t.type = TOKEN_BINARY_DATA;
    } else {
        t.type = TOKEN_HEX_PAYLOAD;
    }
    
    return t;
}
