#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "farland_asm.h"
*/

typedef struct {
    const char *mnemonic;
    uint8_t opcode;
    int num_operands;   /* 0, 1, 2, or 3 */
} opcode_entry_t;

static const opcode_entry_t g_opcodes[] = {
    { "NOP",     0x00, 0 },
    { "HALT",    0x01, 0 },
    { "ADD",     0x10, 3 },
    { "SUB",     0x11, 3 },
    { "MUL",     0x12, 3 },
    { "DIV",     0x13, 3 },
    { "AND",     0x14, 3 },
    { "OR",      0x15, 3 },
    { "XOR",     0x16, 3 },
    { "NOT",     0x17, 2 },
    { "SHL",     0x18, 3 },
    { "SHR",     0x19, 3 },
    { "LOAD",    0x20, 2 },
    { "STORE",   0x21, 2 },
    { "PUSH",    0x22, 1 },
    { "POP",     0x23, 1 },
    { "MOV",     0x30, 2 },
    { "MOVI",    0x31, 2 },
    { "JMP",     0x40, 1 },
    { "JZ",      0x41, 2 },
    { "JNZ",     0x42, 2 },
    { "CALL",    0x43, 1 },
    { "RET",     0x44, 0 },
    { "CMP",     0x50, 2 },
    { "CASCADE", 0x60, 0 },
    { "DECAY",   0x61, 0 },
    { "DRIFT",   0x62, 0 },
    { "PANIC",   0x63, 0 },
    { "VOID",    0xFF, 0 },
};

#define NUM_OPCODES (sizeof(g_opcodes) / sizeof(g_opcodes[0]))


void asm_init(farland_asm_t *as)
{
    memset(as, 0, sizeof(farland_asm_t));
}

/* ══════════════════════════════════════════════════════════════════
 *  SOURCE LOADING
 * ══════════════════════════════════════════════════════════════════ */

int asm_load_source(farland_asm_t *as, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        snprintf(as->error_msg, sizeof(as->error_msg),
            "Cannot open file: %s", filename);
        return ASM_ERR_SYNTAX;
    }

    as->line_count = 0;
    while (as->line_count < ASM_MAX_LINES &&
           fgets(as->lines[as->line_count], ASM_MAX_LINE_LEN, f)) {
        /* Strip newline */
        char *nl = strchr(as->lines[as->line_count], '\n');
        if (nl) *nl = '\0';
        nl = strchr(as->lines[as->line_count], '\r');
        if (nl) *nl = '\0';
        as->line_count++;
    }

    fclose(f);
    return ASM_OK;
}

int asm_load_string(farland_asm_t *as, const char *source)
{
    as->line_count = 0;

    const char *ptr = source;
    while (*ptr && as->line_count < ASM_MAX_LINES) {
        int i = 0;
        while (*ptr && *ptr != '\n' && i < ASM_MAX_LINE_LEN - 1) {
            as->lines[as->line_count][i++] = *ptr++;
        }
        as->lines[as->line_count][i] = '\0';
        if (*ptr == '\n') ptr++;
        as->line_count++;
    }

    return ASM_OK;
}

/* Trim leading and trailing whitespace in-place */
static char *trim(char *str)
{
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) *end-- = '\0';
    return str;
}

/* Check if a line is empty or a comment */
static bool is_empty_or_comment(const char *line)
{
    const char *p = line;
    while (isspace((unsigned char)*p)) p++;

    if (*p == '\0') return true;
    if (*p == ';') return true;
    if (*p == '-' && *(p + 1) == '-' && *(p + 2) != '-') return true;

    return false;
}

/* Check if a line is a destructive comment (---) */
static bool is_decay_comment(const char *line)
{
    const char *p = line;
    while (isspace((unsigned char)*p)) p++;
    return (p[0] == '-' && p[1] == '-' && p[2] == '-');
}

/* Check if a line is a label definition (name:) */
static bool is_label_def(const char *line, char *label_name)
{
    const char *p = line;
    while (isspace((unsigned char)*p)) p++;

    int i = 0;
    while (isalnum((unsigned char)*p) || *p == '_') {
        if (i < ASM_LABEL_LEN - 1)
            label_name[i++] = *p;
        p++;
    }
    label_name[i] = '\0';

    while (isspace((unsigned char)*p)) p++;
    return (*p == ':' && i > 0);
}

