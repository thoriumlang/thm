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

#include <stdbool.h>
#include <string.h>
#include "lexer.h"
#include "memory.h"

typedef struct Lexer {
    char *start;
    char *current;
    int line;
    int column;
} Lexer;

static void skip_whitespaces(Lexer *self);

static void skip_comment(Lexer *self);

static void skip_multiline_comment(Lexer *self);

static bool is_at_end(Lexer *self);

static bool is_digit(char c);

static bool is_alpha(char c);

static char advance(Lexer *self);

static bool match(Lexer *self, char expected);

static char peek(Lexer *self, int n);

static Token make_token(Lexer *self, ETokenType token_type);

static Token make_string(Lexer *self, char delim, ETokenType token_type);

static Token make_number(Lexer *self);

static Token make_identifier(Lexer *self);

static Token match_keyword(Lexer *self, int start, int length, const char *rest, ETokenType token_type);

static Token make_error(Lexer *self);

Lexer *lexer_create(char *source, int line, int column) {
    Lexer *lexer = memory_alloc(sizeof(Lexer));
    lexer->start = source;
    lexer->current = source;
    lexer->line = line;
    lexer->column = column;
    return lexer;
}

void lexer_destroy(Lexer *self) {
    memory_free(self);
}

Token lexer_next(Lexer *self) {
    skip_whitespaces(self);

    self->start = self->current;

    if (is_at_end(self)) {
        return make_token(self, TOKEN_EOF);
    }
    char c = advance(self);

    switch (c) {
        case '+':
            return make_token(self, TOKEN_PLUS);
        case '-':
            return make_token(self, TOKEN_MINUS);
        case '*':
            return make_token(self, TOKEN_STAR);
        case '/':
            return make_token(self, TOKEN_SLASH);
        case '[':
            return make_token(self, TOKEN_LBRACKET);
        case ']':
            return make_token(self, TOKEN_RBRACKET);
        case '(':
            return make_token(self, TOKEN_LPAR);
        case ')':
            return make_token(self, TOKEN_RPAR);
        case '{':
            return make_token(self, TOKEN_LBRACE);
        case '}':
            return make_token(self, TOKEN_RBRACE);
        case '&':
            return make_token(self, match(self, '&') ? TOKEN_AND : TOKEN_AMPERSAND);
        case '|':
            return make_token(self, match(self, '|') ? TOKEN_OR : TOKEN_PIPE);
        case '^':
            return make_token(self, TOKEN_CIRC);
        case '!':
            return make_token(self, match(self, '=') ? TOKEN_NOT_EQUALS : TOKEN_EXCLAM);
        case '=':
            return make_token(self, match(self, '=') ? TOKEN_EQUALS : TOKEN_EQUAL);
        case '>':
            return make_token(self, match(self, '=') ? TOKEN_GT_EQUALS : TOKEN_GT);
        case '<':
            return make_token(self, match(self, '=') ? TOKEN_LT_EQUALS : TOKEN_LT);
        case '@':
            return make_token(self, TOKEN_AT);
        case '$':
            return make_token(self, TOKEN_DOLLAR);
        case ':':
            return make_token(self, match(self, ':') ? TOKEN_CAST : TOKEN_COLON);
        case ';':
            return make_token(self, TOKEN_SEMICOLON);
        case ',':
            return make_token(self, TOKEN_COMMA);
        case '"':
            return make_string(self, '"', TOKEN_ZSTRING);
        case '\'':
            return make_string(self, '\'', TOKEN_STRING);
        default:
            if (is_digit(c)) {
                return make_number(self);
            }
            if (is_alpha(c)) {
                return make_identifier(self);
            }
            return make_error(self);
    }
}

