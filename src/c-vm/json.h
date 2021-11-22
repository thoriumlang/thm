/**
 * Copyright 2019 Christophe Pollet
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

#ifndef C_VM_JSON_H
#define C_VM_JSON_H

#include "stdbool.h"

typedef struct JsonElement JsonElement;

void json_destroy(JsonElement *json_element);

JsonElement *json_object();

void json_object_put(JsonElement *object, char *key, JsonElement *element);

JsonElement *json_array();

void json_array_append(JsonElement *array, JsonElement *element);

JsonElement *json_string(char *value);

JsonElement *json_number(double value);

JsonElement *json_bool(bool value);

char *json_serialize(JsonElement *element);

#endif //C_VM_JSON_H
