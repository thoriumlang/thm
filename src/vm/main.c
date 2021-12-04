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
#include "pic.h"

typedef struct VmThreadParam {
    Options *options;
    CPU *cpu;
    Bus *bus;
    Video *video;
} VmThreadParam;

void *cpu_loop(void *ptr);

void print_json(CPU *cpu, Bus *bus);

int load_file(Bus *bus, char *file, addr_t from);

void bus_mount(Bus *bus, Memory *memory, addr_t from, char *name) {
    if (bus_memory_attach(bus, memory, from, name) != BUS_ERR_OK) {
        fprintf(stderr, "Cannot mount %s in bus\n", name);
        exit(1);
    };
}

int main(int argc, char **argv) {
    Options *options = opts_parse(argc, argv);
    if (options->help_flag) {
        opts_print_help(argv[0]);
        return 0;
    }
    if (options->gen_header) {
        arch_print_header();
    }
    if (options->print_arch) {
        arch_print();
    }

    Bus *bus = bus_create();

    Memory *ram = memory_create(options->ram_size, MEM_MODE_RW);
    bus_mount(bus, ram, 0, "RAM");

    PIC *pic = pic_create();
    bus_mount(bus, pic_memory_get(pic)->interrupt_handlers, INTERRUPT_DESCRIPTOR_TABLE_ADDRESS, "IDT");
    bus_mount(bus, pic_memory_get(pic)->interrupt_mask, INTERRUPT_MASK_ADDRESS, "IMask");

    Video *video = video_create(options->video != OPT_VIDEO_MODE_NONE);
    VideoMemory *memory = video_memory_get(video);
    bus_mount(bus, memory->metadata, VIDEO_META_ADDRESS, "VMeta");
    if (memory->buffer[0]) {
        bus_mount(bus, memory->buffer[0], VIDEO_BUFFER_1_ADDRESS, "VBuf1");
    }
    if (memory->buffer[1]) {
        bus_mount(bus, memory->buffer[1], VIDEO_BUFFER_2_ADDRESS, "VBuf2");
    }

    Memory *rom = memory_create(ROM_SIZE, MEM_MODE_R);
    bus_mount(bus, rom, ROM_ADDRESS, "ROM");

    CPU *cpu = cpu_create(bus, pic, options->registers);

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

    cpu_print_op_enable(cpu, options->print_steps);
    // cpu_debug_enable(cpu, false);
    cpu_pc_set(cpu, options->pc);
    cpu_cs_set(cpu, options->pc); // FIXME
    cpu_idt_set(cpu, INTERRUPT_DESCRIPTOR_TABLE_ADDRESS);
    for (int i = 0; i < options->registers; i++) {
        cpu_register_set(cpu, i, (word_t) options->register_values[i]);
    }

    if (options->print_dump) {
        cpu_state_print(cpu, stdout);
        bus_state_print(bus, stdout);
        bus_dump(bus, 0, 16, stdout);
        bus_dump(bus, STACK_SIZE, 128, stdout);
        bus_dump(bus, ROM_ADDRESS, 128, stdout);
        bus_dump(bus, VIDEO_META_ADDRESS, VIDEO_META_SIZE, stdout);
        bus_dump(bus, INTERRUPT_MASK_ADDRESS, INTERRUPTS_WORDS_COUNT * WORD_SIZE, stdout);
        video_state_print(video, stdout);
    }

    VmThreadParam p = {
            .options = options,
            .cpu = cpu,
            .bus = bus,
            .video = video
    };
    pthread_t cpu_thread;
    pthread_create(&cpu_thread, NULL, cpu_loop, &p);

    video_loop(video);
    if (options->video == OPT_VIDEO_MODE_MASTER) {
        cpu_stop(cpu);
    }
    pthread_join(cpu_thread, NULL);

    opts_free(options);
    cpu_destroy(cpu);
    bus_destroy(bus);
    memory_destroy(ram);
    memory_destroy(rom);
    video_destroy(video);
    pic_destroy(pic);
}

void *cpu_loop(void *ptr) {
    Options *options = ((VmThreadParam *) ptr)->options;
    CPU *cpu = ((VmThreadParam *) ptr)->cpu;
    Bus *bus = ((VmThreadParam *) ptr)->bus;
    Video *video = ((VmThreadParam *) ptr)->video;

    cpu_start(cpu);

    if (options->print_dump) {
        cpu_state_print(cpu, stdout);
        bus_dump(bus, VIDEO_META_ADDRESS, VIDEO_META_SIZE, stdout);
        video_state_print(video, stdout);
    }

    if (options->print_json) {
        print_json(cpu, bus);
    }

    if (options->video == OPT_VIDEO_MODE_SLAVE) {
        video_stop(video);
    }

    return NULL;
}

void print_json(CPU *cpu, Bus *bus) {
    JsonElement *root = json_object();
    json_object_put(root, "arch", arch_json_get());
    json_object_put(root, "cpu", cpu_state_to_json(cpu));
    json_object_put(root, "bus", bus_state_to_json(bus));

    char *json = json_serialize(root);
    fprintf(stdout, "%s\n", json);
    free(json);
}

int load_file(Bus *bus, char *file, addr_t from) {
    const uint8_t NOP[] = {0x01, 0x00, 0x00, 0x00};
    if (file == NULL) {
        switch (bus_word_write(bus, from, vtoh(*NOP))) {
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
        switch (bus_word_write(bus, address, vtoh(word))) {
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