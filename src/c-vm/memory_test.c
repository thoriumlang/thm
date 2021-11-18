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

#include "memory.h"

static void create_rounds_up(void **state) {
    Memory *mem = memory_create(sizeof(word_sz) - 1, MEM_MODE_R);
    assert_int_equal(sizeof(word_sz), memory_size_get(mem));
}

static void create_initializes_to_0(void **state) {
    Memory *mem = memory_create(sizeof(word_sz), MEM_MODE_R);
    word_sz word = 1;
    memory_word_get(mem, 0, &word);
    assert_int_equal(0, word);
}

static void get_size(void **state) {
    Memory *mem = memory_create(4, MEM_MODE_R);
    assert_int_equal(4, memory_size_get(mem));
}

static void get_max_address(void **state) {
    Memory *mem = memory_create(4, MEM_MODE_R);
    assert_int_equal(3, memory_max_address_get(mem));
}

static void get_mode(void **state) {
    Memory *mem = memory_create(4, MEM_MODE_R);
    assert_int_equal(4, memory_size_get(mem));
    assert_int_equal(MEM_MODE_R, MEM_MODE_R);
}

static void get_word_success(void **state) {
    Memory *mem = memory_create(4, MEM_MODE_R);
    word_sz word = 1;
    assert_int_equal(MEM_ERR_OK, memory_word_get(mem, 0, &word));
    assert_int_equal(0, word);
}

static void get_word_not_aligned(void **state) {
    Memory *mem = memory_create(4, MEM_MODE_R);
    word_sz word = 1;
    assert_int_equal(MEM_ERR_NOT_ALIGNED, memory_word_get(mem, 1, &word));
    assert_int_equal(1, word);
    memory_destroy(mem);
}

static void get_word_out_of_bounds(void **state) {
    Memory *mem = memory_create(4, MEM_MODE_R);
    word_sz word = 1;
    assert_int_equal(MEM_ERR_OUT_OF_BOUND, memory_word_get(mem, 4, &word));
    assert_int_equal(1, word);
    memory_destroy(mem);
}

static void set_word_success(void **state) {
    Memory *mem = memory_create(4, MEM_MODE_RW);
    word_sz word = 0;

    assert_int_equal(MEM_ERR_OK, memory_word_set(mem, 0, 1));
    memory_word_get(mem, 0, &word);
    assert_int_equal(1, word);
}

static void set_word_not_writable(void **state) {
    Memory *mem = memory_create(4, MEM_MODE_R);
    assert_int_equal(MEM_ERR_NOT_WRITABLE, memory_word_set(mem, 0, 1));
    memory_destroy(mem);
}

static void set_word_not_aligned(void **state) {
    Memory *mem = memory_create(4, MEM_MODE_RW);
    assert_int_equal(MEM_ERR_NOT_ALIGNED, memory_word_set(mem, 1, 1));
    memory_destroy(mem);
}

static void set_word_out_of_bounds(void **state) {
    Memory *mem = memory_create(4, MEM_MODE_RW);
    assert_int_equal(MEM_ERR_OUT_OF_BOUND, memory_word_set(mem, 4, 1));
    memory_destroy(mem);
}

int main() {
    const struct CMUnitTest tests[] =
            {
                    cmocka_unit_test(create_rounds_up),
                    cmocka_unit_test(create_initializes_to_0),
                    cmocka_unit_test(get_size),
                    cmocka_unit_test(get_max_address),
                    cmocka_unit_test(get_mode),
                    cmocka_unit_test(get_word_success),
                    cmocka_unit_test(get_word_not_aligned),
                    cmocka_unit_test(get_word_out_of_bounds),
                    cmocka_unit_test(set_word_success),
                    cmocka_unit_test(set_word_not_aligned),
                    cmocka_unit_test(set_word_not_writable),
                    cmocka_unit_test(set_word_out_of_bounds),
            };
    cmocka_set_message_output(CM_OUTPUT_STDOUT);
    return cmocka_run_group_tests(tests, NULL, NULL);
}