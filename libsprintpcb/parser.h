//
// Created by Benedikt on 30.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_PARSER_H
#define SPRINTPCB_PARSER_H

#include "errors.h"
#include "token.h"
#include "stringbuilder.h"

#include <stdbool.h>

typedef enum sprint_statement_flags {
    // The statement is the first one after a terminator
    SPRINT_STATEMENT_FLAG_FIRST = 1 << 0,
    // The statement has a value
    SPRINT_STATEMENT_FLAG_VALUE = 1 << 1,
    // The statement has an index
    SPRINT_STATEMENT_FLAG_INDEX = 1 << 2
} sprint_statement_flags;

typedef struct sprint_statement {
    char* name;
    sprint_statement_flags flags;
    int index;
} sprint_statement;

typedef struct sprint_parser {
    sprint_tokenizer* tokenizer;
    sprint_stringbuilder* builder;
} sprint_parser;

#endif //SPRINTPCB_PARSER_H
