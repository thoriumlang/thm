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

#include <malloc.h>
#include "symbol.h"

/**
 * Creates a new symbol.
 * @param name the symbol's name; lifetime is manager by caller
 * @param kind the symbol's kind, any of SymbolKind
 * @param ast_node the AST node defining this symbol; lifetime is manager by caller
 * @return a new symbol.
 */
Symbol *symbol_create(char *name, enum SymbolKind kind, void *ast_node) {
    Symbol *symbol = malloc(sizeof(Symbol));
    symbol->name = name;
    symbol->kind = kind;
    symbol->ast_node=ast_node;
    return symbol;
}
