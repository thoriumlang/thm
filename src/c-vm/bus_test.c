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
#include "bus.h"

static void memory_attach_success(void **state) {
    Bus *bus = bus_create();
    Memory *mem = memory_create(WORD_SIZE, MEM_MODE_RW);
    assert_int_equal(BUS_ERR_OK, bus_memory_attach(bus, mem, 0, "First"));
}

static void memory_attach_failure(void **state) {
    Bus *bus = bus_create();
    Memory *mem = memory_create(WORD_SIZE, MEM_MODE_RW);
    bus_memory_attach(bus, mem, 0, "First");
    assert_int_equal(BUS_ERR_ZONE_OUT_OF_ORDER, bus_memory_attach(bus, mem, 0, "Second"));
}

static void memory_attach_memory_success(void **state) {
    Bus *bus = bus_create();
    assert_int_equal(BUS_ERR_OK, bus_memory_attach(bus, memory_create(WORD_SIZE, MEM_MODE_RW), 0, "First"));
    assert_int_equal(BUS_ERR_OK, bus_memory_attach(bus, memory_create(WORD_SIZE, MEM_MODE_RW), WORD_SIZE, "Second"));
}

static void word_read_invalid_address(void **state) {
    Bus *bus = bus_create();
    bus_memory_attach(bus, memory_create(WORD_SIZE, MEM_MODE_RW), 0, "First");

    word_sz word;
    assert_int_equal(BUS_ERR_INVALID_ADDRESS, bus_word_read(bus, WORD_SIZE, &word));
}

static void word_read_simple_success(void **state) {
    Memory *mem = memory_create(WORD_SIZE, MEM_MODE_RW);
    memory_word_set(mem, 0, 42);

    Bus *bus = bus_create();
    bus_memory_attach(bus, mem, 0, "First");

    word_sz word;
    assert_int_equal(BUS_ERR_OK, bus_word_read(bus, 0, &word));
    assert_int_equal(42, word);
}

static void word_read_second_zone_success(void **state) {
    Bus *bus = bus_create();
    bus_memory_attach(bus, memory_create(WORD_SIZE, MEM_MODE_RW), 0, "First");

    Memory *mem = memory_create(WORD_SIZE, MEM_MODE_RW);
    memory_word_set(mem, 0, 42);

    bus_memory_attach(bus, mem, sizeof(word_sz), "Second");

    word_sz word;
    assert_int_equal(BUS_ERR_OK, bus_word_read(bus, WORD_SIZE, &word));
    assert_int_equal(42, word);
}

static void word_write_invalid_address(void **state) {
    Bus *bus = bus_create();
    bus_memory_attach(bus, memory_create(WORD_SIZE, MEM_MODE_RW), 0, "First");

    assert_int_equal(BUS_ERR_INVALID_ADDRESS, bus_word_write(bus, WORD_SIZE, 42));
}

static void word_write_simple_success(void **state) {
    Bus *bus = bus_create();
    bus_memory_attach(bus, memory_create(WORD_SIZE, MEM_MODE_RW), 0, "First");

    assert_int_equal(BUS_ERR_OK, bus_word_write(bus, 0, 42));

    word_sz word;
    assert_int_equal(BUS_ERR_OK, bus_word_read(bus, 0, &word));
    assert_int_equal(42, word);
}

static void word_write_second_zone_success(void **state) {
    Bus *bus = bus_create();
    bus_memory_attach(bus, memory_create(WORD_SIZE, MEM_MODE_RW), 0, "First");
    bus_memory_attach(bus, memory_create(WORD_SIZE, MEM_MODE_RW), WORD_SIZE, "Second");

    assert_int_equal(BUS_ERR_OK, bus_word_write(bus, WORD_SIZE, 42));

    word_sz word;
    assert_int_equal(BUS_ERR_OK, bus_word_read(bus, WORD_SIZE, &word));
    assert_int_equal(42, word);
}

int main() {
    const struct CMUnitTest tests[] =
            {
                    cmocka_unit_test(memory_attach_success),
                    cmocka_unit_test(memory_attach_failure),
                    cmocka_unit_test(memory_attach_memory_success),
                    cmocka_unit_test(word_read_invalid_address),
                    cmocka_unit_test(word_read_simple_success),
                    cmocka_unit_test(word_read_second_zone_success),
                    cmocka_unit_test(word_write_invalid_address),
                    cmocka_unit_test(word_write_simple_success),
                    cmocka_unit_test(word_write_second_zone_success),
            };
    cmocka_set_message_output(CM_OUTPUT_STDOUT);
    return cmocka_run_group_tests(tests, NULL, NULL);
}