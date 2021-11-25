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
#include <pthread.h>
#include "opts.h"
#include "vmarch.h"
#include "bus.h"
#include "cpu.h"
#include "memory.h"
#include "json.h"
#include "video.h"

typedef struct VmThreadParam {
    Options *options;
    Video *video;
} VmThreadParam;

void *cpu_loop(void *ptr);

void print_arch() {
    printf("Architecture\n");
    printf("  addr_size:   %i\n", ADDR_SIZE);
    printf("  word_size:   %i\n", WORD_SIZE);
    printf("  stack_dept:  %i\n", STACK_LENGTH);
    printf("  stack_size:  %i\n", STACK_SIZE);
    printf("  stack_start: "AXHEX"\n", 0);
    printf("  stack_end:   "AXHEX"\n", STACK_SIZE - 1);
    printf("  rom_size:    %i\n", ROM_SIZE);
    printf("  rom_start:   "AXHEX"\n", ROM_ADDRESS);
    printf("  rom_end:     "AXHEX"\n", ROM_ADDRESS + ROM_SIZE - 1);
}

void print_json(CPU *cpu, Bus *bus) {
    char hex[32];
    JsonElement *arch = json_object();
    json_object_put(arch, "addr_size", json_number(ADDR_SIZE));
    json_object_put(arch, "word_size", json_number(WORD_SIZE));
    json_object_put(arch, "stack_depth", json_number(STACK_LENGTH));
    json_object_put(arch, "stack_size", json_number(STACK_SIZE));
    json_object_put(arch, "stack_start", json_number(0));
    sprintf(hex, AXHEX, 0);
    json_object_put(arch, "stack_start_hex", json_string(hex));
    json_object_put(arch, "stack_end", json_number(STACK_SIZE - 1));
    sprintf(hex, AXHEX, STACK_SIZE - 1);
    json_object_put(arch, "stack_end_hex", json_string(hex));
    json_object_put(arch, "code_start", json_number(STACK_SIZE));
    sprintf(hex, AXHEX, STACK_SIZE);
    json_object_put(arch, "code_start_hex", json_string(hex));
    json_object_put(arch, "rom_size", json_number(ROM_SIZE));
    json_object_put(arch, "rom_start", json_number(ROM_ADDRESS));
    sprintf(hex, AXHEX, ROM_ADDRESS);
    json_object_put(arch, "rom_start", json_string(hex));
    json_object_put(arch, "rom_end", json_number(ROM_ADDRESS + ROM_SIZE - 1));
    sprintf(hex, AXHEX, ROM_ADDRESS + ROM_SIZE - 1);
    json_object_put(arch, "rom_end", json_string(hex));

    JsonElement *root = json_object();
    json_object_put(root, "arch", arch);
    json_object_put(root, "cpu", cpu_state_to_json(cpu));
    json_object_put(root, "bus", bus_state_to_json(bus));

    char *json = json_serialize(root);
    fprintf(stdout, "%s\n", json);
    free(json);
}

int load_file(Bus *bus, char *file, addr_t from) {
    const uint8_t NOP[] = {0x01, 0x00, 0x00, 0x00};
    if (file == NULL) {
        switch (bus_word_write(bus, from, *NOP)) {
            case BUS_ERR_OK:
                break;
            case BUS_ERR_INVALID_ADDRESS:
                fprintf(stderr, "Invalid address: "AXHEX"\n", from);
                return 0;
            case BUS_ERR_ILLEGAL_ACCESS:
                fprintf(stderr, "Cannot write to "AXHEX"\nn", from);
                return 0;
            default:
                abort();
        }
        return 1;
    }
    FILE *fptr;

    if ((fptr = fopen(file, "rb")) == NULL) {
        fprintf(stderr, "Cannot open %s\n", file);
        return 0;
    }

    addr_t total_words_read = 0;
    addr_t words_read;
    word_t word;
    while ((words_read = fread(&word, WORD_SIZE, 1, fptr)) > 0) {
        addr_t address = from + total_words_read * WORD_SIZE;
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
    }

    Video *video = video_create(options->video);

    VmThreadParam p;
    p.options = options;
    p.video = video;

    pthread_t cpu_thread;
    pthread_create(&cpu_thread, NULL, cpu_loop, &p);

    video_loop(video);              // blocking
    pthread_join(cpu_thread, NULL); // blocking

    opts_free(options);
    video_destroy(video);
}

void *cpu_loop(void *ptr) {
    VmThreadParam *p = (VmThreadParam *) ptr;
    Options *options = p->options;

    Bus *bus = bus_create();
    Memory *ram = memory_create(options->ram_size, MEM_MODE_RW);
    Memory *rom = memory_create(ROM_SIZE, MEM_MODE_R);
    CPU *cpu = cpu_create(bus, options->registers);

    bus_memory_attach(bus, ram, 0, "RAM");
    bus_memory_attach(bus, rom, ROM_ADDRESS, "ROM");

    if (!load_file(bus, options->image, STACK_SIZE)) {
        return NULL; // fixme
    }
    if (options->rom != NULL) {
        memory_mode_set(rom, MEM_MODE_RW);
        if (!load_file(bus, options->rom, ROM_ADDRESS)) {
            return NULL; // fixme
        }
        memory_mode_set(rom, MEM_MODE_R);
    }

    cpu_print_op_enable(cpu, options->print_steps);
    // cpu_debug_enable(cpu, false);
    cpu_pc_set(cpu, options->pc);
    cpu_cs_set(cpu, options->pc); // FIXME
    for (int i = 0; i < options->registers; i++) {
        cpu_register_set(cpu, i, (word_t) options->register_values[i]);
    }

    if (options->print_dump) {
        cpu_state_print(cpu, stdout);
        bus_state_print(bus, stdout);
        bus_dump(bus, 0, 16, stdout);
        bus_dump(bus, STACK_SIZE, 128, stdout);
        bus_dump(bus, ROM_ADDRESS, 128, stdout);
    }

    cpu_start(cpu);

    if (p->options->print_dump) {
        cpu_state_print(cpu, stdout);
    }

    if (p->options->print_json) {
        print_json(cpu, bus);
    }

    video_pause(p->video);

    cpu_destroy(cpu);
    bus_destroy(bus);
    memory_destroy(ram);
    memory_destroy(rom);

    return NULL;
}