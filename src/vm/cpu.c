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

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"
#include "cpu_internal.h"
#include "bus.h"
#include "ops.h"
#include "pic.h"

op_ptr cpu_decode(CPU *cpu, word_t word);

CPU *cpu_create(Bus *bus, PIC *pic, uint8_t reg_count) {
    // printf("sizeof(CPU)=%lu", sizeof(CPU));
    CPU *cpu = malloc(sizeof(CPU));
    cpu->bus = bus;
    cpu->pic = pic;
    cpu->registers = malloc(sizeof(*cpu->registers) * reg_count);
    cpu->register_count = reg_count;

    cpu_reset(cpu);

    return cpu;
}

void cpu_start(CPU *cpu) {
    cpu->state.running = 1;
    cpu->state.panic = CPU_ERR_OK;
    if (cpu->debug.print_op) {
        printf("\nCPU Steps\n");
    }
    while (cpu->state.running == 1 && cpu->state.panic == CPU_ERR_OK) {
        word_t word;
        if (cpu->flags.interrupts_enabled && pic_interrupt_active(cpu->pic)) {
            cpu->flags.interrupts_enabled = 0;
            cpu->ir = pic_interrupt_get(cpu->pic);
            pic_interrupt_reset(cpu->pic, cpu->ir);

            cpu->sp -= WORD_SIZE;
            if (bus_word_write(cpu->bus, cpu->sp, cpu->pc) != BUS_ERR_OK) {
                cpu->state.panic = CPU_ERR_CANNOT_WRITE_MEMORY;
            } else {
                cpu->pc = ROM_ADDRESS + 8;
                if (cpu->debug.print_op) {
                    printf("// handling interrupt %i\n", cpu->ir);
                }
            }
        }
        word = htov(cpu_fetch(cpu));

        if (!cpu->state.panic) {
            op_ptr op = cpu_decode(cpu, word);
            if (!cpu->state.panic) {
                if (op) {
                    op(cpu, word);
                    cpu->debug.step++;
                } else {
                    printf("Not implemented: 0x%02x\n", ((uint8_t *) &word)[0]);
                    cpu->state.panic = CPU_ERR_UNIMPLEMENTED_OPCODE;
                }
            }
        }
    }
    cpu->state.running = 0;
}


word_t cpu_fetch(CPU *cpu) {
    if (cpu->state.panic != CPU_ERR_OK){
        return 0;
    }
    word_t word;
    if (bus_word_read(cpu->bus, cpu->pc, &word) != BUS_ERR_OK) {
        cpu->state.panic = CPU_ERR_CANNOT_READ_MEMORY;
        return 0;
    }
    cpu->pc += ADDR_SIZE;
    return word;
}

op_ptr cpu_decode(CPU *cpu, word_t word) {
    if (cpu->state.panic) {
        return NULL;
    }
    return ops[((uint8_t *) &word)[0]];
}

void cpu_stop(CPU *cpu) {
    cpu->state.running = 0;
}

void cpu_reset(CPU *cpu) {
    for (int r = 0; r < 32; r++) {
        cpu->registers[r] = 0;
    }
    cpu->pc = STACK_SIZE;
    cpu->sp = STACK_SIZE;
    cpu->cs = STACK_SIZE;
    cpu->flags.interrupts_enabled = 0;
    cpu->flags.zero = 0;
    cpu->flags.negative = 0;
    cpu->state.running = 0;
    cpu->state.panic = 0;
    cpu->debug.print_op = 0;
    cpu->debug.step = 0;
}

void cpu_destroy(CPU *cpu) {
    free(cpu->registers);
    free(cpu);
}

CpuError cpu_register_get(CPU *cpu, uint8_t reg, word_t *word) {
    if (reg >= cpu->register_count) {
        return CPU_ERR_INVALID_REGISTER;
    }
    *word = cpu->registers[reg];
    return CPU_ERR_OK;
}

CpuError cpu_register_set(CPU *cpu, uint8_t reg, word_t value) {
    if (reg >= cpu->register_count) {
        return CPU_ERR_INVALID_REGISTER;
    }
    cpu->registers[reg] = value;
    cpu_flags_update(cpu, (sword_t) value);
    return CPU_ERR_OK;
}

void cpu_pc_set(CPU *cpu, addr_t address) {
    cpu->pc = address;
}

addr_t cpu_pc_get(CPU *cpu) {
    return cpu->pc;
}

