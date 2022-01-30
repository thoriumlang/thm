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

#include <queue.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "lexer.h"
#include "memory.h"

typedef struct Parser {
    Lexer *lexer;
    Queue *tokens;
    bool error_found;
    bool error_recovery;
} Parser;

#pragma region helpers

/**
 * Returns a pointer to the nth next token.
 * @param self the parser instance.
 * @param n the index of the next token to return; starts at 0.
 * @return the nth next token, NULL if it does not exist.
 */
static Token *peek(Parser *self, int n) {
    for (size_t i = queue_size(self->tokens); i < n + 1; i++) {
        Token token = lexer_next(self->lexer);
        Token *t = memory_alloc(sizeof(Token));
        memcpy(t, &token, sizeof(Token));
        queue_enqueue(self->tokens, t);
    }
    return queue_peek(self->tokens, n);
}

/**
 * Returns whether the end of the token stream is reached.
 * @param self the parser insrance
 * @return true when no tokens are available anymore.
 */
static bool is_at_end(Parser *self) {
    return peek(self, 0)->type == TOKEN_EOF;
}

/**
 * Consumes and returns the next token. In case there is no next token, returns an EOF token.
 * @param self the parser instance.
 * @return the next token, or an EOF token is no next token exists.
 */
static Token advance(Parser *self) {
    if (queue_is_empty(self->tokens)) {
        return lexer_next(self->lexer);
    }

    Token *token_ptr = queue_dequeue(self->tokens);
    Token token;
    memcpy(&token, token_ptr, sizeof(Token));
    memory_free(token_ptr);

    return token;
}

/**
 * Returns whether the next token is of the expected type without consuming it.
 * @param self the parser instance.
 * @param expected the expected next token type.
 * @return true if the next token is of the expected type; false otherwise.
 */
static bool check(Parser *self, ETokenType expected) {
    return (peek(self, 0)->type == expected);
}

/**
 * Returns whether any of the n next tokens is of the expected type.
 * @param self the parser instance.
 * @param expected the expected token type.
 * @param n the amount of next tokens to check.
 * @return true if any of the next n tokens is of the expected type; false otherwise.
 */
static bool check_within(Parser *self, ETokenType expected, int n) {
    for (int i = 0; i < n; i++) {
        if (peek(self, i)->type == expected) {
            return true;
        }
    }
    return false;
}

/**
 * Returns whether the next token is of the expected type. If it is, it consumes it, otherwise it does not.
 * @param self
 * @param expected
 * @return true if the next token is of the expected type; false otherwise.
 */
static bool match(Parser *self, ETokenType expected) {
    if (is_at_end(self) || !check(self, expected)) {
        return false;
    }
    advance(self);
    return true;
}

static void print_token_expected_error(Parser *self, size_t tokens, ...) {
    if (self->error_recovery) {
        return;
    }

    self->error_found = true;
    self->error_recovery = true;

    va_list argp;
    va_start(argp, tokens);

    fprintf(stderr, "Expected ");
    if (tokens > 1) {
        fprintf(stderr, "( ");
    }
    for (size_t i = 0; i < tokens; i++) {
        ETokenType token = va_arg(argp, ETokenType);
        if (i > 0) {
            fprintf(stderr, " | ");
        }
        fprintf(stderr, "<%s>", token_type_to_string(token));
    }
    if (tokens > 1) {
        fprintf(stderr, " )");
    }
    Token *token = peek(self, 0);
    fprintf(stderr, " at %i:%i\n", token->line, token->column);
}

/**
 * Triggers an error in case the next token is not of the expected type.
 * @param self the parser instance.
 * @param expected the next token expected type.
 */
static void expect(Parser *self, ETokenType expected) {
    if (!match(self, expected)) {
        print_token_expected_error(self, 1, expected);
    }
}

static void print_expected_error(Parser *self, const char *expected) {
    if (self->error_recovery) {
        return;
    }

    self->error_found = true;
    self->error_recovery = true;

    Token *token = peek(self, 0);
    fprintf(stderr, "Expected %s at %i:%i\n", expected, token->line, token->column);
}

#ifdef CPOCL_MEMORY_DEBUG
#undef cpocl_memory_alloc
#undef cpocl_memory_free

static void *cpocl_memory_alloc(size_t size) {
    return cpocl_memory_alloc_debug(size, "Parser.Queue", 0);
}

static void cpocl_memory_free(void *ptr) {
    cpocl_memory_free_debug(ptr, "Parser.Queue", 0);
}

#endif

#pragma endregion

#pragma region parsing

// <identifier> := <IDENTIFIER>
static AstNodeIdentifier *parse_identifier(Parser *self) {
    if (!check(self, TOKEN_IDENTIFIER)) {
        print_token_expected_error(self, 1, TOKEN_IDENTIFIER);
        return NULL;
    }

    return ast_node_identifier_create(advance(self));
}

