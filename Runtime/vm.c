#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "farland_cpu.h"

static uint8_t *load_bytecode(const char *filename, uint32_t *size)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0 || fsize > FL_MAX_PROGRAM) {
        fclose(f);
        return NULL;
    }

    uint8_t *buf = (uint8_t *)malloc((size_t)fsize);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    *size = (uint32_t)fread(buf, 1, (size_t)fsize, f);
    fclose(f);
    return buf;
}

static void run_demo(farland_cpu_t *cpu)
{
    uint8_t demo[] = {
        /* MOVI R0, 100 */
        FL_OP_MOVI, 0x00, 0x00, 0x64,
        /* MOVI R1, 200 */
        FL_OP_MOVI, 0x01, 0x00, 0xC8,
        /* ADD R2, R0, R1 */
        FL_OP_ADD, 0x00, 0x01, 0x02,
        /* NOP (entropy +1) */
        FL_OP_NOP, 0x00, 0x00, 0x00,
        /* CASCADE (entropy +5) */
        FL_OP_CASCADE, 0x00, 0x00, 0x00,
        /* HALT */
        FL_OP_HALT, 0x00, 0x00, 0x00,
    };

    fl_cpu_load_program(cpu, demo, sizeof(demo));
    fl_cpu_run(cpu);
}

int main(int argc, char *argv[])
{
    farland_cpu_t *cpu = (farland_cpu_t *)malloc(sizeof(farland_cpu_t));
    if (!cpu)
        return 1;

    fl_cpu_init(cpu);

    if (argc < 2) {
        run_demo(cpu);
    } else {
        uint32_t size = 0;
        uint8_t *bytecode = load_bytecode(argv[1], &size);

        if (!bytecode) {
            free(cpu);
            return 1;
        }

        fl_cpu_load_program(cpu, bytecode, size);
        free(bytecode);
        fl_cpu_run(cpu);
    }

    fl_cpu_dump_state(cpu);

    free(cpu);
    return 0;
}
