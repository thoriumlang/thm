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

#include <stdlib.h>
#include <assert.h>
#include "opts.h"
#include "vmarch.h"
#include "bus.h"
#include "cpu.h"
#include "memory.h"

void print_arch() {
    printf("addr_size:   %lu\n", sizeof(addr_sz));
    printf("word_size:   %i\n", WORD_SIZE);
    printf("stack_dept:  %i\n", STACK_LENGTH);
    printf("stack_size:  %i\n", STACK_SIZE);
    printf("stack_start: "AXHEX"\n", 0);
    printf("stack_end:   "AXHEX"\n", STACK_SIZE - 1);
    printf("rom_size:    %i\n", ROM_SIZE);
    printf("rom_start:   "AXHEX"\n", ROM_ADDRESS);
    printf("rom_end:     "AXHEX"\n", ROM_ADDRESS + ROM_SIZE - 1);
}

int load_file(Bus *bus, char *file, addr_sz from) {
    FILE *fptr;

    if ((fptr = fopen(file, "rb")) == NULL) {
        fprintf(stderr, "Cannot open %s\n", file);
        return 0;
    }

    addr_sz total_words_read = 0;
    addr_sz words_read;
    word_sz word;
    while ((words_read = fread(&word, WORD_SIZE, 1, fptr)) > 0) {
        addr_sz address = from + total_words_read * WORD_SIZE;
        switch (bus_word_write(bus, address, word)) {
            case BUS_ERR_OK:
                break;
            case BUS_ERR_INVALID_ADDRESS:
                fprintf(stderr, "Invalid address: "AXHEX"\n", address);
                return 0;
            case BUS_ERR_ILLEGAL_ACCESS:
                fprintf(stderr, "Cannot write to "AXHEX"\nn", address);
                return 0;
            default:
                abort();
        }
        total_words_read += words_read;
    }
    fclose(fptr);
    printf("Loaded %i bytes from %s to "AXHEX"\n", total_words_read * WORD_SIZE, file, from);

    return 1;
}

int main(int argc, char **argv) {
    Options *options = opts_parse(argc, argv);
    if (options->help_flag) {
        opts_print_help(argv[0]);
        return 0;
    }
    if (options->print_arch) {
        print_arch();
        return 0;
    }

    Bus *bus = bus_create();
    Memory *ram = memory_create(options->ram_size, MEM_MODE_RW);
    Memory *rom = memory_create(ROM_SIZE, MEM_MODE_R);

    bus_memory_attach(bus, ram, 0, "RAM");
    bus_memory_attach(bus, rom, ROM_ADDRESS, "ROM");

    if (!load_file(bus, options->image, STACK_SIZE)) {
        return 1;
    }
    if (options->rom != NULL) {
        memory_mode_set(rom, MEM_MODE_RW);
        if (!load_file(bus, options->rom, ROM_ADDRESS)) {
            return 1;
        }
        memory_mode_set(rom, MEM_MODE_R);
    }

    CPU *cpu = cpu_create(bus, options->registers);
    cpu_print_op_enable(cpu);
    cpu_debug_enable(cpu);
    cpu_set_pc(cpu, options->pc);
    for (int i = 0; i < options->registers; i++) {
        cpu_register_set(cpu, i, i);
    }

    assert(memory_mode_get(rom) == MEM_MODE_R);

    cpu_print_state(stdout, cpu);
    bus_print_state(stdout, bus);
    bus_dump(stdout, bus, 0, 16);
    bus_dump(stdout, bus, STACK_SIZE, 128);
    bus_dump(stdout, bus, ROM_ADDRESS, 128);

    cpu_start(cpu);

    cpu_print_state(stdout, cpu);

    opts_free(options);
    cpu_destroy(cpu);
    bus_destroy(bus);
    memory_destroy(ram);
}
