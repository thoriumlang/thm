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

/*
 * perf. improvement ideas:
 *  - https://stackoverflow.com/questions/30130930/is-there-a-compiler-hint-for-gcc-to-force-branch-prediction-to-always-go-a-certa
 *  - disable print_op with compile-time macro
 */

#include <stdlib.h>
#include "vmarch.h"
#include "ops.h"
#include "cpu.h"
#include "cpu_internal.h"

void op_nop(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tNOP\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
}

void op_halt(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tHALT\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
    cpu->state.running = 0;
}

void op_panic(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPANIC\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
    if (!cpu->state.panic) {
        cpu->state.panic = CPU_ERR_PANIC;
    }
}

void op_push(CPU *cpu, word_t word) {
    uint8_t r = ((uint8_t *) &word)[1];

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

void op_pop(CPU *cpu, word_t word) {
    uint8_t r = ((uint8_t *) &word)[1];

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
void op_mov_rw(CPU *cpu, word_t word) {
    uint8_t to = ((uint8_t *) &word)[1];

    word_t val = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tMOV  r%i, "WXHEX"\n", cpu->debug.step, cpu->pc - 2 * ADDR_SIZE, to, val);
    }
    cpu->state.panic = cpu_register_set(cpu, to, val);
}

// todo implement support for special registers
void op_mov_rr(CPU *cpu, word_t word) {
    uint8_t to = ((uint8_t *) &word)[1];
    uint8_t from = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tMOV  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, to, from);
    }

    word_t value;
    if ((cpu->state.panic = cpu_register_get(cpu, from, &value)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, to, value);
}

void op_cmp(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tCMP  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    word_t b_val;
    if ((cpu->state.panic = cpu_register_get(cpu, b, &b_val)) != CPU_ERR_OK) {
        return;
    }
    cpu_flags_update(cpu, (sword_t) a_val - (sword_t) b_val);
}

void op_jreq(CPU *cpu, word_t word) {
    addr_t address = 0;
    addr_t pc = cpu->pc;
    if (cpu->flags.zero == 1 || cpu->debug.print_op) {
        address = cpu_fetch(cpu);
        if (cpu->state.panic != CPU_ERR_OK) {
            return;
        }

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

void op_jrne(CPU *cpu, word_t word) {
    addr_t address = 0;
    addr_t pc = cpu->pc;
    if (cpu->flags.zero == 0 || cpu->debug.print_op) {
        address = cpu_fetch(cpu);
        if (cpu->state.panic != CPU_ERR_OK) {
            return;
        }

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

void op_jr(CPU *cpu, word_t word) {
    word_t address = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tJ    "AXHEX"\n", cpu->debug.step, cpu->pc - 2 * ADDR_SIZE, address);
    }

    cpu->pc = cpu->cs + address;
}

void op_stor(CPU *cpu, word_t word) {
    uint8_t to = ((uint8_t *) &word)[1];
    uint8_t from = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tSTOR r%i, r%i\t\t", cpu->debug.step, cpu->pc - ADDR_SIZE, to, from);
    }

    word_t address;
    if ((cpu->state.panic = cpu_register_get(cpu, to, &address)) != CPU_ERR_OK) {
        return;
    }
    word_t val;
    if ((cpu->state.panic = cpu_register_get(cpu, from, &val)) != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("// r%i="AXHEX"\tr%i="AXHEX"\n", from, val, to, address);
    }
    if (bus_word_write(cpu->bus, (addr_t) address, val) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
    }
}

void op_load(CPU *cpu, word_t word) {
    uint8_t to = ((uint8_t *) &word)[1];
    uint8_t from = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tLOAD r%i, r%i\t\t", cpu->debug.step, cpu->pc - ADDR_SIZE, to, from);
    }

    word_t address;
    if ((cpu->state.panic = cpu_register_get(cpu, from, &address)) != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("// r%i="AXHEX"\n", from, address);
    }

    word_t val;
    if ((bus_word_read(cpu->bus, (addr_t) address, &val)) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, to, val);
}

void op_add_rr(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tADD  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    word_t b_val;
    if ((cpu->state.panic = cpu_register_get(cpu, b, &b_val)) != CPU_ERR_OK) {
        return;
    }
    // todo implement overflow/underflow
    cpu->state.panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val + (sword_t) b_val));
}

void op_sub_rw(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    word_t value = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tSUB  r%i, "WXHEX"\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, value);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }

    // todo implement overflow/underflow
    cpu->state.panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val - (sword_t) value));
}

void op_mul_rr(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tMUL  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    word_t b_val;
    if ((cpu->state.panic = cpu_register_get(cpu, b, &b_val)) != CPU_ERR_OK) {
        return;
    }
    // todo implement overflow/underflow
    cpu->state.panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val * (sword_t) b_val));
}

void op_dec(CPU *cpu, word_t word) {
    uint8_t r = ((uint8_t *) &word)[1];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tDEC  r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, r);
    }

    word_t r_val;
    if ((cpu->state.panic = cpu_register_get(cpu, r, &r_val)) != CPU_ERR_OK) {
        return;
    }
    // todo implement underflow
    cpu->state.panic = cpu_register_set(cpu, r, (word_t) ((sword_t) r_val - 1));
}

void op_inc(CPU *cpu, word_t word) {
    uint8_t r = ((uint8_t *) &word)[1];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tINC  r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, r);
    }

    word_t r_val;
    if ((cpu->state.panic = cpu_register_get(cpu, r, &r_val)) != CPU_ERR_OK) {
        return;
    }
    // todo implement underflow
    cpu->state.panic = cpu_register_set(cpu, r, (word_t) ((sword_t) r_val + 1));
}

void op_call(CPU *cpu, word_t word) {
    addr_t address = (addr_t) cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tCALL "AXHEX"\n", cpu->debug.step, cpu->pc - 2 * ADDR_SIZE, address);
    }

    cpu->sp -= WORD_SIZE;
    if (bus_word_write(cpu->bus, cpu->sp, cpu->pc) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        return;
    }
    cpu->pc = cpu->cs + address;
}

void op_ret(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tRET\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }

    if (bus_word_read(cpu->bus, cpu->sp, &cpu->pc) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
}

op_ptr ops[OPS_COUNT] = {
        &op_nop,
        &op_halt,
        &op_panic,
        &op_mov_rw,
        &op_mov_rr,
        &op_add_rr,
        NULL,
        NULL,
        &op_sub_rw,
        &op_mul_rr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &op_inc,
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
        &op_load,
        &op_call,
        &op_ret,
};