/* Parse a register operand: R0-R15 */
static int parse_register(const char *token)
{
    if (token[0] != 'R' && token[0] != 'r')
        return -1;

    int reg = atoi(token + 1);
    if (reg < 0 || reg > 15)
        return -1;

    return reg;
}

/* Parse an immediate value (decimal or hex) */
static int parse_immediate(const char *token)
{
    if (token[0] == '0' && (token[1] == 'x' || token[1] == 'X'))
        return (int)strtol(token, NULL, 16);
    return atoi(token);
}

/* Find or create a label */
static int find_label(farland_asm_t *as, const char *name)
{
    for (int i = 0; i < as->label_count; i++) {
        if (strcmp(as->labels[i].name, name) == 0)
            return i;
    }
    return -1;
}

static int add_label(farland_asm_t *as, const char *name,
                     uint32_t addr, bool defined)
{
    int idx = find_label(as, name);
    if (idx >= 0) {
        as->labels[idx].address = addr;
        as->labels[idx].defined = defined;
        return idx;
    }

    if (as->label_count >= ASM_MAX_LABELS)
        return -1;

    idx = as->label_count++;
    strncpy(as->labels[idx].name, name, ASM_LABEL_LEN - 1);
    as->labels[idx].address = addr;
    as->labels[idx].defined = defined;
    return idx;
}

/* Look up opcode by mnemonic */
static const opcode_entry_t *lookup_opcode(const char *mnemonic)
{
    for (int i = 0; i < (int)NUM_OPCODES; i++) {
        if (strcmp(g_opcodes[i].mnemonic, mnemonic) == 0)
            return &g_opcodes[i];
    }
    return NULL;
}

int asm_pass1(farland_asm_t *as)
{
    uint32_t addr = 0;
    bool zombie_next = false;

    for (int i = 0; i < as->line_count; i++) {
        char line_copy[ASM_MAX_LINE_LEN];
        strncpy(line_copy, as->lines[i], ASM_MAX_LINE_LEN - 1);
        line_copy[ASM_MAX_LINE_LEN - 1] = '\0';

        /* SFC Rule 3: --- corrupts next line */
        if (is_decay_comment(line_copy)) {
            zombie_next = true;
            continue;
        }

        if (zombie_next) {
            zombie_next = false;
            continue; /* This line is a zombie, skip it */
        }

        if (is_empty_or_comment(line_copy))
            continue;

        /* Check for label definition */
        char label_name[ASM_LABEL_LEN];
        if (is_label_def(line_copy, label_name)) {
            add_label(as, label_name, addr, true);
            continue; /* Labels don't generate bytecode */
        }

        /* It's an instruction: 4 bytes */
        addr += 4;
    }

    return ASM_OK;
}


/* Tokenize a line into mnemonic and operands */
static int tokenize(char *line, char *tokens[], int max_tokens)
{
    int count = 0;

    /* Replace commas with spaces for uniform splitting */
    for (char *p = line; *p; p++) {
        if (*p == ',') *p = ' ';
    }

    char *saveptr = NULL;
    char *tok = strtok_r(line, " \t", &saveptr);
    while (tok && count < max_tokens) {
        tokens[count++] = tok;
        tok = strtok_r(NULL, " \t", &saveptr);
    }

    return count;
}

static void emit_instruction(farland_asm_t *as, uint8_t opcode,
                             uint8_t a, uint8_t b, uint8_t c)
{
    if (as->output_size + 4 > ASM_MAX_OUTPUT) {
        as->error_count++;
        return;
    }

    as->output[as->output_size++] = opcode;
    as->output[as->output_size++] = a;
    as->output[as->output_size++] = b;
    as->output[as->output_size++] = c;
}

