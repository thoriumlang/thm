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
#include "../headers/result.h"
#include "../headers/memory.h"

MEMORY_GLOBAL()
static void result_success(void **state) {
    int val = 1;

    CpoclResult *result = result_create_success(&val);

    assert_true(result_is_success(result));
    assert_int_equal(1, *(int *) result_get_value(result));

    result_destroy(result);
}

static void result_error(void **state) {
    int val = 1;

    CpoclResult *result = result_create_error(&val);

    assert_false(result_is_success(result));
    assert_int_equal(1, *(int *) result_get_error(result));

    result_destroy(result);
}

static void result_get_value_fails_if_error(void **state) {
    int val = 1;
    expect_assert_failure(result_get_value(result_create_error(&val)))
}

static void result_get_error_fails_if_success(void **state) {
    int val = 1;
    expect_assert_failure(result_get_error(result_create_success(&val)))
}

// todo https://api.cmocka.org/group__cmocka__alloc.html
// todo https://api.cmocka.org/group__cmocka__mock__assert.html

int main(void) {
    MEMORY_INIT()
    const struct CMUnitTest tests[] =
            {
                    cmocka_unit_test(result_success),
                    cmocka_unit_test(result_error),
//                    cmocka_unit_test(result_get_value_fails_if_error), // fixme enable when UNIT_TESTING works
//                    cmocka_unit_test(result_get_error_fails_if_success), // fixme enable when UNIT_TESTING works
            };
    cmocka_set_message_output(CM_OUTPUT_STDOUT);
    return cmocka_run_group_tests(tests, NULL, NULL);
}