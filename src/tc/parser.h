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

#ifndef THM_PARSER_H
#define THM_PARSER_H

#include <stdbool.h>
#include <list.h>
#include "lexer.h"
#include "ast.h"

typedef struct Parser Parser;

Parser *parser_create(Lexer *lexer);

void parser_destroy(Parser *this);

AstRoot *parser_parse(Parser *this);

#endif //THM_PARSER_H