void cpu_cs_set(CPU *cpu, addr_t address) {
    cpu->cs = address;
}

addr_t cpu_cs_get(CPU *cpu) {
    return cpu->cs;
}

addr_t cpu_sp_get(CPU *cpu) {
    return cpu->sp;
}

void cpu_idt_set(CPU *cpu, addr_t addr) {
    cpu->idt = addr;
}

word_t cpu_idt_get(CPU * cpu) {
    return cpu->idt;
}

word_t cpu_ir_get(CPU * cpu) {
    return cpu->ir;
}

void cpu_interrupt_trigger(CPU *cpu, uint8_t interrupt) {
    pic_interrupt_trigger(cpu->pic, interrupt);
}

void cpu_flags_update(CPU *cpu, sword_t value) {
    cpu->flags.zero = (value == 0) ? 1 : 0;
    cpu->flags.negative = ((sword_t) value < 0) ? 1 : 0;
}

void cpu_print_op_enable(CPU *cpu, bool enable) {
    cpu->debug.print_op = enable;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

void cpu_state_print(CPU *cpu, FILE *file) {
    fprintf(file, "\nCPU state\n");
    for (int r = 0; r < cpu->register_count; r++) {
        if (r % 4 == 0) {
            fprintf(file, "  r%03u - r%03u   ", r, min(r + 3, cpu->register_count - 1));
        }
        fprintf(file, "  "WHEX, cpu->registers[r]);
        if (r % 4 != 3) {
            fprintf(file, "    ");
        }
        if (r % 4 == 3) {
            fprintf(file, "\n");
        }
    }
    if (cpu->register_count % 4 != 0) {
        fprintf(file, "\n");
    }
    fprintf(file, "                  pc            sp            cs\n");
    fprintf(file, "                  "WHEX"      "WHEX"      "WHEX"\n", cpu->pc, cpu->sp, cpu->cs);
    fprintf(file, "                  ir            idt\n");
    fprintf(file, "                  "WHEX"      "WHEX"\n", cpu->ir, cpu->idt);
    fprintf(file, "                  z=%i  n=%i  i=%i\n", cpu->flags.zero, cpu->flags.negative,
            cpu->flags.interrupts_enabled);
    fprintf(file, "  running:%s       print_op:%s    panic:%i\n",
            cpu->state.running == (uint32_t) 1 ? "y" : "n",
            cpu->debug.print_op == (uint32_t) 1 ? "y" : "n",
            cpu->state.panic
    );

    if (cpu->state.panic) {
        fprintf(file, "\npanic:%i: ", cpu->state.panic);
        switch (cpu->state.panic) {
            case CPU_ERR_PANIC:
                fprintf(file, "PANIC\n");
                break;
            case CPU_ERR_CANNOT_READ_MEMORY:
                fprintf(file, "CANNOT_READ_MEMORY\n");
                break;
            case CPU_ERR_CANNOT_WRITE_MEMORY:
                fprintf(file, "CANNOT_WRITE_MEMORY\n");
                break;
            case CPU_ERR_INVALID_REGISTER:
                fprintf(file, "INVALID_REGISTER\n");
                break;
            default:
                abort();
        }
    }
}

JsonElement *cpu_state_to_json(CPU *cpu) {
    JsonElement *registers = json_object();
    json_object_put(registers, "pc", json_number(cpu_pc_get(cpu)));
    json_object_put(registers, "sp", json_number(cpu_sp_get(cpu)));
    json_object_put(registers, "cs", json_number(cpu_cs_get(cpu)));

    word_t word;
    JsonElement *general_registers = json_array();
    for (int r = 0; r < cpu->register_count; r++) {
        cpu_register_get(cpu, r, &word);
        json_array_append(general_registers, json_number((double) word));
    }
    json_object_put(registers, "general", general_registers);

    JsonElement *flags = json_object();
    json_object_put(flags, "zero", json_bool(cpu->flags.zero));
    json_object_put(flags, "negative", json_bool(cpu->flags.negative));
    json_object_put(flags, "interrupt_enabled", json_bool(cpu->flags.interrupts_enabled));

    JsonElement *state = json_object();
    json_object_put(state, "panic", json_number(cpu->state.panic));
    json_object_put(state, "running", json_bool(cpu->state.running));

    JsonElement *root = json_object();
    json_object_put(root, "registers", registers);
    json_object_put(root, "flags", flags);
    json_object_put(root, "state", state);

    return root;
}