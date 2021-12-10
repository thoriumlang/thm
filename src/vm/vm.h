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

#ifndef THM_VM_H
#define THM_VM_H

#include "vmarch.h"
#include "bus.h"
#include "cpu.h"
#include "video.h"
#include "pit.h"
#include "debugger.h"

typedef struct VM VM;

enum vm_config_video {
    VM_CONFIG_VIDEO_NONE, VM_CONFIG_VIDEO_MASTER, VM_CONFIG_VIDEO_SLAVE
};

typedef struct vm_config {
    addr_t ram_size;
    uint8_t register_count;
    enum vm_config_video video;
} vm_config;

VM *vm_create(vm_config *config);

void vm_destroy(VM *this);

void vm_start(VM *this);

void vm_attach_cpu_debugger(VM *this, CpuDebugger *debugger);

JsonElement *vm_json_get(VM *this);

void vm_state_print(VM *this, FILE *file);

Bus *vm_bus_get(VM *this);

CPU *vm_cpu_get(VM *this);

Memory *vm_rom_get(VM *this);

#endif //THM_VM_H
