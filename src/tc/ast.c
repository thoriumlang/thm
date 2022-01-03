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
#include "ast.h"
#include "lexer.h"

AstRoot *ast_root_create() {
    AstRoot *node = malloc(sizeof(AstRoot));
    node->variables = list_create();
    node->functions = list_create();
    return node;
}

void ast_root_destroy(AstRoot *this) {
    for (size_t i = 0; i < list_size(this->variables); i++) {
        ast_node_variable_destroy(list_get(this->variables, i));
    }
    list_destroy(this->variables);

    for (size_t i = 0; i < list_size(this->functions); i++) {
        ast_node_function_destroy(list_get(this->functions, i));
    }
    list_destroy(this->functions);
    free(this);
}

AstNodeVariable *ast_node_variable_create() {
    AstNodeVariable *node = malloc(sizeof(AstNodeVariable));
    node->pub = false;
    node->ext = false;
    node->vol = false;
    return node;
}

void ast_node_variable_destroy(AstNodeVariable *this) {
    if (this->name != NULL) {
        ast_node_identifier_destroy(this->name);
    }
    if (this->type != NULL) {
        ast_node_type_destroy(this->type);
    }
    free(this);
}

AstNodeParameters *ast_node_parameters_create() {
    AstNodeParameters *node = malloc(sizeof(AstNodeParameters));
    node->parameters = list_create();
    return node;
}

void ast_node_parameters_destroy(AstNodeParameters *this) {
    if (this->parameters != NULL) {
        list_destroy(this->parameters);
    }
    free(this);
}

AstNodeIdentifier *ast_node_identifier_create(Token token) {
    AstNodeIdentifier *node = malloc(sizeof(AstNodeIdentifier));
    node->name = malloc(token.length * sizeof(char) + 1);
    memcpy(node->name, token.start, token.length);
    node->name[token.length] = 0;
    return node;
}

void ast_node_identifier_destroy(AstNodeIdentifier *this) {
    if (this->name != NULL) {
        free(this->name);
    }
    free(this);
}

AstNodeType *ast_node_type_create(int ptr, AstNodeIdentifier *identifier) {
    if (identifier == NULL) {
        return NULL;
    }

    AstNodeType *node = malloc(sizeof(AstNodeType));
    node->ptr = ptr;
    node->identifier = identifier;

    return node;
}

void ast_node_type_destroy(AstNodeType *this) {
    if (this->identifier != NULL) {
        ast_node_identifier_destroy(this->identifier);
    }
    free(this);
}

AstNodeFunction *ast_node_function_create() {
    AstNodeFunction *node = malloc(sizeof(AstNodeFunction));
    node->pub = false;
    node->ext = false;
    return node;
}

void ast_node_function_destroy(AstNodeFunction *this) {
    if (this->name != NULL) {
        ast_node_identifier_destroy(this->name);
    }
    if (this->type != NULL) {
        ast_node_type_destroy(this->type);
    }
    if (this->parameters != NULL) {
        ast_node_parameters_destroy(this->parameters);
    }
    free(this);
}

AstNodeParameter *ast_node_parameter_create(AstNodeIdentifier *identifier, AstNodeType *type) {
    if (identifier != NULL && type != NULL) {
        AstNodeParameter *node = malloc(sizeof(AstNodeParameter));
        node->name = identifier;
        node->type = type;
        return node;
    }
    if (identifier != NULL) {
        ast_node_identifier_destroy(identifier);
    }
    if (type != NULL) {
        ast_node_type_destroy(type);
    }
    return NULL;
}

