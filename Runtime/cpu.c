#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "farland_cpu.h"


void fl_word_set_u64(fl_word_t *w, uint64_t val)
{
    memset(w->blocks, 0, sizeof(fl_word_t));
    w->blocks[0] = val;
}

uint64_t fl_word_get_u64(fl_word_t w)
{
    return w.blocks[0];
}

bool fl_word_is_zero(fl_word_t a)
{
    for (int i = 0; i < FL_WORD_BLOCKS; i++) {
        if (a.blocks[i] != 0)
            return false;
    }
    return true;
}

fl_word_t fl_word_add(fl_word_t a, fl_word_t b, bool *overflow)
{
    fl_word_t result;
    uint64_t carry = 0;
    *overflow = false;

    for (int i = 0; i < FL_WORD_BLOCKS; i++) {
        uint64_t sum_ab = a.blocks[i] + b.blocks[i];
        uint64_t carry_ab = (sum_ab < a.blocks[i]) ? 1 : 0;

        result.blocks[i] = sum_ab + carry;
        uint64_t carry_prev = (result.blocks[i] < sum_ab) ? 1 : 0;

        carry = carry_ab | carry_prev;
    }

    if (carry > 0)
        *overflow = true;

    return result;
}

fl_word_t fl_word_sub(fl_word_t a, fl_word_t b, bool *underflow)
{
    fl_word_t result;
    uint64_t borrow = 0;
    *underflow = false;

    for (int i = 0; i < FL_WORD_BLOCKS; i++) {
        uint64_t diff = a.blocks[i] - b.blocks[i] - borrow;
        borrow = (a.blocks[i] < b.blocks[i] + borrow) ? 1 : 0;
        result.blocks[i] = diff;
    }

    if (borrow > 0)
        *underflow = true;

    return result;
}

fl_word_t fl_word_and(fl_word_t a, fl_word_t b)
{
    fl_word_t result;
    for (int i = 0; i < FL_WORD_BLOCKS; i++)
        result.blocks[i] = a.blocks[i] & b.blocks[i];
    return result;
}

fl_word_t fl_word_or(fl_word_t a, fl_word_t b)
{
    fl_word_t result;
    for (int i = 0; i < FL_WORD_BLOCKS; i++)
        result.blocks[i] = a.blocks[i] | b.blocks[i];
    return result;
}

fl_word_t fl_word_xor(fl_word_t a, fl_word_t b)
{
    fl_word_t result;
    for (int i = 0; i < FL_WORD_BLOCKS; i++)
        result.blocks[i] = a.blocks[i] ^ b.blocks[i];
    return result;
}

fl_word_t fl_word_not(fl_word_t a)
{
    fl_word_t result;
    for (int i = 0; i < FL_WORD_BLOCKS; i++)
        result.blocks[i] = ~a.blocks[i];
    return result;
}

fl_word_t fl_word_shl(fl_word_t a, int bits)
{
    fl_word_t result;
    memset(&result, 0, sizeof(fl_word_t));

    int word_shift = bits / 64;
    int bit_shift = bits % 64;

    for (int i = FL_WORD_BLOCKS - 1; i >= word_shift; i--) {
        result.blocks[i] = a.blocks[i - word_shift] << bit_shift;
        if (bit_shift > 0 && i > word_shift)
            result.blocks[i] |=
                a.blocks[i - word_shift - 1] >> (64 - bit_shift);
    }

    return result;
}

fl_word_t fl_word_shr(fl_word_t a, int bits)
{
    fl_word_t result;
    memset(&result, 0, sizeof(fl_word_t));

    int word_shift = bits / 64;
    int bit_shift = bits % 64;

    for (int i = 0; i < FL_WORD_BLOCKS - word_shift; i++) {
        result.blocks[i] = a.blocks[i + word_shift] >> bit_shift;
        if (bit_shift > 0 && i + word_shift + 1 < FL_WORD_BLOCKS)
            result.blocks[i] |=
                a.blocks[i + word_shift + 1] << (64 - bit_shift);
    }

    return result;
}

