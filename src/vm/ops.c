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
#include <stdlib.h>
#include "vmarch.h"
#include "cpu.h"
#include "cpu_internal.h"

char *register_name(uint8_t from) {
    char *buf = malloc(5);
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
        case REG_BP:
            strcpy(buf, "bp");
            break;
        case REG_SP:
            strcpy(buf, "sp");
            break;
        default:
            sprintf(buf, "r%i", from);
    }
    return buf;
}

#define PRINT_INSTRUCTION(cpu, word) { \
   if ((cpu)->debug.print_op) { \
        char *s = cpu_instruction_to_string((cpu), (word)); \
        printf("%s\n", s); \
        free(s); \
    } \
}

void op_nop(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)
}

void op_halt(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)
    cpu->state.running = 0;
}

void op_panic(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)
    if (!cpu->state.panic) {
        cpu->state.panic = CPU_ERR_PANIC;
    }
}

void op_push_r(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t r = ((uint8_t *) &word)[1];
    word_t value;
    if ((cpu->state.panic = cpu_register_get(cpu, r, &value)) == CPU_ERR_OK) {
        cpu->sp -= WORD_SIZE;
        if (bus_word_write(cpu->bus, cpu->sp, value) != BUS_ERR_OK) {
            cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        }
    }
}

void op_push_w(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    word_t val = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }
    cpu->sp -= WORD_SIZE;
    if (bus_word_write(cpu->bus, cpu->sp, val) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
    }
}

void op_push_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t r0 = ((uint8_t *) &word)[1];
    uint8_t r1 = ((uint8_t *) &word)[2];

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

void op_push_rrr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t r0 = ((uint8_t *) &word)[1];
    uint8_t r1 = ((uint8_t *) &word)[2];
    uint8_t r2 = ((uint8_t *) &word)[3];

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

void op_pusha(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

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

void op_pop_r(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t r = ((uint8_t *) &word)[1];
    word_t value;
    if (bus_word_read(cpu->bus, cpu->sp, &value) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
    cpu->state.panic = cpu_register_set(cpu, r, value);
}

void op_pop_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t r0 = ((uint8_t *) &word)[1];
    uint8_t r1 = ((uint8_t *) &word)[2];

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

void op_pop_rrr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t r0 = ((uint8_t *) &word)[1];
    uint8_t r1 = ((uint8_t *) &word)[2];
    uint8_t r2 = ((uint8_t *) &word)[3];

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

void op_popa(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

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

void op_mov_rw(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t to = ((uint8_t *) &word)[1];
    word_t val = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, to, val);
}

void op_mov_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t to = ((uint8_t *) &word)[1];
    uint8_t from = ((uint8_t *) &word)[2];

    word_t value;
    if ((cpu->state.panic = cpu_register_get(cpu, from, &value)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, to, value);
}

void op_cmp_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

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

void op_cmp_rw(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    word_t b_val = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    cpu_flags_update(cpu, (sword_t) a_val - (sword_t) b_val);
}

void op_jeq_s(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    addr_t address = 0;
    addr_t pc = cpu->pc;
    if (cpu->flags.zero == 1) {
        address = cpu_fetch(cpu);
        if (cpu->state.panic != CPU_ERR_OK) {
            return;
        }
    }

    cpu->pc = (cpu->flags.zero == 1) * (cpu->cs + address) // if equals, jump
              + (cpu->flags.zero == 0) * (pc + ADDR_SIZE); // otherwise, move to next opcode
}

void op_jeq_a(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    addr_t address = 0;
    addr_t pc = cpu->pc;
    if (cpu->flags.zero == 1) {
        address = cpu_fetch(cpu);
        if (cpu->state.panic != CPU_ERR_OK) {
            return;
        }
    }

    cpu->pc = (cpu->flags.zero == 1) * address             // if equals, jump
              + (cpu->flags.zero == 0) * (pc + ADDR_SIZE); // otherwise, move to next opcode
}

void op_jne_s(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    addr_t address = 0;
    addr_t pc = cpu->pc;
    if (cpu->flags.zero == 0) {
        address = cpu_fetch(cpu);
        if (cpu->state.panic != CPU_ERR_OK) {
            return;
        }
    }

    cpu->pc = (cpu->flags.zero == 0) * (cpu->cs + address) // if not equals, jump
              + (cpu->flags.zero == 1) * (pc + ADDR_SIZE); // otherwise, move to next word
}

void op_jne_a(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    addr_t address = 0;
    addr_t pc = cpu->pc;
    if (cpu->flags.zero == 0) {
        address = cpu_fetch(cpu);
        if (cpu->state.panic != CPU_ERR_OK) {
            return;
        }
    }

    cpu->pc = (cpu->flags.zero == 0) * address             // if not equals, jump
              + (cpu->flags.zero == 1) * (pc + ADDR_SIZE); // otherwise, move to next word
}

void op_j_s(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    word_t address = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    cpu->pc = cpu->cs + address;
}

void op_j_a(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    word_t address = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    cpu->pc = address;
}

void op_stor_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t to = ((uint8_t *) &word)[1];
    uint8_t from = ((uint8_t *) &word)[2];

    word_t address;
    if ((cpu->state.panic = cpu_register_get(cpu, to, &address)) != CPU_ERR_OK) {
        return;
    }
    word_t val;
    if ((cpu->state.panic = cpu_register_get(cpu, from, &val)) != CPU_ERR_OK) {
        return;
    }

    if (bus_word_write(cpu->bus, (addr_t) address, val) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
    }
}

void op_load_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t to = ((uint8_t *) &word)[1];
    uint8_t from = ((uint8_t *) &word)[2];

    word_t address;
    if ((cpu->state.panic = cpu_register_get(cpu, from, &address)) != CPU_ERR_OK) {
        return;
    }

    word_t val;
    if ((bus_word_read(cpu->bus, (addr_t) address, &val)) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, to, val);
}

