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

#ifndef C_VM_CPU_INTERNAL_H
#define C_VM_CPU_INTERNAL_H

typedef struct CPU {
    Bus *bus;
    PIC *pic;
    word_t *registers;
    uint8_t register_count;
    addr_t pc;  // program counter
    addr_t sp;  // stack pointer
    addr_t cs;  // code segment
    addr_t idt; // interrupt descriptor table
    word_t ir;  // current interrupt
    struct {
        uint8_t interrupts_enabled: 1;
        uint8_t zero: 1;
        uint8_t negative: 1;
    } flags;
    struct {
        uint8_t running: 1;
        CpuError panic;
    } state;
    struct {
        uint8_t print_op: 1;
        unsigned long step;
    } debug;
} CPU;

// todo this is shared with tha and should be derived from come common file
#define REG_PC    255
#define REG_SP    254
#define REG_CS    253
#define REG_IR    252
#define REG_IDT   251

word_t cpu_fetch(CPU *cpu);

void cpu_flags_update(CPU *cpu, sword_t value);

#endif //C_VM_CPU_INTERNAL_H
