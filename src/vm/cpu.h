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

#ifndef C_VM_CPU_H
#define C_VM_CPU_H

#include <stdint.h>
#include <stdbool.h>
#include "vmarch.h"
#include "bus.h"
#include "pic.h"

typedef enum {
    CPU_ERR_OK,
    CPU_ERR_PANIC,
    CPU_ERR_CANNOT_READ_MEMORY,
    CPU_ERR_CANNOT_WRITE_MEMORY,
    CPU_ERR_UNIMPLEMENTED_OPCODE,
    CPU_ERR_INVALID_REGISTER
} CpuError;

typedef struct CPU CPU;

CPU *cpu_create(Bus *bus, PIC *pic, uint8_t reg_count);

void cpu_start(CPU *cpu);

void cpu_stop(CPU *cpu);

void cpu_reset(CPU *cpu);

void cpu_destroy(CPU *cpu);

CpuError cpu_register_get(CPU *cpu, uint8_t reg, word_t *word);

CpuError cpu_register_set(CPU *cpu, uint8_t reg, word_t value);

void cpu_pc_set(CPU *cpu, addr_t address);

addr_t cpu_pc_get(CPU *cpu);

void cpu_cs_set(CPU *cpu, addr_t address);

addr_t cpu_cs_get(CPU *cpu);

addr_t cpu_sp_get(CPU *cpu);

void cpu_print_op_enable(CPU *cpu, bool enable);

void cpu_debug_enable(CPU *cpu, bool enable);

void cpu_state_print(CPU *cpu, FILE *file);

JsonElement *cpu_state_to_json(CPU *cpu);

#endif // C_VM_CPU_H