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
#include "vm.h"
#include "debugger.h"

void *cpu_thread_run(void *ptr);

int load_file(Bus *bus, char *file, addr_t from);

enum vm_config_video decode_video_mode(OptsVideMode video);

int main(int argc, char **argv) {
    Options *options = opts_parse(argc, argv);
    if (options->help_flag) {
        opts_print_help(argv[0]);
        return 0;
    }
    if (options->gen_header) {
        vmarch_header_print();
        return 0;
    }
    if (options->print_arch) {
        vmarch_print();
    }

    VM *vm = vm_create((vm_config *) &(vm_config) {
            .ram_size = options->ram_size,
            .register_count = options->registers,
            .video = decode_video_mode(options->video),
    });

    CpuDebugger *debugger = cpu_debugger_create();
    vm_attach_cpu_debugger(vm, debugger);

    if (!load_file(vm_bus_get(vm), options->image, STACK_SIZE)) {
        return 1;
    }
    if (options->rom != NULL) {
        memory_mode_set(vm_rom_get(vm), MEM_MODE_RW);
        if (!load_file(vm_bus_get(vm), options->rom, ROM_ADDRESS)) {
            return 1;
        }
        memory_mode_set(vm_rom_get(vm), MEM_MODE_R);
    }

    cpu_print_op_enable(vm_cpu_get(vm), options->print_steps);
    cpu_pc_set(vm_cpu_get(vm), options->pc);
    cpu_cs_set(vm_cpu_get(vm), options->pc); // FIXME
    cpu_idt_set(vm_cpu_get(vm), INTERRUPT_DESCRIPTOR_TABLE_ADDRESS);
    for (int i = 0; i < options->registers; i++) {
        cpu_register_set(vm_cpu_get(vm), i, (word_t) options->register_values[i]);
    }

    if (options->print_state) {
        vm_state_print(vm, stdout);
    }

    vm_start(vm);

    if (options->print_state) {
        vm_state_print(vm, stdout);
    }

    if (options->print_json) {
        char *json = json_serialize(vm_json_get(vm));
        printf("%s\n", json);
        free(json);
    }

    cpu_debugger_destroy(debugger);
    vm_destroy(vm);
}

enum vm_config_video decode_video_mode(OptsVideMode video) {
    switch (video) {
        case OPT_VIDEO_MODE_NONE:
            return VM_CONFIG_VIDEO_NONE;
        case OPT_VIDEO_MODE_MASTER:
            return VM_CONFIG_VIDEO_MASTER;
        case OPT_VIDEO_MODE_SLAVE:
            return VM_CONFIG_VIDEO_SLAVE;
        default:
            fprintf(stderr, "unsupported video mode: %i", video);
            exit(1);
    }
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