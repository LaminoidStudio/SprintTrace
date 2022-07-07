//
// SprintTrace: error handling and tracing
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTTRACE_ERRORS_H
#define SPRINTTRACE_ERRORS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

typedef enum sprint_error {
    SPRINT_ERROR_NONE,
    SPRINT_ERROR_INTERNAL,
    SPRINT_ERROR_ASSERTION,
    SPRINT_ERROR_UNDERFLOW,
    SPRINT_ERROR_OVERFLOW,
    SPRINT_ERROR_RECURSION,
    SPRINT_ERROR_MEMORY,
    SPRINT_ERROR_IO,
    SPRINT_ERROR_EOF,
    SPRINT_ERROR_EOS,
    SPRINT_ERROR_EOE,
    SPRINT_ERROR_TRUNCATED,
    SPRINT_ERROR_SYNTAX,
    SPRINT_ERROR_STATE_INVALID,
    SPRINT_ERROR_ARGUMENT_NULL,
    SPRINT_ERROR_ARGUMENT_RANGE,
    SPRINT_ERROR_ARGUMENT_FORMAT,
    SPRINT_ERROR_ARGUMENT_INCOMPLETE,
    SPRINT_ERROR_PLUGIN_INPUT_MISSING,
    SPRINT_ERROR_PLUGIN_INPUT_SYNTAX,
    SPRINT_ERROR_PLUGIN_FLAGS_MISSING,
    SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX
} sprint_error;
extern const char* SPRINT_ERROR_NAMES[];

const char* sprint_error_string(sprint_error error);
bool sprint_error_print(sprint_error error, FILE* stream, bool capitalized);

void sprint_debug_internal(const char* file, int line, const char* context);
void sprint_warning_internal(const char* file, int line, const char* context);
bool sprint_error_internal(sprint_error error, bool critical, const char* file, int line, const char* context);
sprint_error sprint_rethrow(sprint_error error);
bool sprint_log(const char* what);
bool sprint_log_format(const char* format, ...);
#ifndef NDEBUG
#define SPRINT_SOURCE __FILE__
#define sprint_debug(what) sprint_debug_internal(SPRINT_SOURCE, __LINE__, (what))
#define sprint_debug_format(format, ...) \
                    do { sprint_debug(NULL); sprint_log_format((format), ##__VA_ARGS__); } while (false)
#else
#define SPRINT_SOURCE (strchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : \
                                            (strrchr(__FILE__, '/') != NULL ? strrchr(__FILE__, '/') + 1 : __FILE__))
#define sprint_debug(what) ((void)0)
#define sprint_debug_format(format, ...) ((void)0)
#endif
#define sprint_warning(what) sprint_warning_internal(SPRINT_SOURCE, __LINE__, (what))
#define sprint_warning_format(format, ...) \
                    do { sprint_warning(NULL); sprint_log_format((format), ##__VA_ARGS__); } while (false)
#define sprint_require(error) sprint_error_internal((error), true, SPRINT_SOURCE, __LINE__, #error)
#define sprint_check(error) sprint_error_internal((error), false, SPRINT_SOURCE, __LINE__, #error)
#define sprint_throw(critical, what) \
                    sprint_error_internal(SPRINT_ERROR_INTERNAL, critical, SPRINT_SOURCE, __LINE__, what)
#define sprint_throw_format(critical, format, ...) \
                    do { sprint_throw((critical), NULL); sprint_log_format((format), ##__VA_ARGS__); } while (false)
#define sprint_assert(critical, success) sprint_error_internal((success) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ASSERTION, \
                                                                (critical), SPRINT_SOURCE, __LINE__, #success)
#define sprint_chain(result, error) (((result) != SPRINT_ERROR_NONE) ? false : \
                            sprint_error_internal(((result) = (error)), false, SPRINT_SOURCE, __LINE__, #error))
#endif //SPRINTTRACE_ERRORS_H
