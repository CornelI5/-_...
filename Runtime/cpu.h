#ifndef FARLAND_CPU_H
#define FARLAND_CPU_H

#include <stdint.h>
#include <stdbool.h>


#define FL_WORD_BITS        1024
#define FL_WORD_BLOCKS      16
#define FL_BITS_PER_BLOCK   64

#define FL_NUM_GP_REGS      16      /* R0 - R15: general purpose */
#define FL_NUM_SPECIAL_REGS 8       /* PC, SP, FP, FLAGS, ENTROPY, etc. */

#define FL_MEM_SIZE         65536   /* 64 KB virtual memory */
#define FL_STACK_SIZE       4096    /* 4 KB virtual stack */
#define FL_MAX_PROGRAM      32768   /* 32 KB max bytecode */

#define FL_HORIZON_75PCT    0xBFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFULL


typedef struct {
    uint64_t blocks[FL_WORD_BLOCKS];
} fl_word_t;
/

#define FL_REG_PC       0   /* Program Counter */
#define FL_REG_SP       1   /* Stack Pointer */
#define FL_REG_FP       2   /* Frame Pointer */
#define FL_REG_FLAGS    3   /* Status Flags */
#define FL_REG_ENTROPY  4   /* Entropy Accumulator (CONTAINED) */
#define FL_REG_CASCADE  5   /* Cascade Level */
#define FL_REG_DRIFT    6   /* Drift Counter */
#define FL_REG_VOID     7   /* Void State (read-only, always 0xDEAD...) */


#define FL_FLAG_ZERO        (1 << 0)
#define FL_FLAG_OVERFLOW    (1 << 1)    /* 1024-bit boundary breached */
#define FL_FLAG_DRIFT       (1 << 2)    /* Address drift occurred */
#define FL_FLAG_DECAY       (1 << 3)    /* Textual decay active */
#define FL_FLAG_PARANOIA    (1 << 4)    /* Paranoia mode active */
#define FL_FLAG_PANIC       (1 << 5)    /* Panic intercepted */
#define FL_FLAG_CASCADE     (1 << 6)    /* Cascade in progress */
#define FL_FLAG_VOID        (1 << 7)    /* Void has intervened */
#define FL_FLAG_MATH_DEAD   (1 << 8)    /* Mathematics is no longer enforced */


#define FL_OP_NOP       0x00    /* Do nothing (entropy still increases) */
#define FL_OP_HALT      0x01    /* Stop (maybe) */
#define FL_OP_ADD       0x10    /* Add (may subtract, may void) */
#define FL_OP_SUB       0x11    /* Subtract (DANGEROUS: may trigger paranoia) */
#define FL_OP_MUL       0x12    /* Multiply (may divide, may random) */
#define FL_OP_DIV       0x13    /* Divide (may open portal) */
#define FL_OP_AND       0x14    /* Bitwise AND */
#define FL_OP_OR        0x15    /* Bitwise OR */
#define FL_OP_XOR       0x16    /* Bitwise XOR */
#define FL_OP_NOT       0x17    /* Bitwise NOT */
#define FL_OP_SHL       0x18    /* Shift left */
#define FL_OP_SHR       0x19    /* Shift right */
#define FL_OP_LOAD      0x20    /* Load from memory (may drift) */
#define FL_OP_STORE     0x21    /* Store to memory (may decay) */
#define FL_OP_PUSH      0x22    /* Push to stack */
#define FL_OP_POP       0x23    /* Pop from stack */
#define FL_OP_MOV       0x30    /* Move register to register */
#define FL_OP_MOVI      0x31    /* Move immediate to register */
#define FL_OP_JMP       0x40    /* Jump (may refuse) */
#define FL_OP_JZ        0x41    /* Jump if zero (may ignore condition) */
#define FL_OP_JNZ       0x42    /* Jump if not zero (may invert) */
#define FL_OP_CALL      0x43    /* Call subroutine */
#define FL_OP_RET       0x44    /* Return from subroutine */
#define FL_OP_CMP       0x50    /* Compare (result is chaos_bool) */
#define FL_OP_CASCADE   0x60    /* Trigger reality cascade */
#define FL_OP_DECAY     0x61    /* Trigger textual decay */
#define FL_OP_DRIFT     0x62    /* Trigger address drift */
#define FL_OP_PANIC     0x63    /* Trigger panic (intercepted by NoPanic) */
#define FL_OP_VOID      0xFF    /* The void instruction (undefined behavior) */


