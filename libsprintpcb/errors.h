//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef LIBSPRINTPCB_ERRORS_H
#define LIBSPRINTPCB_ERRORS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum sprint_error {
    SPRINT_ERROR_NONE,
    SPRINT_ERROR_ASSERTION,
    SPRINT_ERROR_UNDERFLOW,
    SPRINT_ERROR_OVERFLOW,
    SPRINT_ERROR_MEMORY,
    SPRINT_ERROR_IO,
    SPRINT_ERROR_EOF,
    SPRINT_ERROR_STATE_INVALID,
    SPRINT_ERROR_ARGUMENT_NULL,
    SPRINT_ERROR_ARGUMENT_RANGE,
    SPRINT_ERROR_ARGUMENT_FORMAT,
    SPRINT_ERROR_NOT_ASCII,
    SPRINT_ERROR_SYNTAX// fixme
} sprint_error;
extern const char* SPRINT_ERROR_NAMES[];

const char* sprint_error_string(sprint_error error);
bool sprint_error_print(sprint_error error, FILE* stream, bool capitalized);

bool sprint_error_internal(sprint_error error, bool critical, const char* file, int line, const char* context);
#define sprint_require(error) sprint_error_internal((error), true, __FILE__, __LINE__, #error)
#define sprint_check(error) sprint_error_internal((error), false, __FILE__, __LINE__, #error)
#define sprint_assert(critical, success) sprint_error_internal((success) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ASSERTION, \
                                                        (critical), __FILE__, __LINE__, #success)

#endif //LIBSPRINTPCB_ERRORS_H
