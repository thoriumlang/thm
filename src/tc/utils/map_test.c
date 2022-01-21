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
#include <string.h>
#include "headers/map.h"

List *list_create() {
    return NULL;
}

void list_add(List *this, void *item) {
    // nothing
}

static void create(void **state) {
    Map *map = map_create(map_hash_fn_str, map_eq_fn_str);
    assert_non_null(map);
}

static void destroy(void **state) {
    Map *map = map_create(map_hash_fn_str, map_eq_fn_str);
    map_destroy(map);
}

static void empty(void **state) {
    Map *map = map_create(map_hash_fn_str, map_eq_fn_str);

    assert_true(map_is_empty(map));
    assert_int_equal(0, map_size(map));

    map_destroy(map);
}

static void put_one(void **state) {
    Map *map = map_create(map_hash_fn_str, map_eq_fn_str);

    map_put(map, "key", "value");

    assert_false(map_is_empty(map));
    assert_int_equal(1, map_size(map));
    assert_true(map_is_present(map, "key"));
    assert_int_equal(0, strcmp("value", map_get(map, "key")));

    map_destroy(map);
}

static void put_same_key(void **state) {
    Map *map = map_create(map_hash_fn_str, map_eq_fn_str);

    map_put(map, "key", "value1");
    void *old = map_put(map, "key", "value2");

    assert_false(map_is_empty(map));
    assert_int_equal(1, map_size(map));
    assert_true(map_is_present(map, "key"));
    assert_int_equal(0, strcmp("value2", map_get(map, "key")));
    assert_int_equal(0, strcmp("value1", old));

    map_destroy(map);
}

static void put_different_key(void **state) {
    Map *map = map_create(map_hash_fn_str, map_eq_fn_str);

    char *key1 = "key1";
    char *key2 = "key2";

    map_put(map, key1, "value1");
    map_put(map, key2, "value2");

    assert_false(map_is_empty(map));
    assert_int_equal(2, map_size(map));
    assert_true(map_is_present(map, key1));
    assert_true(map_is_present(map, key2));
    assert_int_equal(0, strcmp("value1", map_get(map, key1)));
    assert_int_equal(0, strcmp("value2", map_get(map, key2)));

    map_destroy(map);
}

static void get_missing(void **state) {
    Map *map = map_create(map_hash_fn_str, map_eq_fn_str);

    assert_null(map_get(map, "missing"));
    assert_false(map_is_present(map, "missing"));

    map_destroy(map);
}

static void remove_last(void **state) {
    Map *map = map_create(map_hash_fn_str, map_eq_fn_str);

    map_put(map, "key", "value");
    map_remove(map, "key");

    assert_true(map_is_empty(map));
    assert_int_equal(0, map_size(map));
    assert_null(map_get(map, "key"));

    map_destroy(map);
}

static void remove(void **state) {
    Map *map = map_create(map_hash_fn_str, map_eq_fn_str);

    map_put(map, "key1", "value");
    map_put(map, "key2", "value");
    map_remove(map, "key1");

    assert_false(map_is_empty(map));
    assert_int_equal(1, map_size(map));
    assert_null(map_get(map, "key1"));
    assert_non_null(map_get(map, "key2"));

    map_destroy(map);
}

int main() {
    const struct CMUnitTest tests[] =
            {
                    cmocka_unit_test(create),
                    cmocka_unit_test(destroy),
                    cmocka_unit_test(empty),
                    cmocka_unit_test(put_one),
                    cmocka_unit_test(put_same_key),
                    cmocka_unit_test(put_different_key),
                    cmocka_unit_test(get_missing),
                    cmocka_unit_test(remove_last),
                    cmocka_unit_test(remove),
            };
    cmocka_set_message_output(CM_OUTPUT_STDOUT);
    return cmocka_run_group_tests(tests, NULL, NULL);
}