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

typedef enum sprint_token_type {
    SPRINT_TOKEN_TYPE_
} sprint_token_type;

typedef struct sprint_token {
    sprint_token_type type;
    char* value;
} sprint_token;

struct sprint_tokenizer {
    int line;
    int pos;
    bool last_cr;
    sprint_error (*read)(sprint_tokenizer * tokenizer, char* result);
    union {
        const char* str;
        FILE* file;
    };
};

#endif //SPRINTPCB_TOKEN_H