static void skip_whitespaces(Lexer *self) {
    while (true) {
        char c = peek(self, 0);
        switch (c) {
            case '\n':
                self->line++;
                self->column = 0;
            case ' ':
            case '\t':
                advance(self);
                break;
            case '/':
                if (peek(self, 1) == '/') {
                    skip_comment(self);
                } else if (peek(self, 1) == '*') {
                    skip_multiline_comment(self);
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static void skip_comment(Lexer *self) {
    while (peek(self, 0) != '\n' && !is_at_end(self)) {
        advance(self);
    }
}

static void skip_multiline_comment(Lexer *self) {
    while (!is_at_end(self)) {
        switch (peek(self, 0)) {
            case '*':
                advance(self);
                if (peek(self, 0) == '/') {
                    advance(self);
                    return;
                }
            case '\n':
                self->line++;
                self->column = 0;
            default:
                advance(self);
        }
    }
}

static bool is_at_end(Lexer *self) {
    return *self->current == '\0';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

static char advance(Lexer *self) {
    self->column++;
    self->current++;
    return self->current[-1];
}

static bool match(Lexer *self, char expected) {
    if (is_at_end(self)) {
        return false;
    }
    if (*self->current != expected) {
        return false;
    }
    advance(self);
    return true;
}

static char peek(Lexer *self, int n) {
    return self->current[n];
}

static Token make_token(Lexer *self, ETokenType token_type) {
    Token result;
    result.type = token_type;
    result.start = self->start;
    result.line = self->line;
    result.length = (int) (self->current - self->start);
    result.column = self->column - result.length;
    return result;
}

static Token make_string(Lexer *self, char delim, ETokenType token_type) {
    self->start = self->current; // skip the first '
    while (!is_at_end(self)) {
        char c = peek(self, 0);
        if (c == delim) {
            Token token = make_token(self, token_type);
            advance(self);
            return token;
        }
        if (c == '\n') {
            self->line++;
            self->column = 1;
        }
        advance(self);
    }
    return make_error(self);
}

static Token make_number(Lexer *self) {
    while (is_digit(peek(self, 0))) {
        advance(self);
    }
    return make_token(self, TOKEN_NUMBER);
}

static Token make_identifier(Lexer *self) {
    while (is_alpha(peek(self, 0)) || is_digit(peek(self, 0)) || peek(self, 0) == '_') {
        advance(self);
    }
    switch (self->start[0]) {
        case 'a':
            return match_keyword(self, 1, 4, "lias", TOKEN_ALIAS);
        case 'b':
            switch (self->start[1]) {
                case 'i':
                    return match_keyword(self, 2, 5, "tflag", TOKEN_BITFLAG);
                case 'y':
                    return match_keyword(self, 2, 2, "te", TOKEN_BYTE);
            }
        case 'c':
            return match_keyword(self, 1, 4, "onst", TOKEN_CONST);
        case 'e':
            switch (self->start[1]) {
                case 'l':
                    return match_keyword(self, 2, 2, "se", TOKEN_ELSE);
                case 'n':
                    return match_keyword(self, 2, 2, "um", TOKEN_ENUM);
                case 'x':
                    return match_keyword(self, 2, 4, "tern", TOKEN_EXTERN);
            }
        case 'f':
            return match_keyword(self, 1, 1, "n", TOKEN_FN);
        case 'i':
            return match_keyword(self, 1, 1, "f", TOKEN_IF);
        case 'p':
            return match_keyword(self, 1, 5, "ublic", TOKEN_PUBLIC);
        case 's':
            return match_keyword(self, 1, 5, "truct", TOKEN_STRUCT);
        case 'u':
            return match_keyword(self, 1, 4, "nion", TOKEN_UNION);
        case 'v':
            switch (self->start[1]) {
                case 'a':
                    return match_keyword(self, 2, 1, "r", TOKEN_VAR);
                case 'o':
                    switch (self->start[2]) {
                        case 'i':
                            return match_keyword(self, 3, 1, "d", TOKEN_VOID);
                        case 'l':
                            return match_keyword(self, 3, 5, "atile", TOKEN_VOLATILE);
                    }
            }
        case 'w':
            switch (self->start[1]) {
                case 'h':
                    return match_keyword(self, 2, 3, "ile", TOKEN_WHILE);
                case 'o':
                    return match_keyword(self, 2, 2, "rd", TOKEN_WORD);
            }
    }
    return make_token(self, TOKEN_IDENTIFIER);
}

static Token match_keyword(Lexer *self, int start, int length, const char *rest, ETokenType token_type) {
    if (self->current - self->start == start + length && memcmp(self->start + start, rest, length) == 0) {
        return make_token(self, token_type);
    }
    return make_token(self, TOKEN_IDENTIFIER);
}

static Token make_error(Lexer *self) {
    Token token = {
            .type = TOKEN_ERROR,
            .start = self->start,
            .line = self->line,
            .column = self->column,
            .length = (int) (self->current - self->start),
    };

    return token;
}

char *token_type_to_string(ETokenType token_type) {
    switch (token_type) {
        case TOKEN_ERROR:
            return "ERROR";
        case TOKEN_EOF:
            return "EOF";
        case TOKEN_PLUS:
            return "+";
        case TOKEN_MINUS:
            return "-";
        case TOKEN_STAR:
            return "*";
        case TOKEN_SLASH:
            return "/";
        case TOKEN_LPAR:
            return "(";
        case TOKEN_RPAR:
            return ")";
        case TOKEN_LBRACKET:
            return "[";
        case TOKEN_RBRACKET:
            return "]";
        case TOKEN_LBRACE:
            return "{";
        case TOKEN_RBRACE:
            return "}";
        case TOKEN_AMPERSAND:
            return "&";
        case TOKEN_PIPE:
            return "|";
        case TOKEN_CIRC:
            return "^";
        case TOKEN_EXCLAM:
            return "!";
        case TOKEN_EQUAL:
            return "=";
        case TOKEN_GT:
            return ">";
        case TOKEN_LT:
            return "<";
        case TOKEN_AT:
            return "@";
        case TOKEN_DOLLAR:
            return "$";
        case TOKEN_COLON:
            return ":";
        case TOKEN_SEMICOLON:
            return ";";
        case TOKEN_COMMA:
            return ",";
        case TOKEN_AND:
            return "&&";
        case TOKEN_OR:
            return "||";
        case TOKEN_EQUALS:
            return "==";
        case TOKEN_NOT_EQUALS:
            return "!=";
        case TOKEN_GT_EQUALS:
            return ">=";
        case TOKEN_LT_EQUALS:
            return "<=";
        case TOKEN_CAST:
            return "::";
        case TOKEN_NUMBER:
            return "NUMBER";
        case TOKEN_STRING:
            return "STRING";
        case TOKEN_ZSTRING:
            return "ZSTRING";
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN_ALIAS:
            return "alias";
        case TOKEN_BITFLAG:
            return "bitflag";
        case TOKEN_BYTE:
            return "byte";
        case TOKEN_CONST:
            return "const";
        case TOKEN_ELSE:
            return "else";
        case TOKEN_ENUM:
            return "enum";
        case TOKEN_EXTERN:
            return "extern";
        case TOKEN_FN:
            return "fn";
        case TOKEN_IF:
            return "if";
        case TOKEN_PUBLIC:
            return "public";
        case TOKEN_STRUCT:
            return "struct";
        case TOKEN_UNION:
            return "union";
        case TOKEN_VAR:
            return "var";
        case TOKEN_VOID:
            return "void";
        case TOKEN_VOLATILE:
            return "volatile";
        case TOKEN_WHILE:
            return "while";
        case TOKEN_WORD:
            return "word";
        default:
            return "???";
    }
}