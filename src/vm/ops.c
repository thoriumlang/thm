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

#include <string.h>
#include <pthread.h>
#include "vmarch.h"
#include "cpu.h"
#include "cpu_internal.h"

void register_name(uint8_t from, char *buf) {
    switch (from) {
        case REG_IR:
            strcpy(buf, "ir");
            break;
        case REG_IDT:
            strcpy(buf, "idt");
            break;
        case REG_CS:
            strcpy(buf, "cs");
            break;
        case REG_PC:
            strcpy(buf, "pc");
            break;
        case REG_SP:
            strcpy(buf, "sp");
            break;
        default:
            sprintf(buf, "r%i", from);
    }
}

#define OP_NOP
void op_nop(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tNOP\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
}

#define OP_HALT
void op_halt(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tHALT\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
    cpu->state.running = 0;
}

#define OP_PANIC
void op_panic(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPANIC\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
    if (!cpu->state.panic) {
        cpu->state.panic = CPU_ERR_PANIC;
    }
}

#define OP_PUSH
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

#define OP_PUSH_RR
void op_push_rr(CPU *cpu, word_t word) {
    uint8_t r0 = ((uint8_t *) &word)[1];
    uint8_t r1 = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPUSH r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, r0, r1);
    }

    word_t value;
    if ((cpu->state.panic = cpu_register_get(cpu, r0, &value)) == CPU_ERR_OK) {
        cpu->sp -= WORD_SIZE;
        if (bus_word_write(cpu->bus, cpu->sp, value) != BUS_ERR_OK) {
            cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        }
    }
    if ((cpu->state.panic = cpu_register_get(cpu, r1, &value)) == CPU_ERR_OK) {
        cpu->sp -= WORD_SIZE;
        if (bus_word_write(cpu->bus, cpu->sp, value) != BUS_ERR_OK) {
            cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        }
    }
}

#define OP_PUSH_RRR
void op_push_rrr(CPU *cpu, word_t word) {
    uint8_t r0 = ((uint8_t *) &word)[1];
    uint8_t r1 = ((uint8_t *) &word)[2];
    uint8_t r2 = ((uint8_t *) &word)[3];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPUSH r%i, r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, r0, r1, r2);
    }

    word_t value;
    if ((cpu->state.panic = cpu_register_get(cpu, r0, &value)) == CPU_ERR_OK) {
        cpu->sp -= WORD_SIZE;
        if (bus_word_write(cpu->bus, cpu->sp, value) != BUS_ERR_OK) {
            cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        }
    }
    if ((cpu->state.panic = cpu_register_get(cpu, r1, &value)) == CPU_ERR_OK) {
        cpu->sp -= WORD_SIZE;
        if (bus_word_write(cpu->bus, cpu->sp, value) != BUS_ERR_OK) {
            cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        }
    }
    if ((cpu->state.panic = cpu_register_get(cpu, r2, &value)) == CPU_ERR_OK) {
        cpu->sp -= WORD_SIZE;
        if (bus_word_write(cpu->bus, cpu->sp, value) != BUS_ERR_OK) {
            cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        }
    }
}

#define OP_PUSHA
void op_pusha(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPUSHA\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }

    for (int r = 0; r < cpu->register_count; r++) {
        word_t value;
        if ((cpu->state.panic = cpu_register_get(cpu, r, &value)) == CPU_ERR_OK) {
            cpu->sp -= WORD_SIZE;
            if (bus_word_write(cpu->bus, cpu->sp, value) != BUS_ERR_OK) {
                cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
            }
        }
        //printf("push r%i=%i\n", r, value);
    }
}

#define OP_POP
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

