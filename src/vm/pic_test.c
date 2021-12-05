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

#include "pic.h"

static void interrupt_handler_get(void **state) {
    for (int i = 0; i < INTERRUPTS_COUNT; i++) {
        PIC *pic = pic_create();

        pic_interrupt_unmask(pic, i);
        pic_interrupt_trigger(pic, i);
        assert_int_equal(i, pic_interrupt_get(pic));
        assert_true(pic_interrupt_active(pic));

        pic_destroy(pic);
    }
}

static void interrupt_mask_unmask(void **state) {
    for (int i = 0; i < INTERRUPTS_COUNT; i++) {
        PIC *pic = pic_create();

        pic_interrupt_mask(pic, i);
        pic_interrupt_trigger(pic, i);
        assert_false(pic_interrupt_active(pic));
        pic_interrupt_unmask(pic, i);
        assert_true(pic_interrupt_active(pic));
        assert_int_equal(i, pic_interrupt_get(pic));

        pic_destroy(pic);
    }
}

int main() {
    const struct CMUnitTest tests[] =
            {
                    cmocka_unit_test(interrupt_handler_get),
                    cmocka_unit_test(interrupt_mask_unmask),
            };
    cmocka_set_message_output(CM_OUTPUT_STDOUT);
    return cmocka_run_group_tests(tests, NULL, NULL);
}