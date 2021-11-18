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

#include "vmarch.h"
#include "bus.h"
#include "cpu.h"
#include "memory.h"

int main(int args, char **argv) {
    Bus *bus = bus_create();
    Memory *ram = memory_create(4096, MEM_MODE_RW);
    Memory *rom = memory_create(ROM_SIZE, MEM_MODE_R);

    bus_memory_attach(bus, ram, 0, "RAM");
    bus_memory_attach(bus, rom, ROM_ADDRESS, "ROM");

    CPU *cpu = cpu_create(bus, 32);

    for (int i = 0; i < 32; i++) {
        cpu_register_set(cpu, i, i);
    }
    cpu_print_op_enable(cpu);
    cpu_step_enable(cpu);
    cpu_print_state(stdout, cpu);
    bus_print_state(stdout, bus);

    cpu_start(cpu);

    cpu_destroy(cpu);
    bus_destroy(bus);
    memory_destroy(ram);
}
