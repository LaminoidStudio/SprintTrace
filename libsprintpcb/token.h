//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_TOKEN_H
#define SPRINTPCB_TOKEN_H

#include "errors.h"

#include <stdio.h>
#include <stdbool.h>

typedef enum sprint_tokenizer_state {
    SPRINT_SLICER_STATE_SCANNING,
    SPRINT_SLICER_STATE_IDENTIFIER,
    SPRINT_SLICER_STATE_VALUE_START,
    SPRINT_SLICER_STATE_VALUE

} sprint_tokenizer_state;

extern const char SPRINT_STATEMENT_SEPARATOR;
extern const char SPRINT_STATEMENT_TERMINATOR;
extern const char SPRINT_VALUE_SEPARATOR;
extern const char SPRINT_TUPLE_SEPARATOR;
extern const char SPRINT_STRING_DELIMITER;
extern const char* SPRINT_TRUE_VALUE;
extern const char* SPRINT_FALSE_VALUE;

typedef struct sprint_tokenizer sprint_tokenizer;

typedef struct sprint_source_origin {
    int line;
    int pos;
    const char* source;
} sprint_source_origin;

typedef enum sprint_token_type {
    SPRINT_TOKEN_TYPE_WORD,
    SPRINT_TOKEN_TYPE_NUMBER,
    SPRINT_TOKEN_TYPE_STRING,
    SPRINT_TOKEN_TYPE_VALUE_SEPARATOR,
    SPRINT_TOKEN_TYPE_TUPLE_SEPARATOR,
    SPRINT_TOKEN_TYPE_STATEMENT_SEPARATOR,
    SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR
} sprint_token_type;

typedef struct sprint_token {
    sprint_token_type type;
    sprint_source_origin origin;
    const char* value;
} sprint_token;

struct sprint_tokenizer {
    sprint_source_origin origin;
    bool last_cr;
    sprint_error (*read)(sprint_tokenizer* tokenizer, char* result);
    bool (*close)(sprint_tokenizer* tokenizer);
    union {
        const char* str;
        FILE* file;
    };
};

sprint_tokenizer* sprint_tokenizer_from_str(const char* str, bool free);
sprint_tokenizer* sprint_tokenizer_from_file(const char* path);
sprint_error sprint_tokenizer_destroy(sprint_tokenizer* tokenizer);

#endif //SPRINTPCB_TOKEN_H
