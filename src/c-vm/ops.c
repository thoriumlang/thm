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

#include "ops.h"
#include "cpu.h"
#include "cpu_internal.h"

word_t from_big_endian(word_t *word) {
    uint8_t *bytes = (uint8_t *) word;
    return bytes[3] | bytes[2] << 8 | bytes[1] << 16 | bytes[0] << 24;
}

void op_nop(CPU *cpu, const word_t *word) {
    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tNOP\n", cpu->step, cpu->pc - ADDR_SIZE);
    }
}

void op_halt(CPU *cpu, const word_t *word) {
    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tHALT\n", cpu->step, cpu->pc - ADDR_SIZE);
    }
    cpu->running = 0;
}

void op_panic(CPU *cpu, const word_t *word) {
    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tPANIC\n", cpu->step, cpu->pc - ADDR_SIZE);
    }
    if (!cpu->panic) {
        cpu->panic = CPU_ERR_PANIC;
    }
}

void op_push(CPU *cpu, const word_t *word) {
    uint8_t r = ((uint8_t *) word)[1];
    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tPUSH r%i\n", cpu->step, cpu->pc - ADDR_SIZE, r);
    }
    word_t value;
    if ((cpu->panic = cpu_register_get(cpu, r, &value)) == CPU_ERR_OK) {
        cpu->sp -= WORD_SIZE;
        if (bus_word_write(cpu->bus, cpu->sp, value) != BUS_ERR_OK) {
            cpu->panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        }
    }
}

void op_pop(CPU *cpu, const word_t *word) {
    uint8_t r = ((uint8_t *) word)[1];
    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tPOP  r%i\n", cpu->step, cpu->pc - ADDR_SIZE, r);
    }
    word_t value;
    if (bus_word_read(cpu->bus, cpu->sp, &value) != BUS_ERR_OK) {
        cpu->panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
    cpu->panic = cpu_register_set(cpu, r, value);
}

// todo implement support for special registers
void op_mov_rw(CPU *cpu, const word_t *word) {
    uint8_t to = ((uint8_t *) word)[1];

    word_t val = fetch(cpu);
    val = from_big_endian(&val);

    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tMOV  r%i, %i\n", cpu->step, cpu->pc - 2 * ADDR_SIZE, to, val);
    }
    cpu->registers[to] = val;
}

// todo implement support for special registers
void op_mov_rr(CPU *cpu, const word_t *word) {
    uint8_t to = ((uint8_t *) word)[1];
    uint8_t from = ((uint8_t *) word)[2];

    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tMOV  r%i, r%i\n", cpu->step, cpu->pc - ADDR_SIZE, to, from);
    }

    cpu->registers[to] = cpu->registers[from];
}

void op_cmp(CPU *cpu, const word_t *word) {
    uint8_t a = ((uint8_t *) word)[1];
    uint8_t b = ((uint8_t *) word)[2];

    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tCMP  r%i, r%i\n", cpu->step, cpu->pc - ADDR_SIZE, a, b);
    }

    word_t a_val, b_val;
    if ((cpu->panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    if ((cpu->panic = cpu_register_get(cpu, b, &b_val)) != CPU_ERR_OK) {
        return;
    }
    update_flags(cpu, (sword_t) a_val - (sword_t) b_val);
}


void op_jreq(CPU *cpu, const word_t *word) {
    // todo optimize this
    word_t address = fetch(cpu);
    address = from_big_endian(&address);

    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tJEQ  "AXHEX"\n", cpu->step, cpu->pc - 2 * ADDR_SIZE, address);
    }

    if (cpu->zero) {
        cpu->pc = cpu->cs + address;
    }
}

void op_jrne(CPU *cpu, const word_t *word) {
    // todo optimize this
    word_t address = fetch(cpu);
    address = from_big_endian(&address);

    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tJNE  "AXHEX"\n", cpu->step, cpu->pc - 2 * ADDR_SIZE, address);
    }

    if (!cpu->zero) {
        cpu->pc = cpu->cs + address;
    }
}

void op_jr(CPU *cpu, const word_t *word) {
    word_t address = fetch(cpu);
    address = from_big_endian(&address);

    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tJ    "AXHEX"\n", cpu->step, cpu->pc - 2 * ADDR_SIZE, address);
    }

    cpu->pc = cpu->cs + address;
}

// todo implement
void op_stor(CPU *cpu, const word_t *word) {
    uint8_t to = ((uint8_t *) word)[1];
    uint8_t from = ((uint8_t *) word)[2];

    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tSTOR r%i, r%i\n", cpu->step, cpu->pc - ADDR_SIZE, to, from);
    }
}

// todo implement
void op_load(CPU *cpu, const word_t *word) {
    uint8_t to = ((uint8_t *) word)[1];
    uint8_t from = ((uint8_t *) word)[2];

    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tLOAD r%i, r%i\n", cpu->step, cpu->pc - ADDR_SIZE, to, from);
    }
}

void op_add(CPU *cpu, const word_t *word) {
    uint8_t a = ((uint8_t *) word)[1];
    uint8_t b = ((uint8_t *) word)[2];

    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tADD  r%i, r%i\n", cpu->step, cpu->pc - ADDR_SIZE, a, b);
    }

    word_t a_val, b_val;
    if ((cpu->panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    if ((cpu->panic = cpu_register_get(cpu, b, &b_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val + (sword_t) b_val)); // todo implement overflow/underflow
}

void op_dec(CPU *cpu, const word_t *word) {
    uint8_t r = ((uint8_t *) word)[1];

    if (cpu->print_op) {
        printf("  %lu\t"AXHEX"\tDEC  r%i\n", cpu->step, cpu->pc - ADDR_SIZE, r);
    }

    word_t r_val;
    if ((cpu->panic = cpu_register_get(cpu, r, &r_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->panic = cpu_register_set(cpu, r, (word_t) ((sword_t) r_val - 1)); // todo implement underflow
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