#define OP_POP_RR
void op_pop_rr(CPU *cpu, word_t word) {
    uint8_t r0 = ((uint8_t *) &word)[1];
    uint8_t r1 = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPOP  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, r0, r1);
    }

    word_t value;
    if (bus_word_read(cpu->bus, cpu->sp, &value) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
    cpu->state.panic = cpu_register_set(cpu, r0, value);

    if (bus_word_read(cpu->bus, cpu->sp, &value) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
    cpu->state.panic = cpu_register_set(cpu, r1, value);
}

#define OP_POP_RRR
void op_pop_rrr(CPU *cpu, word_t word) {
    uint8_t r0 = ((uint8_t *) &word)[1];
    uint8_t r1 = ((uint8_t *) &word)[2];
    uint8_t r2 = ((uint8_t *) &word)[3];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPOP  r%i, r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, r0, r1, r2);
    }

    word_t value;
    if (bus_word_read(cpu->bus, cpu->sp, &value) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
    cpu->state.panic = cpu_register_set(cpu, r0, value);

    if (bus_word_read(cpu->bus, cpu->sp, &value) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
    cpu->state.panic = cpu_register_set(cpu, r1, value);

    if (bus_word_read(cpu->bus, cpu->sp, &value) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
    cpu->state.panic = cpu_register_set(cpu, r2, value);
}

#define OP_POPA
void op_popa(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tPOPA\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }

    for (int r = cpu->register_count - 1; r >= 0; r--) {
        word_t value;
        if (bus_word_read(cpu->bus, cpu->sp, &value) != BUS_ERR_OK) {
            cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
            return;
        }
        cpu->sp += WORD_SIZE;
        cpu->state.panic = cpu_register_set(cpu, r, value);
        //printf("pop r%i=%i\n", r, value);
    }
}

#define OP_MOV_RW// todo implement support for special registers
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


#define OP_MOV_RR// todo implement support for special registers
void op_mov_rr(CPU *cpu, word_t word) {
    uint8_t to = ((uint8_t *) &word)[1];
    uint8_t from = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        char from_name[5];
        register_name(from, from_name);
        printf("  %lu\t"AXHEX"\tMOV  r%i, %s\n", cpu->debug.step, cpu->pc - ADDR_SIZE, to, from_name);
    }

    word_t value;
    switch (from) {
        case REG_IDT:
            value = cpu_idt_get(cpu);
            break;
        case REG_IR:
            value = cpu_ir_get(cpu);
            break;
        default:
            if ((cpu->state.panic = cpu_register_get(cpu, from, &value)) != CPU_ERR_OK) {
                return;
            }
    }
    cpu->state.panic = cpu_register_set(cpu, to, value);
}

#define OP_CMP
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

#define OP_JSEQ
void op_jseq(CPU *cpu, word_t word) {
    addr_t address = 0;
    addr_t pc = cpu->pc;
    if (cpu->flags.zero == 1 || cpu->debug.print_op) {
        address = cpu_fetch(cpu);
        if (cpu->state.panic != CPU_ERR_OK) {
            return;
        }

        if (cpu->debug.print_op) {
            printf("  %lu\t"AXHEX"\tJEQ  "AXHEX"\t\t// cs="AXHEX"    z=%i -> %s\n",
                   cpu->debug.step,
                   cpu->pc - 2 * ADDR_SIZE,
                   address,
                   cpu->cs,
                   cpu->flags.zero,
                   cpu->flags.zero == 0 ? "no jump" : "jump"
            );
        }
    }

    cpu->pc = (cpu->flags.zero == 1) * (cpu->cs + address) // if equals, jump
              + (cpu->flags.zero == 0) * (pc + ADDR_SIZE); // otherwise, move to next opcode
}

#define OP_JAEQ
void op_jaeq(CPU *cpu, word_t word) {
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

    cpu->pc = (cpu->flags.zero == 1) * address             // if equals, jump
              + (cpu->flags.zero == 0) * (pc + ADDR_SIZE); // otherwise, move to next opcode
}

#define OP_JSNE
void op_jsne(CPU *cpu, word_t word) {
    addr_t address = 0;
    addr_t pc = cpu->pc;
    if (cpu->flags.zero == 0 || cpu->debug.print_op) {
        address = cpu_fetch(cpu);
        if (cpu->state.panic != CPU_ERR_OK) {
            return;
        }

        if (cpu->debug.print_op) {
            printf("  %lu\t"AXHEX"\tJNE  "AXHEX"\t\t// cs="AXHEX"    z=%i -> %s\n",
                   cpu->debug.step,
                   cpu->pc - 2 * ADDR_SIZE,
                   address,
                   cpu->cs,
                   cpu->flags.zero,
                   cpu->flags.zero == 0 ? "jump" : "no jump"
            );
        }
    }

    cpu->pc = (cpu->flags.zero == 0) * (cpu->cs + address) // if not equals, jump
              + (cpu->flags.zero == 1) * (pc + ADDR_SIZE); // otherwise, move to next word
}

#define OP_JANE
void op_jane(CPU *cpu, word_t word) {
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

    cpu->pc = (cpu->flags.zero == 0) * address             // if not equals, jump
              + (cpu->flags.zero == 1) * (pc + ADDR_SIZE); // otherwise, move to next word
}

#define OP_JS
void op_js(CPU *cpu, word_t word) {
    word_t address = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tJ    "AXHEX"\t\t// cs="AXHEX"\n", cpu->debug.step, cpu->pc - 2 * ADDR_SIZE, address, cpu->cs);
    }

    cpu->pc = cpu->cs + address;
}

#define OP_JA
void op_ja(CPU *cpu, word_t word) {
    word_t address = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tJ    "AXHEX"\n", cpu->debug.step, cpu->pc - 2 * ADDR_SIZE, address);
    }

    cpu->pc = address;
}

#define OP_STOR
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

#define OP_LOAD
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

#define OP_ADD_RR
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

#define OP_ADD_RW
void op_add_rw(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    word_t value = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tADD  r%i, "WXHEX"\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, value);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }

    // todo implement overflow/underflow
    cpu->state.panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val + (sword_t) value));
}

#define OP_SUB_RR
void op_sub_rr(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tSUB  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b);
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
    cpu->state.panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val - (sword_t) b_val));
}

