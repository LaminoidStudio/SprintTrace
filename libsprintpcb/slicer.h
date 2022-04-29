//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_SLICER_H
#define SPRINTPCB_SLICER_H

#include "errors.h"

#include <stdio.h>
#include <stdbool.h>

extern const char SPRINT_STATEMENT_SEPARATOR;
extern const char SPRINT_STATEMENT_TERMINATOR;
extern const char SPRINT_VALUE_SEPARATOR;
extern const char SPRINT_TUPLE_SEPARATOR;
extern const char SPRINT_STRING_DELIMITER;
extern const char* SPRINT_TRUE_VALUE;
extern const char* SPRINT_FALSE_VALUE;

typedef struct sprint_slicer sprint_slicer;

struct sprint_slicer {
    int line;
    int pos;
    sprint_error (*read)(sprint_slicer* slicer, char* result);
    union {
        char* str;
        FILE* file;
    };
};

#endif //SPRINTPCB_SLICER_H
