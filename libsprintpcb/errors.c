//
// libsprintpcb: error handling and tracing
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "errors.h"
#include "plugin.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

const char* SPRINT_ERROR_NAMES[] = {
        [SPRINT_ERROR_NONE] = "none",
        [SPRINT_ERROR_INTERNAL] = "internal",
        [SPRINT_ERROR_ASSERTION] = "assertion",
        [SPRINT_ERROR_UNDERFLOW] = "underflow",
        [SPRINT_ERROR_OVERFLOW] = "overflow",
        [SPRINT_ERROR_RECURSION] = "recursion",
        [SPRINT_ERROR_MEMORY] = "memory",
        [SPRINT_ERROR_IO] = "input/output",
        [SPRINT_ERROR_EOF] = "end of file",
        [SPRINT_ERROR_EOS] = "end of statement",
        [SPRINT_ERROR_EOE] = "end of element",
        [SPRINT_ERROR_TRUNCATED] = "truncated",
        [SPRINT_ERROR_SYNTAX] = "syntax",
        [SPRINT_ERROR_STATE_INVALID] = "state invalid",
        [SPRINT_ERROR_ARGUMENT_NULL] = "argument null",
        [SPRINT_ERROR_ARGUMENT_RANGE] = "argument range",
        [SPRINT_ERROR_ARGUMENT_FORMAT] = "argument format",
        [SPRINT_ERROR_ARGUMENT_INCOMPLETE] = "argument incomplete",
        [SPRINT_ERROR_PLUGIN_INPUT_MISSING] = "plugin input missing",
        [SPRINT_ERROR_PLUGIN_INPUT_SYNTAX] = "plugin input syntax",
        [SPRINT_ERROR_PLUGIN_FLAGS_MISSING] = "plugin flags missing",
        [SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX] = "plugin flags syntax"
};

sprint_error sprint_rethrow(sprint_error error)
{
    switch (error) {
        case SPRINT_ERROR_NONE:
        case SPRINT_ERROR_INTERNAL:
        case SPRINT_ERROR_ASSERTION:
        case SPRINT_ERROR_UNDERFLOW:
        case SPRINT_ERROR_OVERFLOW:
        case SPRINT_ERROR_RECURSION:
        case SPRINT_ERROR_MEMORY:
        case SPRINT_ERROR_IO:
        case SPRINT_ERROR_EOF:
        case SPRINT_ERROR_EOS:
        case SPRINT_ERROR_EOE:
        case SPRINT_ERROR_TRUNCATED:
        case SPRINT_ERROR_SYNTAX:
        case SPRINT_ERROR_STATE_INVALID:
        case SPRINT_ERROR_PLUGIN_INPUT_MISSING:
        case SPRINT_ERROR_PLUGIN_INPUT_SYNTAX:
        case SPRINT_ERROR_PLUGIN_FLAGS_MISSING:
        case SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX:
            return error;

        case SPRINT_ERROR_ARGUMENT_NULL:
        case SPRINT_ERROR_ARGUMENT_RANGE:
        case SPRINT_ERROR_ARGUMENT_FORMAT:
        case SPRINT_ERROR_ARGUMENT_INCOMPLETE:
            return SPRINT_ERROR_INTERNAL;

        default:
            sprint_warning_format("unknown error %d rethrown", error);
            return error;
    }
}

const char* sprint_error_string(sprint_error error)
{
    const char* str = "unknown";
    if (error > 0 && error < sizeof(SPRINT_ERROR_NAMES) / sizeof(char*))
        str = SPRINT_ERROR_NAMES[error];
    return str;
}

bool sprint_error_print(sprint_error error, FILE* stream, bool capitalized)
{
    if (stream == NULL) return false;

    const char* str = sprint_error_string(error);
    char firstChr = *str;
    if (firstChr == 0) return false;
    if (capitalized && firstChr >= 'a' && firstChr <= 'z') {
        firstChr -= 'a';
        firstChr += 'A';
    }
    str++;

    if (fputc(firstChr, stream) == EOF) return false;
    return fputs(str, stream) != EOF;
}

void sprint_debug_internal(const char* file, int line, const char* context)
{
    const char* state = SPRINT_PLUGIN_STATE_NAMES[sprint_plugin_get_state()];
    fprintf(stderr, "Debug [%s, %s:%d]", state, file == NULL ? "" : file, line);

    if (context == NULL || *context != 0)
        fputs(": ", stderr);

    if (context != NULL) {
        fputs(context, stderr);
        fputc('\n', stderr);
    }
}

void sprint_warning_internal(const char* file, int line, const char* context)
{
    const char* state = SPRINT_PLUGIN_STATE_NAMES[sprint_plugin_get_state()];
    fprintf(stderr, "Warning [%s, %s:%d]", state, file == NULL ? "" : file, line);

    if (context == NULL || *context != 0)
        fputs(": ", stderr);

    if (context != NULL) {
        fputs(context, stderr);
        fputc('\n', stderr);
    }
}

bool sprint_error_internal(sprint_error error, bool critical, const char* file, int line, const char* context)
{
    if (error == SPRINT_ERROR_NONE) return true;

    if (critical)
        fputs("Critical ", stderr);

    const char* state = SPRINT_PLUGIN_STATE_NAMES[sprint_plugin_get_state()];
    sprint_error_print(error, stderr, !critical);
    fprintf(stderr, " error [%s, %s:%d]", state, file == NULL ? "" : file, line);

    if (context != NULL && *context != 0)
        fprintf(stderr, ": %s\n", context);
    else if (critical)
        fputc(' ', stderr);
    else if (context == NULL)
        fputs(": ", stderr);
    else
        fputc('\n', stderr);

    if (critical)
    {
        fputs("Exiting.\n", stderr);
        exit(sprint_plugin_get_exit_code());
    }

    return false;
}

bool sprint_log(const char* what)
{
    if (what == NULL) return false;

    bool success = true;
    success &= fputs(what, stderr) != EOF;
    success &= fputc('\n', stderr) != EOF;
    return success;
}

bool sprint_log_format(const char* format, ...)
{
    if (format == NULL) return false;

    bool success = true;
    va_list args;
    va_start(args, format);
    success &= vfprintf(stderr, format, args) != EOF;
    success &= fputc('\n', stderr) != EOF;
    va_end(args);
    return success;
}