void op_load_rrw(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t to = ((uint8_t *) &word)[1];
    uint8_t from = ((uint8_t *) &word)[2];

    word_t address;
    if ((cpu->state.panic = cpu_register_get(cpu, from, &address)) != CPU_ERR_OK) {
        return;
    }
    word_t offset = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    word_t val;
    if ((bus_word_read(cpu->bus, (addr_t) address + (addr_t) offset, &val)) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, to, val);
}

void op_add_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

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

void op_add_rw(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    word_t value = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }

    // todo implement overflow/underflow
    cpu->state.panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val + (sword_t) value));
}

void op_sub_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

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

void op_sub_rw(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    word_t value = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }

    // todo implement overflow/underflow
    cpu->state.panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val - (sword_t) value));
}

void op_mul_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

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

void op_mul_rw(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    word_t value = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }

    // todo implement overflow/underflow
    cpu->state.panic = cpu_register_set(cpu, a, (word_t) ((sword_t) a_val * (sword_t) value));
}

void op_and_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

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

void op_and_rw(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    word_t b_val = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, a, a_val & b_val);
}

void op_or_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

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

void op_or_rw(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    word_t b_val = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, a, a_val | b_val);
}

void op_xor_rr(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    uint8_t b = ((uint8_t *) &word)[2];

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

void op_xor_rw(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t a = ((uint8_t *) &word)[1];
    word_t b_val = cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    word_t a_val;
    if ((cpu->state.panic = cpu_register_get(cpu, a, &a_val)) != CPU_ERR_OK) {
        return;
    }
    cpu->state.panic = cpu_register_set(cpu, a, a_val ^ b_val);
}

void op_dec_r(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t r = ((uint8_t *) &word)[1];

    word_t r_val;
    if ((cpu->state.panic = cpu_register_get(cpu, r, &r_val)) != CPU_ERR_OK) {
        return;
    }
    // todo implement underflow
    cpu->state.panic = cpu_register_set(cpu, r, (word_t) ((sword_t) r_val - 1));
}

void op_inc_r(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t r = ((uint8_t *) &word)[1];
    word_t r_val;
    if ((cpu->state.panic = cpu_register_get(cpu, r, &r_val)) != CPU_ERR_OK) {
        return;
    }
    // todo implement underflow
    cpu->state.panic = cpu_register_set(cpu, r, (word_t) ((sword_t) r_val + 1));
}

void op_call_s(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    addr_t address = (addr_t) cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    cpu->sp -= WORD_SIZE;
    if (bus_word_write(cpu->bus, cpu->sp, cpu->pc) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        return;
    }
    cpu->pc = cpu->cs + address;
}

void op_call_a(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    addr_t address = (addr_t) cpu_fetch(cpu);
    if (cpu->state.panic != CPU_ERR_OK) {
        return;
    }

    cpu->sp -= WORD_SIZE;
    if (bus_word_write(cpu->bus, cpu->sp, cpu->pc) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        return;
    }
    cpu->pc = address;
}

void op_call_r(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t r = ((uint8_t *) &word)[1];
    word_t address;
    if ((cpu->state.panic = cpu_register_get(cpu, r, &address)) != CPU_ERR_OK) {
        return;
    }

    cpu->sp -= WORD_SIZE;
    if (bus_word_write(cpu->bus, cpu->sp, cpu->pc) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
        return;
    }
    cpu->pc = address;
}

void op_ret(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    if (bus_word_read(cpu->bus, cpu->sp, &cpu->pc) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
}

void op_iret(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    if (bus_word_read(cpu->bus, cpu->sp, &cpu->pc) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return;
    }
    cpu->sp += WORD_SIZE;
    cpu->flags.interrupts_enabled = 1;
}

void op_int_b(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t interrupt = ((uint8_t *) &word)[1];
    cpu_interrupt_trigger(cpu, interrupt);
}

void op_mi_b(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t interrupt = ((uint8_t *) &word)[1];
    pic_interrupt_mask(cpu->pic, interrupt);
}

void op_umi_b(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)

    uint8_t interrupt = ((uint8_t *) &word)[1];
    pic_interrupt_unmask(cpu->pic, interrupt);
}

void op_ind(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)
    cpu->flags.interrupts_enabled = 0;
}