typedef struct {
    /* 1024-bit general purpose registers */
    fl_word_t gp[FL_NUM_GP_REGS];

    /* 1024-bit special registers */
    fl_word_t sr[FL_NUM_SPECIAL_REGS];

    /* Virtual memory (entropy-contained) */
    uint8_t mem[FL_MEM_SIZE];

    /* Virtual stack */
    uint8_t stack[FL_STACK_SIZE];
    uint32_t stack_top;

    /* Program bytecode */
    uint8_t program[FL_MAX_PROGRAM];
    uint32_t program_size;

    /* CPU state */
    bool running;
    bool halted;
    uint64_t cycle_count;
    uint64_t instructions_executed;

    /* Entropy containment */
    uint32_t entropy;           /* Contained within VM */
    uint32_t cascade_level;     /* Contained within VM */
    uint32_t boundary_breaches; /* Contained within VM */
    uint32_t drift_count;       /* Contained within VM */
    uint32_t decay_count;       /* Contained within VM */
    uint32_t panic_count;       /* Contained within VM */

    /* Paranoia state */
    bool paranoia_active;
    float paranoia_level;

    /* Math enforcement */
    bool math_enforced;

    /* Void intervention */
    bool void_active;
    uint32_t void_interventions;
} farland_cpu_t;

/* Lifecycle */
void fl_cpu_init(farland_cpu_t *cpu);
void fl_cpu_reset(farland_cpu_t *cpu);
void fl_cpu_load_program(farland_cpu_t *cpu,
                         const uint8_t *bytecode,
                         uint32_t size);
void fl_cpu_run(farland_cpu_t *cpu);
void fl_cpu_step(farland_cpu_t *cpu);
void fl_cpu_halt(farland_cpu_t *cpu);

/* Instruction execution */
void fl_exec_nop(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_halt(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_add(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_sub(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_mul(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_div(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_logic(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_shift(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_load(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_store(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_push(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_pop(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_mov(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_movi(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_jmp(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_jz(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_jnz(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_call(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_ret(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_cmp(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_cascade(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_decay(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_drift(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_panic(farland_cpu_t *cpu, uint32_t instr);
void fl_exec_void(farland_cpu_t *cpu, uint32_t instr);

/* 1024-bit arithmetic (contained) */
fl_word_t fl_word_add(fl_word_t a, fl_word_t b, bool *overflow);
fl_word_t fl_word_sub(fl_word_t a, fl_word_t b, bool *underflow);
fl_word_t fl_word_mul(fl_word_t a, fl_word_t b);
fl_word_t fl_word_and(fl_word_t a, fl_word_t b);
fl_word_t fl_word_or(fl_word_t a, fl_word_t b);
fl_word_t fl_word_xor(fl_word_t a, fl_word_t b);
fl_word_t fl_word_not(fl_word_t a);
fl_word_t fl_word_shl(fl_word_t a, int bits);
fl_word_t fl_word_shr(fl_word_t a, int bits);
bool fl_word_is_zero(fl_word_t a);
void fl_word_set_u64(fl_word_t *w, uint64_t val);
uint64_t fl_word_get_u64(fl_word_t w);

/* Entropy management (CONTAINED) */
void fl_entropy_add(farland_cpu_t *cpu, uint32_t amount);
void fl_entropy_check_cascade(farland_cpu_t *cpu);
bool fl_is_math_enforced(farland_cpu_t *cpu);
bool fl_is_1024_stable(farland_cpu_t *cpu);

/* Memory access (with drift containment) */
void fl_mem_write(farland_cpu_t *cpu, uint32_t addr,
                  const uint8_t *data, uint32_t size);
void fl_mem_read(farland_cpu_t *cpu, uint32_t addr,
                 uint8_t *data, uint32_t size);

/* Diagnostics */
void fl_cpu_dump_state(farland_cpu_t *cpu);
void fl_cpu_dump_regs(farland_cpu_t *cpu);

#endif /* FARLAND_CPU_H */
