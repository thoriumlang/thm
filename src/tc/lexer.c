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
#include <stdbool.h>
#include <string.h>
#include "lexer.h"

typedef struct Lexer {
    char *start;
    char *current;
    int line;
    int column;
} Lexer;

static void skip_whitespaces(Lexer *this);

static void skip_comment(Lexer *this);

static void skip_multiline_comment(Lexer *this);

static bool is_at_end(Lexer *this);

static bool is_digit(char c);

static bool is_alpha(char c);

static char advance(Lexer *this);

static bool match(Lexer *this, char expected);

static char peek(Lexer *this, int n);

static Token make_token(Lexer *this, ETokenType token_type);

static Token make_string(Lexer *this, char delim, ETokenType token_type);

static Token make_number(Lexer *this);

static Token make_identifier(Lexer *this);

static Token match_keyword(Lexer *this, int start, int length, const char *rest, ETokenType token_type);

static Token make_error(Lexer *this);

Lexer *lexer_create(char *source, int line, int column) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->start = source;
    lexer->current = source;
    lexer->line = line;
    lexer->column = column;
    return lexer;
}

void lexer_destroy(Lexer *this) {
    free(this);
}

Token lexer_next(Lexer *this) {
    skip_whitespaces(this);

    this->start = this->current;

    if (is_at_end(this)) {
        return make_token(this, TOKEN_EOF);
    }
    char c = advance(this);

    switch (c) {
        case '+':
            return make_token(this, TOKEN_PLUS);
        case '-':
            return make_token(this, TOKEN_MINUS);
        case '*':
            return make_token(this, TOKEN_STAR);
        case '/':
            return make_token(this, TOKEN_SLASH);
        case '[':
            return make_token(this, TOKEN_LBRACKET);
        case ']':
            return make_token(this, TOKEN_RBRACKET);
        case '(':
            return make_token(this, TOKEN_LPAR);
        case ')':
            return make_token(this, TOKEN_RPAR);
        case '{':
            return make_token(this, TOKEN_LBRACE);
        case '}':
            return make_token(this, TOKEN_RBRACE);
        case '&':
            return make_token(this, match(this, '&') ? TOKEN_AND : TOKEN_AMPERSAND);
        case '|':
            return make_token(this, match(this, '|') ? TOKEN_OR : TOKEN_PIPE);
        case '^':
            return make_token(this, TOKEN_CIRC);
        case '!':
            return make_token(this, match(this, '=') ? TOKEN_NOT_EQUALS : TOKEN_EXCLAM);
        case '=':
            return make_token(this, match(this, '=') ? TOKEN_EQUALS : TOKEN_EQUAL);
        case '>':
            return make_token(this, match(this, '=') ? TOKEN_GT_EQUALS : TOKEN_GT);
        case '<':
            return make_token(this, match(this, '=') ? TOKEN_LT_EQUALS : TOKEN_LT);
        case '@':
            return make_token(this, TOKEN_AT);
        case '$':
            return make_token(this, TOKEN_DOLLAR);
        case ':':
            return make_token(this, match(this, ':') ? TOKEN_CAST : TOKEN_COLON);
        case ';':
            return make_token(this, TOKEN_SEMICOLON);
        case ',':
            return make_token(this, TOKEN_COMMA);
        case '"':
            return make_string(this, '"', TOKEN_ZSTRING);
        case '\'':
            return make_string(this, '\'', TOKEN_STRING);
        default:
            if (is_digit(c)) {
                return make_number(this);
            }
            if (is_alpha(c)) {
                return make_identifier(this);
            }
            return make_error(this);
    }
}

