#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <instruction.h>

struct cpu_s;
struct cpu_execution_state;

typedef void (*cpu_instruction)(struct cpu_execution_state *state);
typedef cpu_instruction (*cpu_instruction_lookup)(uint8_t size,
                                                  uint32_t instruction);

union cpu_flags_s {
    struct
    {
        bool carry : 1;
        bool zero : 1;
        bool sign : 1;
        bool overflow : 1;
        bool debug : 1;
    };
    uint32_t flags;
};

enum cpu_errors
{
    CPU_ERROR_UNKNOWN_INSTRUCTION,
    CPU_ERROR_UNKNOWN_REGISTER,
    CPU_ERROR_INSTRUCTION_TOO_LONG,
    CPU_ERROR_ILLEGAL_STATE,
    CPU_ERROR_NO_INSTRUCTION_SET,
    CPU_ERROR_ILLEGAL_OPERAND,
    CPU_ERROR_DIVIDE_BY_ZERO,
    CPU_ERROR_COUNT
};

struct cpu_registers_s
{
    uint32_t rx[8]; /* r[abcdefgh] */
    uint32_t pc;
    uint32_t sp;
    uint32_t bp;
    union cpu_flags_s flags;
};

enum cpu_operand_type
{
    CPU_OPT_REGISTER,
    CPU_OPT_ADDRESS,
    CPU_OPT_CONSTANT,
    CPU_OPT_NONE
};

struct cpu_operand_s
{
    enum cpu_operand_type type;
    uint32_t value;
};

struct cpu_execution_state
{
    union instruction_head_s head;
    uint8_t instruction[16];
    /* LENGTH (in bytes) */
    uint8_t instruction_length;
    /* SIZE (width, word, dword..) */
    enum cpu_width instruction_width;
    cpu_instruction implementation;
    struct cpu_operand_s operands[2];
    struct cpu_s *cpu;
};

struct cpu_state_s
{
    struct cpu_registers_s regs;
    struct cpu_execution_state execution;
    bool halt;
};

struct cpu_s
{
    struct pc_s *pc;
    cpu_instruction_lookup implementation;
    struct cpu_state_s state;
};

void cpu_init(struct cpu_s *cpu, struct pc_s *pc);

void cpu_start(struct cpu_s *cpu);

void cpu_reset(struct cpu_s *cpu);

void cpu_step(struct cpu_s *cpu);

void cpu_dump_information(struct cpu_s *cpu);

uint32_t *cpu_register(struct cpu_s *cpu, uint32_t name);

void cpu_fetch(struct cpu_s *cpu, struct cpu_execution_state *state);
void cpu_decode(struct cpu_s *cpu, struct cpu_execution_state *state);
