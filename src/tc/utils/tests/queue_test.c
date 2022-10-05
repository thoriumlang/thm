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
#include "../headers/queue.h"
#include "../headers/memory.h"

MEMORY_GLOBAL()

static void enqueue_grows(void **state) {
    Queue *queue = queue_create(1);
    int item = 1;

    queue_enqueue(queue, &item);
    queue_enqueue(queue, &item);
    queue_enqueue(queue, &item);
    queue_enqueue(queue, &item);
    queue_enqueue(queue, &item);

    assert_int_equal(5, queue_size(queue));
}

static void enqueue_dequeue(void **state) {
    Queue *queue = queue_create(1);
    int item = 1;

    queue_enqueue(queue, &item);

    int *actual = queue_dequeue(queue);

    assert_int_equal(1, *actual);

    actual = queue_dequeue(queue);
    assert_null(actual);
}

static void enqueue_then_dequeue(void **state) {
    Queue *queue = queue_create(1);
    int item1 = 1;
    int item2 = 2;
    int item3 = 3;
    int item4 = 4;

    queue_enqueue(queue, &item1);
    queue_enqueue(queue, &item2);
    queue_enqueue(queue, &item3);
    queue_enqueue(queue, &item4);

    int *actual;

    actual = queue_dequeue(queue);
    assert_int_equal(1, *actual);
    actual = queue_dequeue(queue);
    assert_int_equal(2, *actual);
    actual = queue_dequeue(queue);
    assert_int_equal(3, *actual);
    actual = queue_dequeue(queue);
    assert_int_equal(4, *actual);
}

static void mixed_enqueue_and_dequeue(void **state) {
    Queue *queue = queue_create(1);
    int item1 = 1;
    int item2 = 2;
    int item3 = 3;
    int item4 = 4;
    int item5 = 5;
    int *actual;

    assert_true(queue_is_empty(queue));

    queue_enqueue(queue, &item1);
    assert_false(queue_is_empty(queue));
    queue_enqueue(queue, &item2);
    assert_int_equal(2, queue_size(queue));

    actual = queue_dequeue(queue);
    assert_int_equal(1, *actual);
    assert_int_equal(1, queue_size(queue));

    queue_enqueue(queue, &item3);
    assert_int_equal(2, queue_size(queue));

    actual = queue_dequeue(queue);
    assert_int_equal(2, *actual);
    assert_int_equal(1, queue_size(queue));

    queue_enqueue(queue, &item4);
    queue_enqueue(queue, &item5);
    assert_int_equal(3, queue_size(queue));

    actual = queue_dequeue(queue);
    assert_int_equal(3, *actual);
    assert_int_equal(2, queue_size(queue));
    actual = queue_dequeue(queue);
    assert_int_equal(4, *actual);
    assert_int_equal(1, queue_size(queue));
    actual = queue_dequeue(queue);
    assert_int_equal(5, *actual);
    assert_int_equal(0, queue_size(queue));
    assert_true(queue_is_empty(queue));
}

static void peek(void **state) {
    Queue *queue = queue_create(2);
    int item1 = 1;
    int item2 = 2;

    queue_enqueue(queue, &item1);
    queue_enqueue(queue, &item2);

    int *actual;
    actual = queue_peek(queue, 0);
    assert_int_equal(1, *actual);

    actual = queue_peek(queue, 1);
    assert_int_equal(2, *actual);
}

int main(void) {
    MEMORY_INIT()
    const struct CMUnitTest tests[] =
            {
                    cmocka_unit_test(enqueue_grows),
                    cmocka_unit_test(enqueue_dequeue),
                    cmocka_unit_test(enqueue_then_dequeue),
                    cmocka_unit_test(mixed_enqueue_and_dequeue),
                    cmocka_unit_test(peek),
            };
    cmocka_set_message_output(CM_OUTPUT_STDOUT);
    return cmocka_run_group_tests(tests, NULL, NULL);
}