#define OP_SUB_RW
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

#define OP_MUL_RR
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

#define OP_MUL_RW
void op_mul_rw(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    word_t value = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tMUL  r%i, "WXHEX"\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, value);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }

    // todo implement overflow/underflow
    cpu->state.panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val * (sword_t) value));
}

#define OP_AND_RR
void op_and_rr(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tAND  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    word_t b_val;
    if ((cpu->state.panic = cpu_register_get(cpu, b, &b_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, a, a_val & b_val);
}

#define OP_AND_RW
void op_and_rw(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    word_t b_val = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tAND  r%i, "WXHEX"\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b_val);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, a, a_val & b_val);
}

#define OP_OR_RR
void op_or_rr(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tOR   r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    word_t b_val;
    if ((cpu->state.panic = cpu_register_get(cpu, b, &b_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, a, a_val | b_val);
}

#define OP_OR_RW
void op_or_rw(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    word_t b_val = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tOR   r%i, "WXHEX"\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b_val);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, a, a_val | b_val);
}

#define OP_XOR_RR
void op_xor_rr(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tXOR  r%i, r%i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    word_t b_val;
    if ((cpu->state.panic = cpu_register_get(cpu, b, &b_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, a, a_val ^ b_val);
}

#define OP_XOR_RW
void op_xor_rw(CPU *cpu, word_t word) {
    uint8_t a = ((uint8_t *) &word)[1];
    word_t b_val = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tXOR  r%i, "WXHEX"\n", cpu->debug.step, cpu->pc - ADDR_SIZE, a, b_val);
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, a, a_val ^ b_val);
}

#define OP_DEC
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

#define OP_INC
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

#define OP_CALLS
void op_calls(CPU *cpu, word_t word) {
    addr_t address = (addr_t) cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tCALL "AXHEX"\t\t// cs="AXHEX"\n", cpu->debug.step, cpu->pc - 2 * ADDR_SIZE, address, cpu->cs);
    }

    cpu->sp -= WORD_SIZE;
    if (bus_word_write(cpu->bus, cpu->sp, cpu->pc) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        return;
    }
    cpu->pc = cpu->cs + address;
}

#define OP_CALLA_A
void op_calla_a(CPU *cpu, word_t word) {
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
    cpu->pc = address;
}

#define OP_CALLA_R
void op_calla_r(CPU *cpu, word_t word) {
    uint8_t r = ((uint8_t *) &word)[1];

    word_t address;
    if ((cpu->state.panic = cpu_register_get(cpu, r, &address)) != CPU_ERR_OK) {
        return;
    }

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tCALL r%i\n", cpu->debug.step, cpu->pc - 2 * ADDR_SIZE, r);
    }

    cpu->sp -= WORD_SIZE;
    if (bus_word_write(cpu->bus, cpu->sp, cpu->pc) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        return;
    }
    cpu->pc = address;
}

#define OP_RET
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

#define OP_IRET
void op_iret(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tIRET\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }

    if (bus_word_read(cpu->bus, cpu->sp, &cpu->pc) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
    cpu->flags.interrupts_enabled = 1;
}

#define OP_INT
void op_int(CPU *cpu, word_t word) {
    uint8_t interrupt = ((uint8_t *) &word)[1];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tINT  %i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, interrupt);
    }

    cpu_interrupt_trigger(cpu, interrupt);
}

#define OP_MI
void op_mi(CPU *cpu, word_t word) {
    uint8_t interrupt = ((uint8_t *) &word)[1];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tMI   %i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, interrupt);
    }

    pic_interrupt_mask(cpu->pic, interrupt);
}

#define OP_UMI
void op_umi(CPU *cpu, word_t word) {
    uint8_t interrupt = ((uint8_t *) &word)[1];

    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tUMI  %i\n", cpu->debug.step, cpu->pc - ADDR_SIZE, interrupt);
    }

    pic_interrupt_unmask(cpu->pic, interrupt);
}

#define OP_IND
void op_ind(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tIND\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }

    cpu->flags.interrupts_enabled = 0;
}

#define OP_INE
void op_ine(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tINE\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }

    cpu->flags.interrupts_enabled = 1;
}

#define OP_WFI
void op_wfi(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) {
        printf("  %lu\t"AXHEX"\tWFI\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
    int rc = pthread_mutex_lock(&pic_got_interrupt_lock);
    if (rc) {
        perror("pthread_mutex_lock");
        pthread_exit(NULL);
    }

    pthread_cond_wait(&pic_got_interrupt, &pic_got_interrupt_lock);
    pthread_mutex_unlock(&pic_got_interrupt_lock);
}

#define OP_XDBG
void op_xdbg(CPU *cpu, word_t word) {
    if (cpu->debug.print_op) { // todo guard with debug cli options
        printf("  %lu\t"AXHEX"\tXDBG\n", cpu->debug.step, cpu->pc - ADDR_SIZE);
    }
}

#include "ops_array.h"
