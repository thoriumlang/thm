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

#ifndef THM_LIST_H
#define THM_LIST_H

typedef struct List List;

List *list_create();

void list_destroy(List *this);

void list_add(List *this, void *item);

size_t list_size(List *this);

void *list_get(List *this, size_t index);

#endif //THM_LIST_H
