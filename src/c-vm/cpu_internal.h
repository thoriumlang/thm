/**
 * Copyright 2019 Christophe Pollet
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
    uint64_t step;
    uint8_t register_count;
    word_sz *registers;
    addr_sz pc;
    addr_sz sp;
    addr_sz cs;
    CpuError panic;
    uint8_t running: 1;
    uint8_t debug: 1;
    uint8_t print_op: 1;
} CPU;

word_sz fetch(CPU *cpu);

#endif //C_VM_CPU_INTERNAL_H
