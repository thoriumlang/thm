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
#include "headers/list.h"

static void add_then_get(void **state) {
    List *list = list_create();
    int item = 1;

    list_add(list, &item);

    int *actual = list_get(list, 0);

    assert_int_equal(1, *actual);

    list_destroy(list);
}

static void add_then_size(void **state) {
    List *list = list_create();
    int item = 1;

    assert_int_equal(0, list_size(list));

    list_add(list, &item);

    assert_int_equal(1, list_size(list));

    list_destroy(list);
}


int main() {
    const struct CMUnitTest tests[] =
            {
                    cmocka_unit_test(add_then_get),
                    cmocka_unit_test(add_then_size),
            };
    cmocka_set_message_output(CM_OUTPUT_STDOUT);
    return cmocka_run_group_tests(tests, NULL, NULL);
}