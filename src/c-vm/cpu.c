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

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"
#include "cpu_internal.h"
#include "bus.h"
#include "ops.h"

op_ptr decode(CPU *cpu, const word_t *word);

CPU *cpu_create(Bus *bus, uint8_t reg_count) {
    // printf("sizeof(CPU)=%lu", sizeof(CPU));
    CPU *cpu = malloc(sizeof(CPU));
    cpu->bus = bus;
    cpu->register_count = reg_count;
    cpu->registers = malloc(sizeof(*cpu->registers) * reg_count);
    cpu->step = 0;

    cpu_reset(cpu);

    return cpu;
}

void cpu_start(CPU *cpu) {
    cpu->running = 1;
    cpu->panic = CPU_ERR_OK;
    if (cpu->print_op) {
        printf("\nCPU Steps\n");
    }
    while (cpu->running == 1 && cpu->panic == CPU_ERR_OK) {
        word_t word = fetch(cpu);
        if (!cpu->panic) {
            op_ptr op = decode(cpu, &word);
            if (!cpu->panic) {
                if (op) {
                    op(cpu, &word);
                    cpu->step++;
                } else {
                    cpu->panic = CPU_ERR_UNIMPLEMENTED_OPCODE;
                }
            }
        }
    }
    cpu->running = 0;
}


word_t fetch(CPU *cpu) {
    word_t word;
    if (bus_word_read(cpu->bus, cpu->pc, &word) != BUS_ERR_OK) {
        cpu->panic = CPU_ERR_CANNOT_READ_MEMORY;
        return 0;
    }
    cpu->pc += ADDR_SIZE;
    return word;
}

op_ptr decode(CPU *cpu, const word_t *word) {
    if (cpu->panic) {
        return NULL;
    }
    uint8_t op = ((uint8_t *) word)[0];
    if (op >= OPS_COUNT) {
        cpu->panic = CPU_ERR_UNKNOWN_OPCODE;
        return NULL;
    }
    return ops[op];
}

void cpu_stop(CPU *cpu) {
    cpu->running = 0;
}

void cpu_reset(CPU *cpu) {
    for (int r = 0; r < 32; r++) {
        cpu->registers[r] = 0;
    }
    cpu->pc = 0;
    cpu->cs = 0;
    cpu->sp = STACK_SIZE;
    cpu->print_op = 0;
}

void cpu_destroy(CPU *cpu) {
    free(cpu->registers);
    free(cpu);
}

CpuError cpu_register_get(CPU *cpu, uint8_t reg, word_t *word) {
    if (reg >= cpu->register_count) {
        return CPU_ERR_INVALID_REGISTER;
    }
    *word = cpu->registers[reg];
    return CPU_ERR_OK;
}

CpuError cpu_register_set(CPU *cpu, uint8_t reg, word_t value) {
    if (reg >= cpu->register_count) {
        return CPU_ERR_INVALID_REGISTER;
    }
    cpu->registers[reg] = value;
    update_flags(cpu, (sword_t) value);
    return CPU_ERR_OK;
}

void update_flags(CPU *cpu, sword_t value) {
    cpu->zero = (value == 0) ? 1 : 0;
    cpu->negative = ((sword_t) value < 0) ? 1 : 0;
}

void cpu_set_pc(CPU *cpu, addr_t address) {
    cpu->pc = address;
}

void cpu_set_cs(CPU *cpu, addr_t address) {
    cpu->cs = address;
}

void cpu_print_op_enable(CPU *cpu) {
    cpu->print_op = 1;
}

void cpu_print_op_disable(CPU *cpu) {
    cpu->print_op = 0;
}

void cpu_debug_enable(CPU *cpu) {
    cpu->debug = 1;
}

void cpu_debug_disable(CPU *cpu) {
    cpu->debug = 0;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

void cpu_print_state(FILE *file, CPU *cpu) {
    // fprintf(file, "\nCPU state\n");
    for (int r = 0; r < cpu->register_count; r++) {
        if (r % 4 == 0) {
            fprintf(file, "  r%03u - r%03u   ", r, min(r + 3, cpu->register_count - 1));
        }
        fprintf(file, "  "WHEX, cpu->registers[r]);
        if (r % 4 != 3) {
            fprintf(file, "    ");
        }
        if (r % 4 == 3) {
            fprintf(file, "\n");
        }
    }
    if (cpu->register_count % 4 != 0) {
        fprintf(file, "\n");
    }
    fprintf(file, "                  pc            sp            cs\n");
    fprintf(file, "                  "WHEX"      "WHEX"      "WHEX"\n", cpu->pc, cpu->sp, cpu->cs);
    fprintf(file, "                  z=%i  n=%i\n", cpu->zero, cpu->negative);
    fprintf(file, "  running:%s       print_op:%s    debug:%s       panic:%i\n",
            cpu->running == (uint32_t) 1 ? "y" : "n",
            cpu->print_op == (uint32_t) 1 ? "y" : "n",
            cpu->debug == (uint32_t) 1 ? "y" : "n",
            cpu->panic
    );

    if (cpu->panic) {
        fprintf(file, "\npanic:%i: ", cpu->panic);
        switch (cpu->panic) {
            case CPU_ERR_PANIC:
                fprintf(file, "PANIC\n");
                break;
            case CPU_ERR_CANNOT_READ_MEMORY:
                fprintf(file, "CANNOT_READ_MEMORY\n");
                break;
            case CPU_ERR_CANNOT_WRITE_MEMORY:
                fprintf(file, "CANNOT_WRITE_MEMORY\n");
                break;
            case CPU_ERR_UNKNOWN_OPCODE:
                fprintf(file, "UNKNOWN_OPCODE\n");
                break;
            case CPU_ERR_INVALID_REGISTER:
                fprintf(file, "INVALID_REGISTER\n");
                break;
            default:
                abort();
        }
    }
}