#include "pc.h"

#include <stdio.h>

void pc_init(struct pc_s *pc)
{
    bus_init(&pc->bus, pc);
    cpu_init(&pc->cpu, pc);
    vio_init(&pc->vio, pc, 0);
    memory_init(&pc->memory, 0x100000);
    memory_map(&pc->memory, &pc->bus, 0x100000);
}

void pc_raise_exception(struct pc_s *pc, enum pc_exception_category category,
                        int data)
{
    printf("exception raised: %d %d\n", category, data);
    pc->cpu.state.halt = true;
}
