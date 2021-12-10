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

#ifndef THM_LEXER_H
#define THM_LEXER_H

#endif //THM_LEXER_H

typedef struct Lexer Lexer;

typedef struct Token {
    struct {
        unsigned int start;
        unsigned int end;
    } position;
    enum {
        TOKEN_COMMA,
        TOKEN_COLON,
        TOKEN_IDENTIFIER,
        TOKEN_NUMBER,
        TOKEN_EOF,
        TOKEN_UNKNOWN,
    } type;
    union {
        char *string;
        int number;
    } token;
} Token;

Lexer *lexer_create();

void lexer_reset(Lexer *this, char *text);

Token *lexer_next(Lexer *this);