#ifndef FARLAND_ASM_H
#define FARLAND_ASM_H

#include <stdint.h>
#include <stdbool.h>

#define ASM_MAX_LINES       4096
#define ASM_MAX_LABELS      256
#define ASM_MAX_LINE_LEN    256
#define ASM_MAX_OUTPUT      32768
#define ASM_LABEL_LEN       64

/* Assembler error codes */
#define ASM_OK              0
#define ASM_ERR_SYNTAX      1
#define ASM_ERR_UNKNOWN_OP  2
#define ASM_ERR_BAD_REG     3
#define ASM_ERR_BAD_LABEL   4
#define ASM_ERR_OVERFLOW    5
#define ASM_ERR_DECAY       6   /* Line was corrupted by --- */
#define ASM_ERR_VOID        7   /* The void intervened */

/* Label table entry */
typedef struct {
    char name[ASM_LABEL_LEN];
    uint32_t address;       /* Byte offset in output */
    bool defined;
} asm_label_t;

/* Assembler state */
typedef struct {
    /* Input */
    char lines[ASM_MAX_LINES][ASM_MAX_LINE_LEN];
    int line_count;

    /* Label table */
    asm_label_t labels[ASM_MAX_LABELS];
    int label_count;

    /* Output bytecode */
    uint8_t output[ASM_MAX_OUTPUT];
    uint32_t output_size;

    /* State */
    int current_line;
    uint32_t current_addr;
    bool next_line_zombie;    /* SFC Rule 3: corrupts next line */
    int decay_count;
    int error_count;
    int warning_count;

    /* Error message */
    char error_msg[256];
} farland_asm_t;

/* Public API */
void asm_init(farland_asm_t *as);
int asm_load_source(farland_asm_t *as, const char *filename);
int asm_load_string(farland_asm_t *as, const char *source);
int asm_pass1(farland_asm_t *as);    /* Collect labels */
int asm_pass2(farland_asm_t *as);    /* Generate bytecode */
int asm_assemble(farland_asm_t *as); /* Run both passes */
int asm_save(farland_asm_t *as, const char *filename);
void asm_dump(farland_asm_t *as);    /* Print bytecode hex dump */

#endif /* FARLAND_ASM_H */