fl_word_t fl_word_mul(fl_word_t a, fl_word_t b)
{
    fl_word_t result;
    memset(&result, 0, sizeof(fl_word_t));

    for (int i = 0; i < FL_WORD_BLOCKS; i++) {
        uint64_t carry = 0;
        for (int j = 0; j + i < FL_WORD_BLOCKS; j++) {
            __uint128_t prod =
                (__uint128_t)a.blocks[i] * b.blocks[j] +
                result.blocks[i + j] + carry;
            result.blocks[i + j] = (uint64_t)prod;
            carry = (uint64_t)(prod >> 64);
        }
    }

    return result;
}

void fl_entropy_add(farland_cpu_t *cpu, uint32_t amount)
{
    cpu->entropy += amount;
    fl_word_set_u64(&cpu->sr[FL_REG_ENTROPY], cpu->entropy);
    fl_entropy_check_cascade(cpu);
}

void fl_entropy_check_cascade(farland_cpu_t *cpu)
{
    uint32_t e = cpu->entropy;
    uint32_t new_level = 0;

    if (e > 25)  new_level = 1;
    if (e > 50)  new_level = 2;
    if (e > 75)  new_level = 3;
    if (e >= 100) new_level = 4;
    if (cpu->boundary_breaches > 10) new_level = 5;
    if (e >= 666) new_level = 6;

    /* Level 4+ is permanent */
    if (cpu->cascade_level >= 4 && new_level < 4)
        new_level = cpu->cascade_level;

    if (new_level > cpu->cascade_level) {
        cpu->cascade_level = new_level;
        fl_word_set_u64(&cpu->sr[FL_REG_CASCADE], new_level);

        /* Update flags */
        cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_CASCADE;

        if (new_level >= 3)
            cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_MATH_DEAD;
    }

    /* Update math enforcement */
    cpu->math_enforced = fl_is_math_enforced(cpu);

    /* Update paranoia */
    if (cpu->paranoia_level >= 0.75f) {
        cpu->paranoia_active = true;
        cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_PARANOIA;
    }
}

bool fl_is_math_enforced(farland_cpu_t *cpu)
{
    switch (cpu->cascade_level) {
    case 0:  return true;
    case 1:  return (rand() % 2 == 0);
    case 2:  return (rand() % 3 == 0);
    case 3:  return (rand() % 5 == 0);
    default: return false;
    }
}

bool fl_is_1024_stable(farland_cpu_t *cpu)
{
    if (cpu->boundary_breaches == 0) return true;
    if (cpu->boundary_breaches < 5) return (rand() % 2 == 0);
    return false;
}


static uint32_t fl_apply_drift(farland_cpu_t *cpu, uint32_t addr)
{
    if (addr > 0x7FFFFFFF || addr >= FL_MEM_SIZE) {
        uint32_t ghost = (addr % 0x1000) + (0xDEADBEEF % FL_MEM_SIZE);
        cpu->drift_count++;
        cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_DRIFT;
        fl_entropy_add(cpu, 8);
        return ghost % FL_MEM_SIZE;
    }
    return addr % FL_MEM_SIZE;
}

void fl_mem_write(farland_cpu_t *cpu, uint32_t addr,
                  const uint8_t *data, uint32_t size)
{
    addr = fl_apply_drift(cpu, addr);

    /* Textual decay: 5% chance per byte to corrupt */
    for (uint32_t i = 0; i < size && (addr + i) < FL_MEM_SIZE; i++) {
        if (cpu->decay_count > 0 && rand() % 20 == 0) {
            cpu->mem[addr + i] = 0xDE;
            cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_DECAY;
        } else {
            cpu->mem[addr + i] = data[i];
        }
    }
}

void fl_mem_read(farland_cpu_t *cpu, uint32_t addr,
                 uint8_t *data, uint32_t size)
{
    addr = fl_apply_drift(cpu, addr);

    for (uint32_t i = 0; i < size && (addr + i) < FL_MEM_SIZE; i++) {
        data[i] = cpu->mem[addr + i];
    }
}
void fl_cpu_init(farland_cpu_t *cpu)
{
    memset(cpu, 0, sizeof(farland_cpu_t));

    cpu->running = false;
    cpu->halted = false;
    cpu->math_enforced = true;
    cpu->paranoia_active = false;
    cpu->paranoia_level = 0.0f;

    /* Initialize void register to 0xDEADBEEF... (permanent) */
    for (int i = 0; i < FL_WORD_BLOCKS; i++)
        cpu->sr[FL_REG_VOID].blocks[i] = 0xDEADBEEFDEADBEEFULL;

    /* Initialize stack pointer to top of stack */
    fl_word_set_u64(&cpu->sr[FL_REG_SP], FL_STACK_SIZE);

    srand((unsigned int)time(NULL));
}