static void skip_whitespaces(Lexer *this) {
    while (true) {
        char c = peek(this, 0);
        switch (c) {
            case '\n':
                this->line++;
                this->column = 0;
            case ' ':
            case '\t':
                advance(this);
                break;
            case '/':
                if (peek(this, 1) == '/') {
                    skip_comment(this);
                } else if (peek(this, 1) == '*') {
                    skip_multiline_comment(this);
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static void skip_comment(Lexer *this) {
    while (peek(this, 0) != '\n' && !is_at_end(this)) {
        advance(this);
    }
}

static void skip_multiline_comment(Lexer *this) {
    while (!is_at_end(this)) {
        switch (peek(this, 0)) {
            case '*':
                advance(this);
                if (peek(this, 0) == '/') {
                    advance(this);
                    return;
                }
            case '\n':
                this->line++;
                this->column = 0;
            default:
                advance(this);
        }
    }
}

static bool is_at_end(Lexer *this) {
    return *this->current == '\0';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

static char advance(Lexer *this) {
    this->column++;
    this->current++;
    return this->current[-1];
}

static bool match(Lexer *this, char expected) {
    if (is_at_end(this)) {
        return false;
    }
    if (*this->current != expected) {
        return false;
    }
    advance(this);
    return true;
}

static char peek(Lexer *this, int n) {
    return this->current[n];
}

static Token make_token(Lexer *this, ETokenType token_type) {
    Token result;
    result.type = token_type;
    result.start = this->start;
    result.line = this->line;
    result.length = (int) (this->current - this->start);
    result.column = this->column - result.length;
    return result;
}

static Token make_string(Lexer *this, char delim, ETokenType token_type) {
    this->start = this->current; // skip the first '
    while (!is_at_end(this)) {
        char c = peek(this, 0);
        if (c == delim) {
            Token token = make_token(this, token_type);
            advance(this);
            return token;
        }
        if (c == '\n') {
            this->line++;
            this->column = 1;
        }
        advance(this);
    }
    return make_error(this);
}

static Token make_number(Lexer *this) {
    while (is_digit(peek(this, 0))) {
        advance(this);
    }
    return make_token(this, TOKEN_NUMBER);
}

static Token make_identifier(Lexer *this) {
    while (is_alpha(peek(this, 0)) || is_digit(peek(this, 0)) || peek(this, 0) == '_') {
        advance(this);
    }
    switch (this->start[0]) {
        case 'a':
            return match_keyword(this, 1, 4, "lias", TOKEN_ALIAS);
        case 'b':
            switch (this->start[1]) {
                case 'i':
                    return match_keyword(this, 2, 5, "tflag", TOKEN_BITFLAG);
                case 'y':
                    return match_keyword(this, 2, 2, "te", TOKEN_BYTE);
            }
        case 'c':
            return match_keyword(this, 1, 4, "onst", TOKEN_CONST);
        case 'e':
            switch (this->start[1]) {
                case 'l':
                    return match_keyword(this, 2, 2, "se", TOKEN_ELSE);
                case 'n':
                    return match_keyword(this, 2, 2, "um", TOKEN_ENUM);
                case 'x':
                    return match_keyword(this, 2, 4, "tern", TOKEN_EXTERN);
            }
        case 'f':
            return match_keyword(this, 1, 1, "n", TOKEN_FN);
        case 'i':
            return match_keyword(this, 1, 1, "f", TOKEN_IF);
        case 'p':
            return match_keyword(this, 1, 5, "ublic", TOKEN_PUBLIC);
        case 's':
            return match_keyword(this, 1, 5, "truct", TOKEN_STRUCT);
        case 'u':
            return match_keyword(this, 1, 4, "nion", TOKEN_UNION);
        case 'v':
            switch (this->start[1]) {
                case 'a':
                    return match_keyword(this, 2, 1, "r", TOKEN_VAR);
                case 'o':
                    switch (this->start[2]) {
                        case 'i':
                            return match_keyword(this, 3, 1, "d", TOKEN_VOID);
                        case 'l':
                            return match_keyword(this, 3, 5, "atile", TOKEN_VOLATILE);
                    }
            }
        case 'w':
            switch (this->start[1]) {
                case 'h':
                    return match_keyword(this, 2, 3, "ile", TOKEN_WHILE);
                case 'o':
                    return match_keyword(this, 2, 2, "rd", TOKEN_WORD);
            }
    }
    return make_token(this, TOKEN_IDENTIFIER);
}

static Token match_keyword(Lexer *this, int start, int length, const char *rest, ETokenType token_type) {
    if (this->current - this->start == start + length && memcmp(this->start + start, rest, length) == 0) {
        return make_token(this, token_type);
    }
    return make_token(this, TOKEN_IDENTIFIER);
}

static Token make_error(Lexer *this) {
    Token token = {
            .type = TOKEN_ERROR,
            .start = this->start,
            .line = this->line,
            .column = this->column,
            .length = (int) (this->current - this->start),
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