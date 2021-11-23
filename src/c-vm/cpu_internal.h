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
    unsigned long step;
    word_t *registers;
    addr_t pc;
    addr_t sp;
    addr_t cs;
    CpuError panic;
    uint8_t register_count;
    uint8_t zero: 1;
    uint8_t negative: 1;
    uint8_t running: 1;
    uint8_t debug: 1;
    uint8_t print_op: 1;
} CPU;

word_t fetch(CPU *cpu);

void update_flags(CPU *cpu, sword_t value);

#endif //C_VM_CPU_INTERNAL_H
