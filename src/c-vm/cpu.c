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
#include "bus.h"

typedef struct CPU {
    Bus *bus;
    uint8_t register_count;
    word_sz *registers;
    addr_sz pc;
    addr_sz sp;
    addr_sz cs;
    uint32_t running: 1;
    uint32_t step: 1;
    uint32_t print_op: 1;
} CPU;

CPU *cpu_create(Bus *bus, uint8_t reg_count) {
    CPU *cpu = malloc(sizeof(CPU));
    cpu->bus = bus;
    cpu->register_count = reg_count;
    cpu->registers = malloc(sizeof(*cpu->registers) * reg_count);

    cpu_reset(cpu);

    return cpu;
}

void cpu_start(CPU *cpu) {
    cpu->running = 1;
    while (cpu->running) {
        fprintf(stderr, "better not to run ;)\n");
        cpu->running = 0;
    }
}

void cpu_stop(CPU *cpu) {
    cpu->running = 0;
}

void cpu_reset(CPU *cpu) {
    for (int r = 0; r < 32; r++) {
        cpu->registers[r] = 0;
    }
    cpu->pc = cpu->sp = cpu->cs = 0;
    cpu->print_op = 0;
}

void cpu_destroy(CPU *cpu) {
    free(cpu->registers);
    free(cpu);
}

CpuError cpu_register_get(CPU *cpu, uint8_t reg, word_sz *word) {
    if (reg >= cpu->register_count) {
        return CPU_ERR_INVALID_REGISTER;
    }
    *word = cpu->registers[reg];
    return CPU_ERR_OK;
}

CpuError cpu_register_set(CPU *cpu, uint8_t reg, word_sz value) {
    if (reg >= cpu->register_count) {
        return CPU_ERR_INVALID_REGISTER;
    }
    cpu->registers[reg] = value;
    return CPU_ERR_OK;
}

void cpu_print_op_enable(CPU *cpu) {
    cpu->print_op = 1;
}

void cpu_print_op_disable(CPU *cpu) {
    cpu->print_op = 0;
}

void cpu_step_enable(CPU *cpu) {
    cpu->step = 1;
}

void cpu_step_disable(CPU *cpu) {
    cpu->step = 0;
}

void cpu_print_state(FILE *file, CPU *cpu) {
    fprintf(file, "\nCPU state\n");
    for (int r = 0; r < cpu->register_count; r++) {
        if (r % 4 == 0) {
            fprintf(file, "  r%02u - r%02u   ", r, r + 3);
        }
        fprintf(file, "  "WHEX, cpu->registers[r]);
        if (r % 4 != 3) {
            fprintf(file, "    ");
        }
        if (r % 4 == 3) {
            fprintf(file, "\n");
        }
    }
    fprintf(file, "                pc            sp            cs\n");
    fprintf(file, "                "WHEX"      "WHEX"      "WHEX"\n", cpu->pc, cpu->sp, cpu->cs);
    fprintf(file, "  running:%s     print_op:%s    step:%s\n",
            cpu->running == (uint32_t) 1 ? "y" : "n",
            cpu->print_op == (uint32_t) 1 ? "y" : "n",
            cpu->step == (uint32_t) 1 ? "y" : "n"
    );
}