/*
 * main.c
 *
 *  Created on: Feb 3, 2018
 *      Author: nullifiedcat
 */

#include "pc.h"
#include "cpu.h"
#include "vio/vio.h"
#include "vio/dev/term.h"
#include "cpu/cat/cat.h"
#include "cpu/cat/instructions_byte.h"

#include <stdio.h>
#include <string.h>

#define INSTR_MAKE_HEAD(size, op1, op2) \
    CPU_INSTRUCTION_LENGTH_##size | (CPU_OPERAND_MODE_##op1 << 3) | CPU_OPERAND_MODE_##op2

void test_bus(struct pc_s *pc)
{
    struct
    {
        uint32_t data[4];
    } test_struct;

    test_struct.data[0] = 123;
    test_struct.data[1] = 456;
    test_struct.data[2] = 789;
    test_struct.data[3] = 321;

    printf("test: bus write struct: %d\n",
           bus_write(&pc->bus, 0x100000, sizeof(test_struct), &test_struct));

    memset(&test_struct, 0, sizeof(test_struct));

    printf("test: bus read struct: %d\n",
           bus_read(&pc->bus, 0x100000, sizeof(test_struct), &test_struct));

    printf("test: %u %u %u %u\n", test_struct.data[0], test_struct.data[1],
           test_struct.data[2], test_struct.data[3]);
}

int main(int argc, const char **argv)
{
    struct pc_s pc;

    pc_init(&pc);
    vio_add_device(&pc.vio, viod_term);

    pc.cpu.implementation = cat_cpu_instruction_lookup;
    pc.cpu.state.regs.flags.debug = 1;

    /*test_bus(&pc);*/

    /* TODO this is a test!
     * A simple program that prints the line from 100100h to printer repeatedly, a single character at a time
     *
     * Assembly pseudocode:
     *
     *  xor ra, ra                  ; reset ra
     * loop:
     *  cmp [60h], 0                ; vio[0].r0: is device ready?
     *  jnz loop                    ; loop until terminal is ready
     *  mov [80h], [ra + 100100h]   ; vio[0].data[0]: set the message (single character)
     *  mov [64h], 1                ; vio[0].r1: length of message = 1
     *  mov [60h], 1                ; vio[0].r0: VDCS_TERM_PRINT
     *                              ; This actually sends the command to terminal to print the message. r0 is then reset by the device.
     *  cmp [ra + 100100h], 0       ; if end of string is reached (null character)
     *  je ireset                   ; jump to iterator reset
     *  inc ra                      ; iterator++
     *  jmp loop                    ; go back to loop
     * ireset:
     *  xor ra, ra                  ; ra = 0
     *  jmp loop
     *
     * C pseudocode:
     *
     *  uint32_t *terminal_registers = 0x60;
     *  uint8_t  *terminal_data = 0x80;
     *  uint32_t *message = 0x100100;
     *
     *  uint32_t iterator; // Stored in RA
     *
     *  while (true)
     *  {
     *      while (*terminal_registers[0]); // Locking loop
     *      terminal_data[0] = message[iterator];
     *      *terminal_registers[1] = 1;
     *      *terminal_registers[0] = VDCS_TERM_PRINT;
     *      if (message[iterator] == 0)
     *          iterator = 0;
     *  }
     *
     * WTFCAT assembly (closer to bytecode)
     *
     *  xor.dw reg/reg 0, 0
     *
     *  cmp.dw adr/cst 60, 0
     *  jnz.dw adr/--- 100004
     *  mov.dw adr/rso 80, 0, 100100
     *  mov.dw adr/cst 64, 1
     *  mov.dw adr/cst 60, 1
     *  cmp.dw rso/cst ra, 100100, 0
     *  je.dw  off/--- 9
     *  inc.dw reg/--- 0
     *  jmp.dw adr/--- 100004
     *
     *  xor.dw reg/reg 0, 0
     *  jmp.dw adr/--- 100004
     *
     *  */
    uint8_t program[] = {
            INSTR_MAKE_HEAD(BYTE, REGISTER, REGISTER), CPU_CAT_8_XOR, 0x00, 0x00,
            INSTR_MAKE_HEAD(BYTE, ADDRESS, IMMEDIATE), CPU_CAT_8_COMPARE, /* LSB DWORD 00000060 = 60 00 00 00 */ 0x60, 0x00, 0x00, 0x00, /* RA */ 0x00,
            INSTR_MAKE_HEAD(BYTE, ADDRESS, NONE), CPU_CAT_8_JUMP_IF_NOT_EQUAL, /* LSB DWORD 00100004 = 04 00 10 00 */ 0x04, 0x00, 0x10, 0x00,
            INSTR_MAKE_HEAD(BYTE, ADDRESS, REGISTER_INDIRECT_WITH_OFFSET), CPU_CAT_8_MOVE,

    };

    bus_write(&pc.bus, 0x100000, sizeof(program), program);
    pc.cpu.state.regs.pc = 0x100000;

    cpu_start(&pc.cpu);
    printf("CPU done\n");
    return 0;
}
