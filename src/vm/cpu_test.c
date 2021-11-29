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

static void create_initializes_cpu(void **state) {
    CPU *cpu = cpu_create(NULL, 2);

    for (uint8_t i = 0; i < 2; i++) {
        word_t word = 1;
        assert_int_equal(CPU_ERR_OK, cpu_register_get(cpu, 0, &word));
        assert_int_equal(0, word);
    }
}

static void register_get_invalid(void **state) {
    CPU *cpu = cpu_create(NULL, 0);

    word_t word = 1;
    assert_int_equal(CPU_ERR_INVALID_REGISTER, cpu_register_get(cpu, 0, &word));
    assert_int_equal(1, word);
}

static void register_set_invalid(void **state) {
    CPU *cpu = cpu_create(NULL, 0);

    assert_int_equal(CPU_ERR_INVALID_REGISTER, cpu_register_set(cpu, 0, 1));
}

static void register_set(void **state) {
    CPU *cpu = cpu_create(NULL, 2);

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
            };
    cmocka_set_message_output(CM_OUTPUT_STDOUT);
    return cmocka_run_group_tests(tests, NULL, NULL);
}