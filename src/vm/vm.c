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
#include "vm.h"
#include "memory.h"
#include "bus.h"
#include "pit.h"
#include "video.h"
#include "cpu.h"

typedef struct VM {
    vm_config *config;
    Memory *ram;
    Memory *rom;
    Bus *bus;
    PIC *pic;
    PIT *pit;
    Keyboard *keyboard;
    Video *video;
    CPU *cpu;
} VM;

void *cpu_thread_run(void *ptr);

VM *vm_create(vm_config *config) {
    VM *vm = malloc(sizeof(VM));

    vm->config = config;
    vm->bus = bus_create();
    vm->ram = memory_create(config->ram_size, MEM_MODE_RW);
    vm->rom = memory_create(ROM_SIZE, MEM_MODE_R);
    vm->pic = pic_create(vm->bus);
    vm->pit = pit_create(vm->pic, 1000 * 1000, INT_TIMER);
    vm->keyboard = keyboard_create(vm->bus, vm->pic);
    vm->video = video_create(vm->bus, vm->pic, vm->keyboard, config->video != VM_CONFIG_VIDEO_NONE);
    vm->cpu = cpu_create(vm->bus, vm->pic, config->register_count);

    bus_memory_attach(vm->bus, vm->ram, 0, "RAM");
    bus_memory_attach(vm->bus, vm->rom, ROM_ADDRESS, "ROM");

    return vm;
}

void vm_destroy(VM *this) {
    memory_destroy(this->ram);
    memory_destroy(this->rom);
    bus_destroy(this->bus);
    pic_destroy(this->pic);
    pit_destroy(this->pit);
    keyboard_destroy(this->keyboard);
    video_destroy(this->video);
    cpu_destroy(this->cpu);
    free(this);
}

void vm_start(VM *this) {
    pthread_t cpu_thread;
    pthread_create(&cpu_thread, NULL, cpu_thread_run, this);

    keyboard_start(this->keyboard);
    pit_start(this->pit);
    video_loop(this->video);

    if (this->config->video == VM_CONFIG_VIDEO_MASTER) {
        cpu_stop(this->cpu);
    }

    pthread_join(cpu_thread, NULL);
}

void vm_attach_cpu_debugger(VM *this, CpuDebugger *debugger) {
    cpu_debugger_set(this->cpu, debugger);
}

void *cpu_thread_run(void *ptr) {
    VM *vm = ((VM *) ptr);

    cpu_loop(vm->cpu);

    if (vm->config->video == VM_CONFIG_VIDEO_SLAVE) {
        video_stop(vm->video);
    }

    return NULL;
}

JsonElement *vm_json_get(VM *this) {
    JsonElement *root = json_object();
    json_object_put(root, "arch", vmarch_json_get());
    json_object_put(root, "cpu", cpu_json_get(this->cpu));
    json_object_put(root, "bus", bus_json_get(this->bus));
    return root;
}

void vm_state_print(VM *this, FILE *file) {
    cpu_state_print(this->cpu, file);
    bus_state_print(this->bus, file);
    bus_dump(this->bus, STACK_SIZE, 128, file);
    bus_dump(this->bus, ROM_ADDRESS, 128, file);
    video_state_print(this->video, file);
}

Bus *vm_bus_get(VM *this) {
    return this->bus;
}

CPU *vm_cpu_get(VM *this) {
    return this->cpu;
}

Memory *vm_rom_get(VM *this) {
    return this->rom;
}