int asm_pass2(farland_asm_t *as)
{
    as->output_size = 0;
    as->current_addr = 0;
    bool zombie_next = false;

    for (int i = 0; i < as->line_count; i++) {
        char line_copy[ASM_MAX_LINE_LEN];
        strncpy(line_copy, as->lines[i], ASM_MAX_LINE_LEN - 1);
        line_copy[ASM_MAX_LINE_LEN - 1] = '\0';

        as->current_line = i + 1;

        /* SFC Rule 3: --- corrupts next line */
        if (is_decay_comment(line_copy)) {
            zombie_next = true;
            as->decay_count++;
            as->warning_count++;
            continue;
        }

        if (zombie_next) {
            zombie_next = false;
            as->warning_count++;
            /* Emit a zombie instruction: VOID opcode */
            emit_instruction(as, 0xFF, 0xDE, 0xAD, 0xEF);
            as->current_addr += 4;
            continue;
        }

        if (is_empty_or_comment(line_copy))
            continue;

        /* Skip label definitions */
        char label_name[ASM_LABEL_LEN];
        if (is_label_def(line_copy, label_name))
            continue;

        /* Tokenize */
        char *tokens[8];
        int ntokens = tokenize(line_copy, tokens, 8);

        if (ntokens == 0)
            continue;

        /* Convert mnemonic to uppercase */
        for (char *p = tokens[0]; *p; p++)
            *p = (char)toupper((unsigned char)*p);

        /* Look up opcode */
        const opcode_entry_t *op = lookup_opcode(tokens[0]);
        if (!op) {
            snprintf(as->error_msg, sizeof(as->error_msg),
                "Line %d: Unknown opcode '%s'",
                as->current_line, tokens[0]);
            as->error_count++;
            continue;
        }

        /* Parse operands based on instruction type */
        uint8_t a = 0, b = 0, c = 0;

        switch (op->num_operands) {
        case 0:
            /* No operands: NOP, HALT, RET, CASCADE, etc. */
            break;

        case 1:
            /* One operand: PUSH, POP, JMP, CALL */
            if (ntokens < 2) {
                as->error_count++;
                break;
            }

            if (op->opcode == 0x40 || op->opcode == 0x43) {
                /* JMP or CALL: operand is a label or address */
                int lbl_idx = find_label(as, tokens[1]);
                if (lbl_idx >= 0 && as->labels[lbl_idx].defined) {
                    uint32_t target = as->labels[lbl_idx].address;
                    a = (target >> 16) & 0xFF;
                    b = (target >> 8) & 0xFF;
                    c = target & 0xFF;
                } else {
                    /* Try as immediate address */
                    uint32_t target = (uint32_t)parse_immediate(tokens[1]);
                    a = (target >> 16) & 0xFF;
                    b = (target >> 8) & 0xFF;
                    c = target & 0xFF;
                }
                /* For JMP/CALL, encode target in lower 24 bits */
                emit_instruction(as, op->opcode, a, b, c);
                as->current_addr += 4;
                continue;
            } else {
                /* PUSH or POP: operand is a register */
                int reg = parse_register(tokens[1]);
                if (reg < 0) {
                    as->error_count++;
                    break;
                }
                a = (uint8_t)reg;
            }
            break;

        case 2:
            /* Two operands: MOV, MOVI, LOAD, STORE, CMP, JZ, JNZ, NOT, SHL, SHR */
            if (ntokens < 3) {
                as->error_count++;
                break;
            }

            if (op->opcode == 0x31) {
                /* MOVI: register, immediate */
                int reg = parse_register(tokens[1]);
                int imm = parse_immediate(tokens[2]);
                if (reg < 0) { as->error_count++; break; }
                a = (uint8_t)reg;
                b = (imm >> 8) & 0xFF;
                c = imm & 0xFF;
                emit_instruction(as, op->opcode, a, b, c);
                as->current_addr += 4;
                continue;
            }

            if (op->opcode == 0x41 || op->opcode == 0x42) {
                /* JZ / JNZ: register, label */
                int reg = parse_register(tokens[1]);
                if (reg < 0) { as->error_count++; break; }
                a = (uint8_t)reg;

                int lbl_idx = find_label(as, tokens[2]);
                uint32_t target = 0;
                if (lbl_idx >= 0 && as->labels[lbl_idx].defined)
                    target = as->labels[lbl_idx].address;
                else
                    target = (uint32_t)parse_immediate(tokens[2]);

                b = (target >> 8) & 0xFF;
                c = target & 0xFF;
                emit_instruction(as, op->opcode, a, b, c);
                as->current_addr += 4;
                continue;
            }

            /* Generic two-register: MOV, LOAD, STORE, CMP, NOT, SHL, SHR */
            {
                int ra = parse_register(tokens[1]);
                int rb = parse_register(tokens[2]);
                if (ra < 0 || rb < 0) { as->error_count++; break; }
                a = (uint8_t)ra;
                b = (uint8_t)rb;
            }
            break;

        case 3:
            /* Three operands: ADD, SUB, MUL, DIV, AND, OR, XOR, SHL, SHR */
            if (ntokens < 4) {
                as->error_count++;
                break;
            }
            {
                int ra = parse_register(tokens[1]);
                int rb = parse_register(tokens[2]);
                int rc = parse_register(tokens[3]);
                if (ra < 0 || rb < 0 || rc < 0) {
                    as->error_count++;
                    break;
                }
                a = (uint8_t)ra;
                b = (uint8_t)rb;
                c = (uint8_t)rc;
            }
            break;
        }

        emit_instruction(as, op->opcode, a, b, c);
        as->current_addr += 4;
    }

    return (as->error_count > 0) ? ASM_ERR_SYNTAX : ASM_OK;
}