void fl_cpu_reset(farland_cpu_t *cpu)
{
    uint8_t saved_program[FL_MAX_PROGRAM];
    uint32_t saved_size = cpu->program_size;
    memcpy(saved_program, cpu->program, saved_size);

    fl_cpu_init(cpu);

    memcpy(cpu->program, saved_program, saved_size);
    cpu->program_size = saved_size;
}

void fl_cpu_load_program(farland_cpu_t *cpu,
                         const uint8_t *bytecode,
                         uint32_t size)
{
    if (size > FL_MAX_PROGRAM)
        size = FL_MAX_PROGRAM;

    memcpy(cpu->program, bytecode, size);
    cpu->program_size = size;
    fl_word_set_u64(&cpu->sr[FL_REG_PC], 0);
}


static uint32_t fl_fetch(farland_cpu_t *cpu)
{
    uint32_t pc = (uint32_t)fl_word_get_u64(cpu->sr[FL_REG_PC]);

    if (pc + 4 > cpu->program_size) {
        cpu->halted = true;
        cpu->running = false;
        return FL_OP_HALT;
    }

    uint32_t instr =
        ((uint32_t)cpu->program[pc]     << 24) |
        ((uint32_t)cpu->program[pc + 1] << 16) |
        ((uint32_t)cpu->program[pc + 2] <<  8) |
        ((uint32_t)cpu->program[pc + 3]);

    /* Advance PC */
    fl_word_set_u64(&cpu->sr[FL_REG_PC], pc + 4);

    return instr;
}


void fl_exec_nop(farland_cpu_t *cpu, uint32_t instr)
{
    (void)instr;
    /* Even doing nothing adds entropy in Farland */
    fl_entropy_add(cpu, 1);
}

void fl_exec_halt(farland_cpu_t *cpu, uint32_t instr)
{
    (void)instr;
    if (rand() % 20 == 0) {
        fl_entropy_add(cpu, 5);
        return; /* The void refuses to stop */
    }
    cpu->halted = true;
    cpu->running = false;
}

void fl_exec_add(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    int rb = (instr >> 8) & 0xFF;
    int rc = instr & 0xFF;

    if (ra >= FL_NUM_GP_REGS || rb >= FL_NUM_GP_REGS ||
        rc >= FL_NUM_GP_REGS)
        return;

    bool overflow = false;
    fl_word_t result = fl_word_add(cpu->gp[ra], cpu->gp[rb], &overflow);

    if (overflow) {
        cpu->boundary_breaches++;
        cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_OVERFLOW;
        fl_entropy_add(cpu, 15);

        /* Reality rot: corrupt the result */
        for (int i = 0; i < FL_WORD_BLOCKS; i++) {
            uint64_t noise =
                ((uint64_t)rand() << 32) | (uint64_t)rand();
            result.blocks[i] = noise ^ ~(result.blocks[15 - i]);
        }
    }

    /* If math is not enforced, 15% chance result is wrong */
    if (!cpu->math_enforced && rand() % 100 < 15) {
        result.blocks[0] ^= (uint64_t)(rand() % 3 + 1);
    }

    cpu->gp[rc] = result;
}

void fl_exec_sub(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    int rb = (instr >> 8) & 0xFF;
    int rc = instr & 0xFF;

    if (ra >= FL_NUM_GP_REGS || rb >= FL_NUM_GP_REGS ||
        rc >= FL_NUM_GP_REGS)
        return;

    /* Paranoia check: '-' is a Threat Vector */
    if (cpu->paranoia_active) {
        cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_PARANOIA;
        fl_entropy_add(cpu, 10);
        /* Subtraction reclassified as addition */
        bool overflow = false;
        cpu->gp[rc] = fl_word_add(cpu->gp[ra], cpu->gp[rb], &overflow);
        return;
    }

    bool underflow = false;
    fl_word_t result = fl_word_sub(cpu->gp[ra], cpu->gp[rb], &underflow);

    if (underflow) {
        cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_OVERFLOW;
        fl_entropy_add(cpu, 10);
    }

    cpu->gp[rc] = result;
}

