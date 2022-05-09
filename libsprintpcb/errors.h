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
#include <stdarg.h>

typedef enum sprint_error {
    SPRINT_ERROR_NONE,
    SPRINT_ERROR_INTERNAL,
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
    SPRINT_ERROR_PLUGIN_INPUT_MISSING,
    SPRINT_ERROR_PLUGIN_INPUT_SYNTAX,
    SPRINT_ERROR_PLUGIN_FLAGS_MISSING,
    SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX,
    SPRINT_ERROR_NOT_ASCII,
    SPRINT_ERROR_SYNTAX// fixme
} sprint_error;
extern const char* SPRINT_ERROR_NAMES[];

const char* sprint_error_string(sprint_error error);
bool sprint_error_print(sprint_error error, FILE* stream, bool capitalized);

void sprint_debug_internal(const char* file, int line, const char* context);
void sprint_warning_internal(const char* file, int line, const char* context);
bool sprint_error_internal(sprint_error error, bool critical, const char* file, int line, const char* context);
void sprint_log(const char* format, ...);
#ifndef NDEBUG
#define sprint_debug(what) sprint_debug_internal(__FILE__, __LINE__, (what))
#define sprint_debug_format() sprint_debug(NULL); sprint_log
#else
#define sprint_debug(what) ((void)0)
#define sprint_debug_format() if (false) sprint_log
#endif
#define sprint_warn(what) sprint_warning_internal(__FILE__, __LINE__, (what))
#define sprint_warn_format() sprint_warn(NULL); sprint_log
#define sprint_require(error) sprint_error_internal((error), true, __FILE__, __LINE__, #error)
#define sprint_check(error) sprint_error_internal((error), false, __FILE__, __LINE__, #error)
#define sprint_throw(critical, what) sprint_error_internal(SPRINT_ERROR_INTERNAL, critical, __FILE__, __LINE__, what)
#define sprint_throw_format(critical) sprint_throw((critical), NULL); sprint_log
#define sprint_assert(critical, success) sprint_error_internal((success) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ASSERTION, \
                                                                (critical), __FILE__, __LINE__, #success)
#define sprint_chain(result, error) (((result) != SPRINT_ERROR_NONE) ? false : \
                                        sprint_error_internal(((result) = (error)), false, __FILE__, __LINE__, #error))
#endif //LIBSPRINTPCB_ERRORS_H
