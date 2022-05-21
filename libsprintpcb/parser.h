//
// Created by Benedikt on 30.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_PARSER_H
#define SPRINTPCB_PARSER_H

#include "primitives.h"
#include "elements.h"
#include "list.h"
#include "stringbuilder.h"
#include "token.h"
#include "errors.h"

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
char* sprint_parser_statement_name(sprint_statement* statement);
bool sprint_parser_statement_flags(sprint_statement* statement, bool equal, sprint_statement_flags flags);
int sprint_parser_statement_index(sprint_statement* statement);
sprint_error sprint_parser_statement_destroy(sprint_statement* statement);

typedef struct sprint_parser {
    sprint_tokenizer* tokenizer;
    sprint_stringbuilder* builder;
    sprint_token token;
    bool subsequent;
    bool value;
} sprint_parser;
sprint_parser* sprint_parser_create(sprint_tokenizer* tokenizer);
sprint_error sprint_parser_token(sprint_parser* parser, sprint_token* token);
sprint_error sprint_parser_origin(sprint_parser* parser, sprint_source_origin* origin);
sprint_error sprint_parser_next_statement(sprint_parser* parser, sprint_statement* statement, bool sync);
sprint_error sprint_parser_next_bool(sprint_parser* parser, bool* val);
sprint_error sprint_parser_next_int(sprint_parser* parser, int* val);
sprint_error sprint_parser_next_dist(sprint_parser* parser, sprint_dist* dist);
sprint_error sprint_parser_next_size(sprint_parser* parser, sprint_dist* size);
sprint_error sprint_parser_next_angle(sprint_parser* parser, sprint_angle* angle, sprint_prim_format format);
sprint_error sprint_parser_next_tuple(sprint_parser* parser, sprint_tuple* tuple);
sprint_error sprint_parser_next_str(sprint_parser* parser, char** str);
sprint_error sprint_parser_next_element(sprint_parser* parser, sprint_element* element, bool* salvaged);
/**
 * Destroys the parser and releases its memory and resources.
 * @param parser The parser instance.
 * @param tokenizer Whether to also destroy the tokenizer.
 * @param contents If not-null, the destination to where to store the last token contents.
 * @return The error status.
 */
sprint_error sprint_parser_destroy(sprint_parser* parser, bool tokenizer, char** contents);

#endif //SPRINTPCB_PARSER_H