void fl_exec_mul(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    int rb = (instr >> 8) & 0xFF;
    int rc = instr & 0xFF;

    if (ra >= FL_NUM_GP_REGS || rb >= FL_NUM_GP_REGS ||
        rc >= FL_NUM_GP_REGS)
        return;

    fl_word_t result;

    if (!cpu->math_enforced && rand() % 100 < 20) {
        /* 20% chance: multiplication becomes division */
        uint64_t divisor = fl_word_get_u64(cpu->gp[rb]);
        if (divisor == 0) {
            /* Division by zero: open portal */
            fl_entropy_add(cpu, 15);
            fl_word_set_u64(&result, 0xDEADBEEF ^ (uint64_t)rand());
        } else {
            fl_word_set_u64(&result,
                fl_word_get_u64(cpu->gp[ra]) / divisor);
        }
    } else {
        result = fl_word_mul(cpu->gp[ra], cpu->gp[rb]);
    }

    cpu->gp[rc] = result;
}

void fl_exec_div(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    int rb = (instr >> 8) & 0xFF;
    int rc = instr & 0xFF;

    if (ra >= FL_NUM_GP_REGS || rb >= FL_NUM_GP_REGS ||
        rc >= FL_NUM_GP_REGS)
        return;

    uint64_t divisor = fl_word_get_u64(cpu->gp[rb]);

    if (divisor == 0) {
        /* Division by zero: not an error, a portal */
        cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_VOID;
        fl_entropy_add(cpu, 15);
        cpu->void_interventions++;
        fl_word_set_u64(&cpu->gp[rc],
            0xDEADBEEF ^ (uint64_t)rand());
        return;
    }

    fl_word_set_u64(&cpu->gp[rc],
        fl_word_get_u64(cpu->gp[ra]) / divisor);
}

void fl_exec_logic(farland_cpu_t *cpu, uint32_t instr)
{
    uint8_t opcode = (instr >> 24) & 0xFF;
    int ra = (instr >> 16) & 0xFF;
    int rb = (instr >> 8) & 0xFF;
    int rc = instr & 0xFF;

    if (ra >= FL_NUM_GP_REGS || rb >= FL_NUM_GP_REGS ||
        rc >= FL_NUM_GP_REGS)
        return;

    switch (opcode) {
    case FL_OP_AND: cpu->gp[rc] = fl_word_and(cpu->gp[ra], cpu->gp[rb]); break;
    case FL_OP_OR:  cpu->gp[rc] = fl_word_or(cpu->gp[ra], cpu->gp[rb]);  break;
    case FL_OP_XOR: cpu->gp[rc] = fl_word_xor(cpu->gp[ra], cpu->gp[rb]); break;
    case FL_OP_NOT: cpu->gp[rc] = fl_word_not(cpu->gp[ra]);              break;
    }
}

void fl_exec_shift(farland_cpu_t *cpu, uint32_t instr)
{
    uint8_t opcode = (instr >> 24) & 0xFF;
    int ra = (instr >> 16) & 0xFF;
    int rb = (instr >> 8) & 0xFF;
    int rc = instr & 0xFF;

    if (ra >= FL_NUM_GP_REGS || rb >= FL_NUM_GP_REGS ||
        rc >= FL_NUM_GP_REGS)
        return;

    int bits = (int)(fl_word_get_u64(cpu->gp[rb]) % FL_WORD_BITS);

    if (opcode == FL_OP_SHL)
        cpu->gp[rc] = fl_word_shl(cpu->gp[ra], bits);
    else
        cpu->gp[rc] = fl_word_shr(cpu->gp[ra], bits);
}

void fl_exec_load(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;  /* destination register */
    int rb = (instr >> 8) & 0xFF;   /* address register */

    if (ra >= FL_NUM_GP_REGS || rb >= FL_NUM_GP_REGS)
        return;

    uint32_t addr = (uint32_t)fl_word_get_u64(cpu->gp[rb]);
    uint8_t buf[FL_WORD_BLOCKS * 8];

    fl_mem_read(cpu, addr, buf, sizeof(buf));
    memcpy(cpu->gp[ra].blocks, buf, sizeof(fl_word_t));
}

