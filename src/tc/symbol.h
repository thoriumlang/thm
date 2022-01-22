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

#ifndef THM_SYMBOL_H
#define THM_SYMBOL_H

enum SymbolKind {
    SYM_VAR, SYM_CONST, SYM_FN
};

typedef struct Symbol {
    char *name;
    enum SymbolKind kind;
    void *ast_node;
} Symbol;

Symbol *symbol_create(char *name, enum SymbolKind kind, void *ast_node);

#endif //THM_SYMBOL_H