int asm_assemble(farland_asm_t *as)
{
    int result;

    result = asm_pass1(as);
    if (result != ASM_OK)
        return result;

    result = asm_pass2(as);
    return result;
}


int asm_save(farland_asm_t *as, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (!f) {
        snprintf(as->error_msg, sizeof(as->error_msg),
            "Cannot write to file: %s", filename);
        return ASM_ERR_SYNTAX;
    }

    fwrite(as->output, 1, as->output_size, f);
    fclose(f);
    return ASM_OK;
}

void asm_dump(farland_asm_t *as)
{
    printf("\n");
    printf("  FARLAND ASSEMBLER — BYTECODE DUMP\n");
    printf("  ==========================================\n");
    printf("  Lines:    %d\n", as->line_count);
    printf("  Labels:   %d\n", as->label_count);
    printf("  Output:   %u bytes (%u instructions)\n",
           as->output_size, as->output_size / 4);
    printf("  Decay:    %d line(s) corrupted\n", as->decay_count);
    printf("  Errors:   %d\n", as->error_count);
    printf("  Warnings: %d\n", as->warning_count);
    printf("  ------------------------------------------\n");

    for (uint32_t i = 0; i < as->output_size; i += 4) {
        printf("  %04X:  %02X %02X %02X %02X",
               i,
               as->output[i],
               (i + 1 < as->output_size) ? as->output[i + 1] : 0,
               (i + 2 < as->output_size) ? as->output[i + 2] : 0,
               (i + 3 < as->output_size) ? as->output[i + 3] : 0);

        /* Decode mnemonic */
        uint8_t opcode = as->output[i];
        const char *name = "???";
        for (int j = 0; j < (int)NUM_OPCODES; j++) {
            if (g_opcodes[j].opcode == opcode) {
                name = g_opcodes[j].mnemonic;
                break;
            }
        }
        printf("    ; %s", name);

        /* Check for zombie instruction */
        if (opcode == 0xFF && as->output[i+1] == 0xDE &&
            as->output[i+2] == 0xAD && as->output[i+3] == 0xEF) {
            printf(" (ZOMBIE — corrupted by ---)");
        }

        printf("\n");
    }

    printf("  ==========================================\n");

    /* Print label table */
    if (as->label_count > 0) {
        printf("\n  LABEL TABLE:\n");
        for (int i = 0; i < as->label_count; i++) {
            printf("    %-20s -> 0x%04X %s\n",
                   as->labels[i].name,
                   as->labels[i].address,
                   as->labels[i].defined ? "" : "(UNDEFINED)");
        }
    }

    printf("\n");
}


#ifdef ASM_STANDALONE
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Farland Assembler v1.0\n");
        printf("Usage: %s <input.fls> [output.flb]\n", argv[0]);
        printf("\n");
        printf("  .fls  Farland Assembly Source\n");
        printf("  .flb  Farland Bytecode (output)\n");
        printf("\n");
        printf("  If no output file is specified,\n");
        printf("  bytecode is dumped to stdout.\n");
        return 1;
    }

    farland_asm_t *as = (farland_asm_t *)malloc(sizeof(farland_asm_t));
    if (!as) return 1;

    asm_init(as);

    int result = asm_load_source(as, argv[1]);
    if (result != ASM_OK) {
        printf("ERROR: %s\n", as->error_msg);
        free(as);
        return 1;
    }

    result = asm_assemble(as);

    asm_dump(as);

    if (result == ASM_OK && argc >= 3) {
        result = asm_save(as, argv[2]);
        if (result == ASM_OK) {
            printf("  Saved: %s (%u bytes)\n",
                   argv[2], as->output_size);
        }
    }

    if (as->error_count > 0) {
        printf("  Assembly completed with %d error(s).\n",
               as->error_count);
    }

    free(as);
    return (as->error_count > 0) ? 1 : 0;
}
#endif /* ASM_STANDALONE */