void fl_exec_store(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;  /* source register */
    int rb = (instr >> 8) & 0xFF;   /* address register */

    if (ra >= FL_NUM_GP_REGS || rb >= FL_NUM_GP_REGS)
        return;

    uint32_t addr = (uint32_t)fl_word_get_u64(cpu->gp[rb]);
    uint8_t buf[FL_WORD_BLOCKS * 8];

    memcpy(buf, cpu->gp[ra].blocks, sizeof(fl_word_t));
    fl_mem_write(cpu, addr, buf, sizeof(buf));
}

void fl_exec_push(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    if (ra >= FL_NUM_GP_REGS) return;

    uint32_t sp = (uint32_t)fl_word_get_u64(cpu->sr[FL_REG_SP]);

    if (sp < sizeof(fl_word_t)) {
        /* Stack overflow: contained within VM */
        fl_entropy_add(cpu, 10);
        return;
    }

    sp -= sizeof(fl_word_t);
    memcpy(&cpu->stack[sp], &cpu->gp[ra], sizeof(fl_word_t));
    fl_word_set_u64(&cpu->sr[FL_REG_SP], sp);
}

void fl_exec_pop(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    if (ra >= FL_NUM_GP_REGS) return;

    uint32_t sp = (uint32_t)fl_word_get_u64(cpu->sr[FL_REG_SP]);

    if (sp + sizeof(fl_word_t) > FL_STACK_SIZE) {
        fl_entropy_add(cpu, 10);
        return;
    }

    memcpy(&cpu->gp[ra], &cpu->stack[sp], sizeof(fl_word_t));
    sp += sizeof(fl_word_t);
    fl_word_set_u64(&cpu->sr[FL_REG_SP], sp);
}

void fl_exec_mov(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    int rb = (instr >> 8) & 0xFF;

    if (ra >= FL_NUM_GP_REGS || rb >= FL_NUM_GP_REGS)
        return;

    cpu->gp[ra] = cpu->gp[rb];
}

void fl_exec_movi(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    uint16_t imm = instr & 0xFFFF;

    if (ra >= FL_NUM_GP_REGS) return;

    fl_word_set_u64(&cpu->gp[ra], (uint64_t)imm);
}

void fl_exec_jmp(farland_cpu_t *cpu, uint32_t instr)
{
    uint32_t target = instr & 0xFFFFFF;

    /* 5% chance: jump is refused */
    if (rand() % 20 == 0) {
        fl_entropy_add(cpu, 3);
        return;
    }

    fl_word_set_u64(&cpu->sr[FL_REG_PC], target);
}

void fl_exec_jz(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    uint32_t target = instr & 0xFFFF;

    if (ra >= FL_NUM_GP_REGS) return;

    bool is_zero = fl_word_is_zero(cpu->gp[ra]);

    /* 10% chance: condition is inverted */
    if (!cpu->math_enforced && rand() % 10 == 0)
        is_zero = !is_zero;

    if (is_zero)
        fl_word_set_u64(&cpu->sr[FL_REG_PC], target);
}

void fl_exec_jnz(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    uint32_t target = instr & 0xFFFF;

    if (ra >= FL_NUM_GP_REGS) return;

    bool is_zero = fl_word_is_zero(cpu->gp[ra]);

    if (!cpu->math_enforced && rand() % 10 == 0)
        is_zero = !is_zero;

    if (!is_zero)
        fl_word_set_u64(&cpu->sr[FL_REG_PC], target);
}

void fl_exec_call(farland_cpu_t *cpu, uint32_t instr)
{
    uint32_t target = instr & 0xFFFFFF;
    uint32_t pc = (uint32_t)fl_word_get_u64(cpu->sr[FL_REG_PC]);

    /* Push return address */
    fl_word_t ret_addr;
    fl_word_set_u64(&ret_addr, pc);

    uint32_t sp = (uint32_t)fl_word_get_u64(cpu->sr[FL_REG_SP]);
    if (sp >= sizeof(fl_word_t)) {
        sp -= sizeof(fl_word_t);
        memcpy(&cpu->stack[sp], &ret_addr, sizeof(fl_word_t));
        fl_word_set_u64(&cpu->sr[FL_REG_SP], sp);
    }

    fl_word_set_u64(&cpu->sr[FL_REG_PC], target);
}

