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

#ifndef THM_ANALYSER_H
#define THM_ANALYSER_H

#include "ast.h"

typedef struct Analyser Analyser;

Analyser *analyzer_create();

void analyzer_destroy(Analyser *this);

bool analyzer_analyse(Analyser *this, AstRoot *root);

void analyser_dump_symbol_table(Analyser *this);

#endif //THM_ANALYSER_H