// <type> := <@>* <identifier>
static AstNodeType *parse_type(Parser *self) {
    Token *token = peek(self, 0);

    if (token->type != TOKEN_BYTE &&
        token->type != TOKEN_WORD &&
        token->type != TOKEN_IDENTIFIER &&
        token->type != TOKEN_AT) {
        print_token_expected_error(self, 4, TOKEN_WORD, TOKEN_BYTE, TOKEN_IDENTIFIER, TOKEN_AT);
        return NULL;
    }

    int line = token->line;
    int column = token->column;

    int ptr = 0;
    while (match(self, TOKEN_AT)) {
        ptr++;
    }

    AstNodeIdentifier *identifier = NULL;
    token = peek(self, 0);
    switch (token->type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_WORD:
        case TOKEN_BYTE:
            identifier = ast_node_identifier_create(*token);
            advance(self);
            break;
        default:
            print_token_expected_error(self, 4, TOKEN_WORD, TOKEN_BYTE, TOKEN_IDENTIFIER, TOKEN_AT);
            break;
    }

    if (identifier == NULL) {
        return NULL;
    }
    return ast_node_type_create(ptr, identifier, line, column);
}

// <variable> := ( <PUBLIC> | <EXTERN> )? <VOLATILE>? <VAR> <identifier> <:> <type> <;>
static AstNodeVariable *parse_variable(Parser *self) {
    AstNodeVariable *node = ast_node_variable_create();

    Token *token = peek(self, 0);
    node->metadata->start_line = token->line;
    node->metadata->start_column = token->column;

    switch (token->type) {
        case TOKEN_PUBLIC:
            advance(self);
            node->pub = true;
            break;
        case TOKEN_EXTERN:
            advance(self);
            node->ext = true;
            break;
        case TOKEN_VOLATILE:
        case TOKEN_VAR:
            // keep for later
            break;
        default:
            print_token_expected_error(self, 4, TOKEN_PUBLIC, TOKEN_EXTERN, TOKEN_VOLATILE, TOKEN_VAR);
            break;
    }

    if (match(self, TOKEN_VOLATILE)) {
        node->vol = true;
    }

    if (!match(self, TOKEN_VAR)) {
        print_token_expected_error(self, 1, TOKEN_VAR);
    }

    node->name = parse_identifier(self);

    if (!match(self, TOKEN_COLON)) {
        print_token_expected_error(self, 1, TOKEN_COLON);
    }

    node->type = parse_type(self);

    // todo add expr

    if (!match(self, TOKEN_SEMICOLON)) {
        print_token_expected_error(self, 1, TOKEN_SEMICOLON);
        ast_node_variable_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_variable_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <const> := ( <PUBLIC> | <EXTERN> )? <CONST> <IDENTIFIER> <:> <type> <=> <expr> <;>
static AstNodeConst *parse_const(Parser *self) {
    AstNodeConst *node = ast_node_const_create();

    Token *token = peek(self, 0);
    node->metadata->start_line = token->line;
    node->metadata->start_column = token->column;

    switch (token->type) {
        case TOKEN_PUBLIC:
            advance(self);
            node->pub = true;
            break;
        case TOKEN_EXTERN:
            advance(self);
            node->ext = true;
            break;
        default:
            // nothing
            break;
    }

    if (!match(self, TOKEN_CONST)) {
        print_token_expected_error(self, 1, TOKEN_CONST);
    }

    node->name = parse_identifier(self);

    if (!match(self, TOKEN_COLON)) {
        print_token_expected_error(self, 1, TOKEN_COLON);
    }

    node->type = parse_type(self);

    if (!match(self, TOKEN_EQUAL)) {
        print_token_expected_error(self, 1, TOKEN_EQUAL);
    }

    // todo expr

    if (!match(self, TOKEN_SEMICOLON)) {
        print_token_expected_error(self, 1, TOKEN_SEMICOLON);
        ast_node_const_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_const_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <parameter> := <IDENTIFIER> <:> <type>
static AstNodeParameter *parse_parameter(Parser *self) {
    Token *token = peek(self, 0);
    int line = token->line;
    int column = token->column;

    AstNodeIdentifier *identifier = parse_identifier(self);

    if (!match(self, TOKEN_COLON)) {
        print_token_expected_error(self, 1, TOKEN_COLON);
    }

    return ast_node_parameter_create(identifier, parse_type(self), line, column);
}

// <parameters> := ( <parameter> ( <,> <parameter> )* )?
static AstNodeParameters *parse_parameters(Parser *self) {
    // todo allow trailing comma
    AstNodeParameters *node = ast_node_parameters_create();

    Token *token = peek(self, 0);
    node->metadata->start_line = token->line;
    node->metadata->start_column = token->column;

    AstNodeParameter *parameter;
    if (token->type != TOKEN_RPAR) {
        parameter = parse_parameter(self);
        if (parameter != NULL) {
            list_add(node->parameters, parameter);
        }
        while (match(self, TOKEN_COMMA)) {
            parameter = parse_parameter(self);
            if (parameter != NULL) {
                list_add(node->parameters, parameter);
            }
        }
    }

    return node;
}

static AstNodeStmt *parse_stmt(Parser *self);

// <stmts> := <stmts>*
static AstNodeStatements *parse_stmts(Parser *self) {
    AstNodeStatements *node = ast_node_stmts_create();

    bool first_statement = true;
    while (!check(self, TOKEN_EOF) && !check(self, TOKEN_RBRACE)) {
        AstNodeStmt *stmt = parse_stmt(self);
        if (stmt != NULL) {
            if (first_statement) {
                first_statement = false;
                node->metadata->start_line = (*stmt->metadata)->start_line;
                node->metadata->start_column = (*stmt->metadata)->start_column;
            }
            list_add(node->stmts, stmt);
        }
    }

    return node;
}

// <function> != ( <PUBLIC> | <EXTERN> )? <FN> <IDENTIFIER> <(> <parameters> <)> <:> <type> <{> <statements> <}>
static AstNodeFunction *parse_function(Parser *self) {
    AstNodeFunction *node = ast_node_function_create();

    Token *token = peek(self, 0);
    node->metadata->start_line = token->line;
    node->metadata->start_column = token->column;

    switch (token->type) {
        case TOKEN_PUBLIC:
            advance(self);
            node->pub = true;
            break;
        case TOKEN_EXTERN:
            advance(self);
            node->ext = true;
            break;
        default:
            // nothing
            break;
    }

    if (!match(self, TOKEN_FN)) {
        print_token_expected_error(self, 1, TOKEN_FN);
    }

    node->name = parse_identifier(self);

    if (!match(self, TOKEN_LPAR)) {
        print_token_expected_error(self, 1, TOKEN_LPAR);
    }

    node->parameters = parse_parameters(self);

    if (!match(self, TOKEN_RPAR)) {
        print_token_expected_error(self, 2, TOKEN_COMMA, TOKEN_RPAR);
    }

    if (!match(self, TOKEN_COLON)) {
        print_token_expected_error(self, 1, TOKEN_COLON);
    }

    node->type = parse_type(self);

    if (!match(self, TOKEN_LBRACE)) { // todo may move to stmts?
        print_token_expected_error(self, 1, TOKEN_LBRACE);
    }

    node->statements = parse_stmts(self);

    if (!match(self, TOKEN_RBRACE)) {
        print_token_expected_error(self, 1, TOKEN_RBRACE);
        ast_node_function_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_function_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <constStmt> := <CONST> <identifier> <:> <type> <=> <expr> <;>
static AstNodeStmt *parse_stmt_const(Parser *self) {
    AstNodeStmt *node = ast_node_stmt_const_create();

    Token *token = peek(self, 0);
    (*node->metadata)->start_line = token->line;
    (*node->metadata)->start_column = token->column;

    expect(self, TOKEN_CONST);
    node->constStmt->identifier = parse_identifier(self);
    expect(self, TOKEN_COLON);
    node->constStmt->type = parse_type(self);
    expect(self, TOKEN_EQUAL);

    // todo expr

    if (!match(self, TOKEN_SEMICOLON)) {
        print_token_expected_error(self, 1, TOKEN_SEMICOLON);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <varStmt> := <VAR> <identifier> <:> <type> ( <=> <expr> )? <;>
static AstNodeStmt *parse_stmt_var(Parser *self) {
    AstNodeStmt *node = ast_node_stmt_var_create();

    Token *token = peek(self, 0);
    (*node->metadata)->start_line = token->line;
    (*node->metadata)->start_column = token->column;

    expect(self, TOKEN_VAR);
    node->varStmt->identifier = parse_identifier(self);
    expect(self, TOKEN_COLON);
    node->varStmt->type = parse_type(self);

    if (match(self, TOKEN_EQUAL)) {
        // todo expr
    }

    if (!match(self, TOKEN_SEMICOLON)) {
        print_token_expected_error(self, 1, TOKEN_SEMICOLON);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <assignmentStmt> := <identifier> <=> <expr> <;>
static AstNodeStmt *parse_stmt_assignment(Parser *self) {
    AstNodeStmt *node = ast_node_stmt_assignment_create();

    Token *token = peek(self, 0);
    (*node->metadata)->start_line = token->line;
    (*node->metadata)->start_column = token->column;

    node->assignmentStmt->identifier = parse_identifier(self);

    expect(self, TOKEN_EQUAL);

    // todo expr

    if (!match(self, TOKEN_SEMICOLON)) {
        print_token_expected_error(self, 1, TOKEN_SEMICOLON);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <ifStmt> := <IF> <(> <expr> <)> <{> <statements> <}> ( <ELSE> ( <ifStmt> | <{> <statements> <}> ) )?
static AstNodeStmt *parse_stmt_if(Parser *self) {
    AstNodeStmt *node = ast_node_stmt_if_create();

    Token *token = peek(self, 0);
    (*node->metadata)->start_line = token->line;
    (*node->metadata)->start_column = token->column;

    expect(self, TOKEN_IF);
    expect(self, TOKEN_LPAR);

    // todo expr

    expect(self, TOKEN_RPAR);
    expect(self, TOKEN_LBRACE);

    node->ifStmt->true_block = parse_stmts(self);

    expect(self, TOKEN_RBRACE);
    bool rbrace_required = false;

    if (match(self, TOKEN_ELSE)) {
        if (check(self, TOKEN_IF)) {
            node->ifStmt->false_block = ast_node_stmts_create();
            list_add(node->ifStmt->false_block->stmts, parse_stmt_if(self));
        } else if (match(self, TOKEN_LBRACE)) {
            rbrace_required = true;
            node->ifStmt->false_block = parse_stmts(self);
        } else {
            print_token_expected_error(self, 2, TOKEN_IF, TOKEN_LBRACE);
        }
    } else {
        node->ifStmt->false_block = ast_node_stmts_create();
    }

    if (rbrace_required && !match(self, TOKEN_RBRACE)) {
        print_token_expected_error(self, 1, TOKEN_RBRACE);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <whileStmt> := <WHILE> <(> <expr> <)> <{> <statements> <}>
static AstNodeStmt *parse_stmt_while(Parser *self) {
    AstNodeStmt *node = ast_node_stmt_while_create();

    Token *token = peek(self, 0);
    (*node->metadata)->start_line = token->line;
    (*node->metadata)->start_column = token->column;

    expect(self, TOKEN_WHILE);
    expect(self, TOKEN_LPAR);

    // todo expr

    expect(self, TOKEN_RPAR);
    expect(self, TOKEN_LBRACE);

    node->whileStmt->block = parse_stmts(self);

    if (!match(self, TOKEN_RBRACE)) {
        print_token_expected_error(self, 1, TOKEN_RBRACE);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <stmt> := ( <;> | <ifStmt> | <whileStmt> | <assignmentStmt> )
static AstNodeStmt *parse_stmt(Parser *self) {
    switch (peek(self, 0)->type) {
        case TOKEN_SEMICOLON: // just eat it as an empty statement
            advance(self);
            return NULL;
        case TOKEN_CONST:
            return parse_stmt_const(self);
        case TOKEN_VAR:
            return parse_stmt_var(self);
        case TOKEN_IDENTIFIER:
            return parse_stmt_assignment(self);
        case TOKEN_IF:
            return parse_stmt_if(self);
        case TOKEN_WHILE:
            return parse_stmt_while(self);
        default:
            print_expected_error(self, "<statement>");
            return NULL;
    }
}

#pragma endregion

#pragma region public

Parser *parser_create(Lexer *lexer) {
    Parser *parser = memory_alloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->tokens = queue_create(4,
                                  .malloc = cpocl_memory_alloc,
                                  .free = cpocl_memory_free);
    parser->error_recovery = false;
    parser->error_found = false;
    return parser;
}

void parser_destroy(Parser *self) {
    while (!queue_is_empty(self->tokens)) {
        memory_free(queue_dequeue(self->tokens));
    }
    queue_destroy(self->tokens);
    memory_free(self);
}

AstRoot *parser_parse(Parser *self) {
    AstRoot *root = ast_root_create();

    while (peek(self, 0)->type != TOKEN_EOF) {
        while (match(self, TOKEN_SEMICOLON)) {
            // nothing
        }
        if (check_within(self, TOKEN_VAR, 3)) {
            AstNodeVariable *variable = parse_variable(self);
            if (variable != NULL) {
                list_add(root->variables, variable);
            }
        } else if (check_within(self, TOKEN_FN, 2)) {
            AstNodeFunction *function = parse_function(self);
            if (function != NULL) {
                list_add(root->functions, function);
            }
        } else if (check_within(self, TOKEN_CONST, 2)) {
            AstNodeConst *constant = parse_const(self);
            if (constant != NULL) {
                list_add(root->constants, constant);
            }
        } else {
            print_expected_error(self, "constant, variable or function definition");
            advance(self);
        }
    }

    if (self->error_found) {
        ast_root_destroy(root);
        return NULL;
    } else {
        return root;
    }
}

#pragma endregion