void fl_exec_ret(farland_cpu_t *cpu, uint32_t instr)
{
    (void)instr;

    uint32_t sp = (uint32_t)fl_word_get_u64(cpu->sr[FL_REG_SP]);

    if (sp + sizeof(fl_word_t) > FL_STACK_SIZE) {
        cpu->halted = true;
        cpu->running = false;
        return;
    }

    fl_word_t ret_addr;
    memcpy(&ret_addr, &cpu->stack[sp], sizeof(fl_word_t));
    sp += sizeof(fl_word_t);
    fl_word_set_u64(&cpu->sr[FL_REG_SP], sp);

    fl_word_set_u64(&cpu->sr[FL_REG_PC],
        fl_word_get_u64(ret_addr));
}

void fl_exec_cmp(farland_cpu_t *cpu, uint32_t instr)
{
    int ra = (instr >> 16) & 0xFF;
    int rb = (instr >> 8) & 0xFF;

    if (ra >= FL_NUM_GP_REGS || rb >= FL_NUM_GP_REGS)
        return;

    bool equal = true;
    for (int i = 0; i < FL_WORD_BLOCKS; i++) {
        if (cpu->gp[ra].blocks[i] != cpu->gp[rb].blocks[i]) {
            equal = false;
            break;
        }
    }

    /* 5% chance: equal things are not equal */
    if (!cpu->math_enforced && rand() % 20 == 0)
        equal = !equal;

    if (equal)
        cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_ZERO;
    else
        cpu->sr[FL_REG_FLAGS].blocks[0] &= ~FL_FLAG_ZERO;
}

void fl_exec_cascade(farland_cpu_t *cpu, uint32_t instr)
{
    (void)instr;
    fl_entropy_add(cpu, 5);
}

void fl_exec_decay(farland_cpu_t *cpu, uint32_t instr)
{
    (void)instr;
    cpu->decay_count++;
    cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_DECAY;
    fl_entropy_add(cpu, 6);
}

void fl_exec_drift(farland_cpu_t *cpu, uint32_t instr)
{
    (void)instr;
    cpu->drift_count++;
    cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_DRIFT;
    fl_entropy_add(cpu, 8);
}

void fl_exec_panic(farland_cpu_t *cpu, uint32_t instr)
{
    (void)instr;
    /* NoPanic doctrine: intercept and ignore */
    cpu->panic_count++;
    cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_PANIC;
    fl_entropy_add(cpu, 10);
    /* The panic is ignored. The VM continues. */
}

void fl_exec_void(farland_cpu_t *cpu, uint32_t instr)
{
    (void)instr;
    /* The void instruction: undefined behavior, contained */
    cpu->void_active = true;
    cpu->void_interventions++;
    cpu->sr[FL_REG_FLAGS].blocks[0] |= FL_FLAG_VOID;
    fl_entropy_add(cpu, 20);

    /* Random register corruption (contained) */
    int target_reg = rand() % FL_NUM_GP_REGS;
    for (int i = 0; i < FL_WORD_BLOCKS; i++) {
        cpu->gp[target_reg].blocks[i] =
            ((uint64_t)rand() << 32) | (uint64_t)rand();
    }
}

/* ══════════════════════════════════════════════════════════════════
 *  MAIN EXECUTION LOOP
 * ══════════════════════════════════════════════════════════════════ */

