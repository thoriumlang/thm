/**
 * Copyright 2021 Christophe Pollet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vmarch.h"
#include "ops.h"
#include "cpu.h"
#include "cpu_internal.h"

void op_nop(CPU *cpu, const word_t *word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tNOP\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
}

void op_halt(CPU *cpu, const word_t *word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tHALT\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
    cpu->state.running = 0;
}

void op_panic(CPU *cpu, const word_t *word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPANIC\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
    if (!cpu->state.panic) {
        cpu->state.panic = CPU_ERR_PANIC;
    }
}

void op_push(CPU *cpu, const word_t *word) {
    uint8_t r = ((uint8_t *) word)[1];
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPUSH r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, r);
    }
    word_t value;
    if ((cpu->state.panic = cpu_register_get(cpu, r, &value)) == CPU_ERR_OK) {
        cpu->sp -= WORD_SIZE;
        if (bus_word_write(cpu->bus, cpu->sp, value) != BUS_ERR_OK) {
            cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        }
    }
}

void op_pop(CPU *cpu, const word_t *word) {
    uint8_t r = ((uint8_t *) word)[1];
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPOP  r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, r);
    }
    word_t value;
    if (bus_word_read(cpu->bus, cpu->sp, &value) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
    cpu->state.panic = cpu_register_set(cpu, r, value);
}

// todo implement support for special registers
void op_mov_rw(CPU *cpu, const word_t *word) {
    uint8_t to = ((uint8_t *) word)[1];

    word_t val = cpu_fetch(cpu);
    val = from_big_endian(&val);

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tMOV  r%i, %i\n", cpu->debug.step, cpu->pc - 2 * ADDR_SIZE, to, val);
    }
    cpu->registers[to] = val;
}

// todo implement support for special registers
void op_mov_rr(CPU *cpu, const word_t *word) {
    uint8_t to = ((uint8_t *) word)[1];
    uint8_t from = ((uint8_t *) word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tMOV  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, to, from);
    }

    cpu->registers[to] = cpu->registers[from];
}

void op_cmp(CPU *cpu, const word_t *word) {
    uint8_t a = ((uint8_t *) word)[1];
    uint8_t b = ((uint8_t *) word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tCMP  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b);
    }

    word_t a_val, b_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    if ((cpu->state.panic = cpu_register_get(cpu, b, &b_val)) != CPU_ERR_OK) {
        return;
    }
    cpu_flags_update(cpu, (sword_t) a_val - (sword_t) b_val);
}

void op_jreq(CPU *cpu, const word_t *word) {
    addr_t address = 0;
    addr_t pc = cpu->pc;
    if (cpu->flags.zero == 1 || cpu->debug.print_op) {
        address = cpu_fetch(cpu);
        address = from_big_endian(&address);

        if (cpu->debug.print_op) {
            printf("  %lu\t"AXHEX"\tJEQ  "AXHEX"\t\t// z=%i -> %s\n",
                   cpu->debug.step,
                   cpu->pc - 2 * ADDR_SIZE,
                   address,
                   cpu->flags.zero,
                   cpu->flags.zero == 0 ? "no jump" : "jump"
            );
        }
    }

    cpu->pc = (cpu->flags.zero == 1) * (cpu->cs + address) // if equals, jump
              + (cpu->flags.zero == 0) * (pc + ADDR_SIZE); // otherwise, move to next opcode
}

void op_jrne(CPU *cpu, const word_t *word) {
    addr_t address = 0;
    addr_t pc = cpu->pc;
    if (cpu->flags.zero == 0 || cpu->debug.print_op) {
        address = cpu_fetch(cpu);
        address = from_big_endian(&address);

        if (cpu->debug.print_op) {
            printf("  %lu\t"AXHEX"\tJNE  "AXHEX"\t\t// z=%i -> %s\n",
                   cpu->debug.step,
                   cpu->pc - 2 * ADDR_SIZE,
                   address,
                   cpu->flags.zero,
                   cpu->flags.zero == 0 ? "jump" : "no jump"
            );
        }
    }

    cpu->pc = (cpu->flags.zero == 0) * (cpu->cs + address) // if not equals, jump
              + (cpu->flags.zero == 1) * (pc + ADDR_SIZE); // otherwise, move to next word
}

void op_jr(CPU *cpu, const word_t *word) {
    word_t address = cpu_fetch(cpu);
    address = from_big_endian(&address);

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tJ    "AXHEX"\n", cpu->debug.step, cpu->pc - 2 * ADDR_SIZE, address);
    }

    cpu->pc = cpu->cs + address;
}

// todo implement
void op_stor(CPU *cpu, const word_t *word) {
    uint8_t to = ((uint8_t *) word)[1];
    uint8_t from = ((uint8_t *) word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tSTOR r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, to, from);
    }
}

// todo implement
void op_load(CPU *cpu, const word_t *word) {
    uint8_t to = ((uint8_t *) word)[1];
    uint8_t from = ((uint8_t *) word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tLOAD r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, to, from);
    }
}

void op_add(CPU *cpu, const word_t *word) {
    uint8_t a = ((uint8_t *) word)[1];
    uint8_t b = ((uint8_t *) word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tADD  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b);
    }

    word_t a_val, b_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    if ((cpu->state.panic = cpu_register_get(cpu, b, &b_val)) != CPU_ERR_OK) {
        return;
    }
    // todo implement overflow/underflow
    cpu->state.panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val + (sword_t) b_val));
}

void op_dec(CPU *cpu, const word_t *word) {
    uint8_t r = ((uint8_t *) word)[1];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tDEC  r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, r);
    }

    word_t r_val;
    if ((cpu->state.panic = cpu_register_get(cpu, r, &r_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, r, (word_t) ((sword_t) r_val - 1)); // todo implement underflow
}

op_ptr ops[OPS_COUNT] = {
        &op_nop,
        &op_halt,
        &op_panic,
        &op_mov_rw,
        &op_mov_rr,
        &op_add,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &op_dec,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &op_cmp,
        &op_push,
        &op_pop,
        NULL,
        &op_jreq,
        &op_jrne,
        &op_jr,
        &op_stor,
        &op_load
};


