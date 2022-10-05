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
#include "../headers/list.h"
#include "../headers/memory.h"

MEMORY_GLOBAL()
static void add_then_get(void **state) {
    List *list = list_create();
    int item = 1;

    cpocl_list_add(list, &item);

    int *actual = cpocl_list_get(list, 0);

    assert_int_equal(1, *actual);

    cpocl_list_destroy(list);
}

static void add_add_then_get_get(void **state) {
    List *list = list_create();
    int item1 = 1;
    int item2 = 2;

    cpocl_list_add(list, &item1);
    cpocl_list_add(list, &item2);

    int *actual;
    actual = cpocl_list_get(list, 0);
    assert_int_equal(1, *actual);
    actual = cpocl_list_get(list, 1);
    assert_int_equal(2, *actual);

    cpocl_list_destroy(list);
}

static void add_then_size(void **state) {
    List *list = list_create();
    int item = 1;

    assert_int_equal(0, cpocl_list_size(list));

    cpocl_list_add(list, &item);

    assert_int_equal(1, cpocl_list_size(list));

    cpocl_list_destroy(list);
}

int main(void) {
    MEMORY_INIT()
    const struct CMUnitTest tests[] =
            {
                    cmocka_unit_test(add_then_get),
                    cmocka_unit_test(add_add_then_get_get),
                    cmocka_unit_test(add_then_size),
            };
    cmocka_set_message_output(CM_OUTPUT_STDOUT);
    return cmocka_run_group_tests(tests, NULL, NULL);
}