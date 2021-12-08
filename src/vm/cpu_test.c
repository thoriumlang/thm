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

#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <cmocka.h>

#include "cpu.h"
#include "ops.h"

static void create_initializes_cpu(void **state) {
    CPU *cpu = cpu_create(NULL, NULL, 2);

    for (uint8_t i = 0; i < 2; i++) {
        word_t word = 1;
        assert_int_equal(CPU_ERR_OK, cpu_register_get(cpu, 0, &word));
        assert_int_equal(0, word);
    }
}

static void register_get_invalid(void **state) {
    CPU *cpu = cpu_create(NULL, NULL, 0);

    word_t word = 1;
    assert_int_equal(CPU_ERR_INVALID_REGISTER, cpu_register_get(cpu, 0, &word));
    assert_int_equal(1, word);
}

static void register_set_invalid(void **state) {
    CPU *cpu = cpu_create(NULL, NULL, 0);

    assert_int_equal(CPU_ERR_INVALID_REGISTER, cpu_register_set(cpu, 0, 1));
}

static void register_set(void **state) {
    CPU *cpu = cpu_create(NULL, NULL, 2);

    assert_int_equal(CPU_ERR_OK, cpu_register_set(cpu, 1, 1));
    for (uint8_t i = 0; i < 2; i++) {
        word_t word = 2;
        assert_int_equal(CPU_ERR_OK, cpu_register_get(cpu, i, &word));
        assert_int_equal(i, word);
    }
}

int main() {
    const struct CMUnitTest tests[] =
            {
                    cmocka_unit_test(create_initializes_cpu),
                    cmocka_unit_test(register_get_invalid),
                    cmocka_unit_test(register_set_invalid),
                    cmocka_unit_test(register_set),
            };
    cmocka_set_message_output(CM_OUTPUT_STDOUT);
    return cmocka_run_group_tests(tests, NULL, NULL);
}

// dummies

op_ptr ops[256] = { 0 };

BusError bus_word_read(Bus *bus, addr_t address, word_t *word) { return BUS_ERR_OK; }

BusError bus_word_write(Bus *bus, addr_t address, word_t word) { return BUS_ERR_OK; }

word_t vtoh(word_t word) { return word; }

word_t htov(word_t word) { return word; }

JsonElement *json_array() { return NULL; }

void json_array_append(JsonElement *array, JsonElement *element) {}

JsonElement *json_string(char *value) { return NULL; }

JsonElement *json_number(double value) { return NULL; }

JsonElement *json_bool(bool value) { return NULL; }

JsonElement *json_object() { return NULL; }

void json_object_put(JsonElement *object, char *key, JsonElement *element) {}

bool pic_interrupt_active(PIC *this) { return false; }

interrupt_t pic_interrupt_get(PIC *this) { return 0; }

void pic_interrupt_trigger(PIC *this, interrupt_t interrupt) {}

void pic_interrupt_reset(PIC *this, interrupt_t interrupt) {}