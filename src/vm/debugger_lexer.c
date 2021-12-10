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

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "debugger_lexer.h"

typedef struct Lexer {
    char *text;
    unsigned long text_len;
    unsigned long current;
} Lexer;

Lexer *lexer_create() {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->text = NULL;
    lexer->text_len = 0;
    lexer->current = 0;
    return lexer;
}

void lexer_reset(Lexer *this, char *text) {
    this->text = text;
    this->text_len = strlen(text);
    this->current = 0;
}

char peek(Lexer *this);

char next(Lexer *this);

Token *read_identifier(Lexer *this);

Token *read_number(Lexer *this);

int read_decnumber(Lexer *this);

int read_hexnumber(Lexer *this);

Token *read_unknown(Lexer *this);

bool is_alpha(char c) { return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '_'; }

bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool is_hexdigit(char c) { return is_digit(c) || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f'; }

bool is_alphanum(char c) { return is_alpha(c) || c >= '0' && c <= '9'; }

bool is_whitespace(char c) { return c == ' ' || c == '\n' || c == '\t'; }

Token *lexer_next(Lexer *this) {
    while (this->current < this->text_len) {
        char c = peek(this);
        if (is_whitespace(c)) {
            next(this);
            continue;
        }
        if (is_alpha(c)) {
            return read_identifier(this);
        }
        if (is_digit(c)) {
            return read_number(this);
        }
        if (c == ':') {
            next(this);
            Token *colon = malloc(sizeof(Token));
            colon->token.string = ":";
            colon->position.start = this->current;
            colon->position.end = this->current;
            colon->type = TOKEN_COLON;
            return colon;
        }
        if (c == ',') {
            next(this);
            Token *comma = malloc(sizeof(Token));
            comma->token.string = ",";
            comma->position.start = this->current;
            comma->position.end = this->current;
            comma->type = TOKEN_COMMA;
            return comma;
        }
        return read_unknown(this);
    }
    Token *eof = malloc(sizeof(Token));
    eof->type = TOKEN_EOF;
    return eof;
}

Token *read_identifier(Lexer *this) {
    unsigned long token_start = this->current;
    while (is_alphanum(peek(this)) && this->current < this->text_len) {
        next(this);
    }
    Token *token = malloc(sizeof(Token));
    token->type = TOKEN_IDENTIFIER;
    token->position.start = token_start;
    token->position.end = this->current;
    token->token.string = malloc(token->position.end - token->position.start + 1);
    strncat(token->token.string, &this->text[token_start], token->position.end - token->position.start);
    return token;
}

Token *read_number(Lexer *this) {
    unsigned long token_start = this->current;
    int number;
    if (peek(this) == '0' && next(this) == 'x') {
        next(this);
        number = read_hexnumber(this);
    } else {
        number = read_decnumber(this);
    }

    Token *token = malloc(sizeof(Token));
    token->type = TOKEN_NUMBER;
    token->position.start = token_start;
    token->position.end = this->current;
    token->token.number = number;
    return token;
}

int read_decnumber(Lexer *this) {
    int value = 0;
    while (is_digit(peek(this)) && this->current < this->text_len) {
        int i = peek(this) - '0';
        if (value > 0 || i > 0) {
            value *= 10;
            value += i;
        }
        next(this);
    }
    return value;
}

int read_hexnumber(Lexer *this) {
    int value = 0;
    while (is_hexdigit(peek(this)) && this->current < this->text_len) {
        int i;
        char c = peek(this);
        if (is_digit(c)) {
            i = c - '0';
        } else {
            i = 10 + tolower(c) - 'a';
        }
        if (value > 0 || i > 0) {
            value *= 16;
            value += i;
        }
        next(this);
    }
    return value;
}

Token *read_unknown(Lexer *this) {
    unsigned long token_start = this->current;
    while (!is_whitespace(peek(this)) && this->current < this->text_len) {
        this->current++;
    }
    Token *token = malloc(sizeof(Token));
    token->type = TOKEN_UNKNOWN;
    token->position.start = token_start;
    token->position.end = this->current;
    token->token.string = malloc(token->position.end - token->position.start + 1);
    strncat(token->token.string, &this->text[token_start], token->position.end - token->position.start);
    return token;
}

char peek(Lexer *this) {
    return this->text[this->current];
}

char next(Lexer *this) {
    this->current += 1;
    return peek(this);
}