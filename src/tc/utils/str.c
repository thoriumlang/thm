/**
 * Copyright 2022 Christophe Pollet
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

#include <string.h>
#include <math.h>
#include "headers/str.h"

void str_repeat(char *dst, const char *str, size_t count) {
    if (count < 1) {
        return;
    }
    for (size_t i = count; i > 0; i--) {
        strcat(dst, str);
    }
}

size_t str_lenszt(size_t number) {
    return (size_t) floor(log10((double) number)) + 1;
}