void op_ine(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)
    cpu->flags.interrupts_enabled = 1;
}

void op_wfi(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)
    int rc = pthread_mutex_lock(&pic_got_interrupt_lock);
    if (rc) {
        perror("pthread_mutex_lock");
        pthread_exit(NULL);
    }

    pthread_cond_wait(&pic_got_interrupt, &pic_got_interrupt_lock);
    pthread_mutex_unlock(&pic_got_interrupt_lock);
}

void op_xbm(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)
}

void op_xbrk(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)
    cpu->debug.trap = 1;
}

void op_xdbg(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)
}

void op_xpse(CPU *cpu, word_t word) {
    cpu->debug.print_op = 1;
    PRINT_INSTRUCTION(cpu, word)
}

void op_xpsd(CPU *cpu, word_t word) {
    PRINT_INSTRUCTION(cpu, word)
    cpu->debug.print_op = 0;
}

#include "ops_array.h"

char *cpu_instruction_to_string(CPU *cpu, word_t word) {
    char *buffer = malloc(128);
    uint8_t op = ((uint8_t *) &word)[0];
    addr_t addr = cpu->pc - ADDR_SIZE;
    switch (op) {
        case HALT:
        case IRET:
        case IND:
        case INE:
        case NOP:
        case PANIC:
        case PUSHA:
        case POPA:
        case RET:
        case WFI:
        case XBM:
        case XBRK:
        case XDBG:
        case XPSE:
        case XPSD:
            sprintf(buffer, "  %lu\t"AXHEX"\t%s", cpu->debug.step, addr, ops_name[op]);
            break;
        case CALL_R:
        case DEC_R:
        case INC_R:
        case POP_R:
        case PUSH_R: {
            char *r = register_name(((uint8_t *) &word)[1]);
            sprintf(buffer, "  %lu\t"AXHEX"\t%s %s", cpu->debug.step, addr, ops_name[op], r);
            free(r);
            break;
        }
        case MI_B :
        case UMI_B:
        case INT_B: {
            uint8_t b = ((uint8_t *) &word)[1];
            sprintf(buffer, "  %lu\t"AXHEX"\t%s %i", cpu->debug.step, addr, ops_name[op], b);
            break;
        }
        case ADD_RR:
        case AND_RR:
        case CMP_RR:
        case LOAD_RR:
        case OR_RR:
        case MOV_RR:
        case MUL_RR:
        case POP_RR:
        case PUSH_RR:
        case SUB_RR:
        case STOR_RR:
        case XOR_RR: {
            char *r1 = register_name(((uint8_t *) &word)[1]);
            char *r2 = register_name(((uint8_t *) &word)[2]);
            sprintf(buffer, "  %lu\t"AXHEX"\t%s %s, %s", cpu->debug.step, addr, ops_name[op], r1, r2);
            free(r1);
            free(r2);
            break;
        }
        case POP_RRR:
        case PUSH_RRR: {
            char *r1 = register_name(((uint8_t *) &word)[1]);
            char *r2 = register_name(((uint8_t *) &word)[2]);
            char *r3 = register_name(((uint8_t *) &word)[3]);
            sprintf(buffer, "  %lu\t"AXHEX"\t%s %s, %s, %s", cpu->debug.step, addr, ops_name[op], r1, r2, r3);
            free(r1);
            free(r2);
            free(r3);
            break;
        }
        case ADD_RW:
        case AND_RW:
        case CMP_RW:
        case MOV_RW:
        case MUL_RW:
        case OR_RW:
        case SUB_RW:
        case XOR_RW: {
            char *r = register_name(((uint8_t *) &word)[1]);
            word_t val = cpu_read_pc_word(cpu, 0);
            if (cpu->state.panic != CPU_ERR_OK) {
                sprintf(buffer, "  %lu\t"AXHEX"\t%s %s, %s", cpu->debug.step, addr, ops_name[op], r, "ERR");
            } else {
                sprintf(buffer, "  %lu\t"AXHEX"\t%s %s, "WXHEX, cpu->debug.step, addr, ops_name[op], r, val);
            }
            free(r);
            break;
        }
        case CALL_A:
        case J_A: {
            word_t val = cpu_read_pc_word(cpu, 0);
            if (cpu->state.panic != CPU_ERR_OK) {
                sprintf(buffer, "  %lu\t"AXHEX"\t%s %s", cpu->debug.step, addr, ops_name[op], "ERR");
            } else {
                sprintf(buffer, "  %lu\t"AXHEX"\t%s "AXHEX, cpu->debug.step, addr, ops_name[op], val);
            }
            break;
        }
        case CALL_S:
        case J_S: {
            word_t address = cpu_read_pc_word(cpu, 0);
            if (cpu->state.panic != CPU_ERR_OK) {
                sprintf(buffer, "  %lu\t"AXHEX"\t%s %s", cpu->debug.step, addr, ops_name[op], "ERR");
            } else {
                sprintf(buffer, "  %lu\t"AXHEX"\t%s "AXHEX"\t\t// cs="AXHEX"",
                        cpu->debug.step,
                        addr,
                        ops_name[op],
                        address, cpu->cs
                );
            }
            break;
        }
        case JEQ_A:
        case JNE_A: {
            addr_t address = cpu_read_pc_word(cpu, 0);
            if (cpu->state.panic != CPU_ERR_OK) {
                sprintf(buffer, "  %lu\t"AXHEX"\t%s %s", cpu->debug.step, addr, ops_name[op], "ERR");
            } else {
                sprintf(buffer, "  %lu\t"AXHEX"\t%s "AXHEX"\t\t// z=%i",
                        cpu->debug.step,
                        addr,
                        ops_name[op],
                        address,
                        cpu->flags.zero
                );
            }
            break;
        }
        case JEQ_S:
        case JNE_S: {
            addr_t address = cpu_read_pc_word(cpu, 0);
            if (cpu->state.panic != CPU_ERR_OK) {
                sprintf(buffer, "  %lu\t"AXHEX"\t%s %s", cpu->debug.step, addr, ops_name[op], "ERR");
            } else {
                sprintf(buffer, "  %lu\t"AXHEX"\t%s "AXHEX"\t\t// z=%i       cs="AXHEX"",
                        cpu->debug.step,
                        addr,
                        ops_name[op],
                        address,
                        cpu->flags.zero,
                        cpu->cs
                );
            }
            break;
        }
        default:
            sprintf(buffer, "  %lu\t"AXHEX"\t"WXHEX" ???", cpu->debug.step, addr, op);
    }
    return buffer;
}