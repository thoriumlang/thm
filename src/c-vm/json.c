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

#include "ccan/json/json.h"
#include "json.h"

typedef struct JsonElement {
    JsonNode node;
} JsonElement;

inline void json_destroy(JsonElement *json_element) {
    json_delete(&json_element->node);
}

inline JsonElement *json_object() {
    return (JsonElement *) json_mkobject();
}

inline void json_object_put(JsonElement *object, char *key, JsonElement *element) {
    json_append_member((JsonNode *) object, key, (JsonNode *) element);
}

inline JsonElement *json_array() {
    return (JsonElement *) json_mkarray();
}

inline void json_array_append(JsonElement *array, JsonElement *element) {
    json_append_element((JsonNode *) array, (JsonNode *) element);
}

inline JsonElement *json_string(char *value) {
    return (JsonElement *) json_mkstring(value);
}

inline JsonElement *json_number(double value) {
    return (JsonElement *) json_mknumber(value);
}

inline JsonElement *json_bool(bool value) {
    return (JsonElement *) json_mkbool(value);
}

inline char *json_serialize(JsonElement *element) {
    char *serialized = json_encode((JsonNode *) element);
    json_destroy(element);
    return serialized;
}