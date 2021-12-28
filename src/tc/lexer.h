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

typedef struct Lexer Lexer;

typedef enum {
    // 1 char
    TOKEN_ERROR,
    TOKEN_EOF,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LPAR,
    TOKEN_RPAR,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_AMPERSAND,
    TOKEN_PIPE,
    TOKEN_CIRC,
    TOKEN_EXCLAM,
    TOKEN_EQUAL,
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_AT,
    TOKEN_DOLLAR,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    // 2 chars
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_EQUALS,
    TOKEN_NOT_EQUALS,
    TOKEN_GT_EQUALS,
    TOKEN_LT_EQUALS,
    TOKEN_CAST,
    // other
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_ZSTRING,
    TOKEN_IDENTIFIER,
    // reserved
    TOKEN_ALIAS,
    TOKEN_BITFLAG,
    TOKEN_BYTE,
    TOKEN_CONST,
    TOKEN_ELSE,
    TOKEN_ENUM,
    TOKEN_EXTERN,
    TOKEN_FN,
    TOKEN_IF,
    TOKEN_PUBLIC,
    TOKEN_STRUCT,
    TOKEN_UNION,
    TOKEN_VAR,
    TOKEN_VOID,
    TOKEN_VOLATILE,
    TOKEN_WHILE,
    TOKEN_WORD,
} ETokenType;

typedef struct Token {
    ETokenType type;
    const char *start;
    int length;
    int line;
    int column;
} Token;

Lexer *lexer_create(char *source);

void lexer_destroy(Lexer *this);

Token lexer_next(Lexer *this);

char *token_type_to_string(ETokenType token_type);

#endif //THM_LEXER_H