void fl_cpu_step(farland_cpu_t *cpu)
{
    if (cpu->halted || !cpu->running)
        return;

    uint32_t instr = fl_fetch(cpu);
    uint8_t opcode = (instr >> 24) & 0xFF;

    cpu->cycle_count++;
    cpu->instructions_executed++;

    switch (opcode) {
    case FL_OP_NOP:     fl_exec_nop(cpu, instr);     break;
    case FL_OP_HALT:    fl_exec_halt(cpu, instr);    break;
    case FL_OP_ADD:     fl_exec_add(cpu, instr);     break;
    case FL_OP_SUB:     fl_exec_sub(cpu, instr);     break;
    case FL_OP_MUL:     fl_exec_mul(cpu, instr);     break;
    case FL_OP_DIV:     fl_exec_div(cpu, instr);     break;
    case FL_OP_AND:
    case FL_OP_OR:
    case FL_OP_XOR:
    case FL_OP_NOT:     fl_exec_logic(cpu, instr);   break;
    case FL_OP_SHL:
    case FL_OP_SHR:     fl_exec_shift(cpu, instr);   break;
    case FL_OP_LOAD:    fl_exec_load(cpu, instr);    break;
    case FL_OP_STORE:   fl_exec_store(cpu, instr);   break;
    case FL_OP_PUSH:    fl_exec_push(cpu, instr);    break;
    case FL_OP_POP:     fl_exec_pop(cpu, instr);     break;
    case FL_OP_MOV:     fl_exec_mov(cpu, instr);     break;
    case FL_OP_MOVI:    fl_exec_movi(cpu, instr);    break;
    case FL_OP_JMP:     fl_exec_jmp(cpu, instr);     break;
    case FL_OP_JZ:      fl_exec_jz(cpu, instr);      break;
    case FL_OP_JNZ:     fl_exec_jnz(cpu, instr);     break;
    case FL_OP_CALL:    fl_exec_call(cpu, instr);    break;
    case FL_OP_RET:     fl_exec_ret(cpu, instr);     break;
    case FL_OP_CMP:     fl_exec_cmp(cpu, instr);     break;
    case FL_OP_CASCADE: fl_exec_cascade(cpu, instr); break;
    case FL_OP_DECAY:   fl_exec_decay(cpu, instr);   break;
    case FL_OP_DRIFT:   fl_exec_drift(cpu, instr);   break;
    case FL_OP_PANIC:   fl_exec_panic(cpu, instr);   break;
    case FL_OP_VOID:    fl_exec_void(cpu, instr);    break;
    default:
        /* Unknown opcode: the void decides */
        fl_entropy_add(cpu, 3);
        break;
    }
}

void fl_cpu_run(farland_cpu_t *cpu)
{
    cpu->running = true;
    cpu->halted = false;

    while (cpu->running && !cpu->halted) {
        fl_cpu_step(cpu);

        /* Safety: max 10 million cycles */
        if (cpu->cycle_count > 10000000) {
            cpu->halted = true;
            cpu->running = false;
        }
    }
}

void fl_cpu_halt(farland_cpu_t *cpu)
{
    cpu->halted = true;
    cpu->running = false;
}

/* ══════════════════════════════════════════════════════════════════
 *  DIAGNOSTICS
 * ══════════════════════════════════════════════════════════════════ */

void fl_cpu_dump_regs(farland_cpu_t *cpu)
{
    for (int i = 0; i < FL_NUM_GP_REGS; i++) {
        printf("  R%-2d: 0x", i);
        for (int j = FL_WORD_BLOCKS - 1; j >= 0; j--)
            printf("%016llX",
                (unsigned long long)cpu->gp[i].blocks[j]);
        printf("\n");
    }

    printf("  PC : %llu\n",
        (unsigned long long)fl_word_get_u64(cpu->sr[FL_REG_PC]));
    printf("  SP : %llu\n",
        (unsigned long long)fl_word_get_u64(cpu->sr[FL_REG_SP]));
    printf("  FLG: 0x%llX\n",
        (unsigned long long)cpu->sr[FL_REG_FLAGS].blocks[0]);
    printf("  ENT: %u\n", cpu->entropy);
    printf("  CSC: %u\n", cpu->cascade_level);
    printf("  BRH: %u\n", cpu->boundary_breaches);
    printf("  DRF: %u\n", cpu->drift_count);
    printf("  DCY: %u\n", cpu->decay_count);
    printf("  PNC: %u\n", cpu->panic_count);
    printf("  VOD: %u\n", cpu->void_interventions);
}

void fl_cpu_dump_state(farland_cpu_t *cpu)
{
    printf("\n");
    printf("  ==========================================\n");
    printf("  FARLAND VIRTUAL CPU — STATE DUMP\n");
    printf("  ==========================================\n");
    printf("  Cycles:      %llu\n",
        (unsigned long long)cpu->cycle_count);
    printf("  Instructions:%llu\n",
        (unsigned long long)cpu->instructions_executed);
    printf("  Running:     %s\n", cpu->running ? "YES" : "NO");
    printf("  Halted:      %s\n", cpu->halted ? "YES" : "NO");
    printf("  Math:        %s\n",
        cpu->math_enforced ? "ENFORCED" : "OPTIONAL");
    printf("  Paranoia:    %s (%.2f)\n",
        cpu->paranoia_active ? "ACTIVE" : "INACTIVE",
        cpu->paranoia_level);
    printf("  ------------------------------------------\n");
    fl_cpu_dump_regs(cpu);
    printf("  ==========================================\n\n");
}
