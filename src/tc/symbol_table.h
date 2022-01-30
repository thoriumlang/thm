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

#ifndef THM_SYMBOL_TABLE_H
#define THM_SYMBOL_TABLE_H

#include <stdbool.h>
#include "symbol.h"

typedef struct SymbolTable SymbolTable;

SymbolTable *symbol_table_create(void);

SymbolTable *symbol_table_create_child_for(SymbolTable *self, /*actually a (AstNode*) */void *node);

void symbol_table_destroy(SymbolTable *self);

void symbol_table_add(SymbolTable *self, Symbol *symbol);

bool symbol_table_symbol_exists_in_current_scope(SymbolTable *self, char *name);

bool symbol_table_symbol_exists(SymbolTable *self, char *name);

Symbol *symbol_table_get(SymbolTable *self, char *name);

void symbol_table_dump(SymbolTable *self);

#endif //THM_SYMBOL_TABLE_H
