//
// Created by Benedikt on 04.05.2022.
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
        [SPRINT_ERROR_MEMORY] = "memory",
        [SPRINT_ERROR_IO] = "I/O",
        [SPRINT_ERROR_EOF] = "EOF",
        [SPRINT_ERROR_STATE_INVALID] = "state invalid",
        [SPRINT_ERROR_ARGUMENT_NULL] = "argument null",
        [SPRINT_ERROR_ARGUMENT_RANGE] = "argument range",
        [SPRINT_ERROR_ARGUMENT_FORMAT] = "argument format",
        [SPRINT_ERROR_PLUGIN_INPUT_MISSING] = "plugin input missing",
        [SPRINT_ERROR_PLUGIN_INPUT_SYNTAX] = "plugin input syntax",
        [SPRINT_ERROR_PLUGIN_FLAGS_MISSING] = "plugin flags missing",
        [SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX] = "plugin flags syntax"
};

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

    // critical, NULL:       " "        " Exiting.\n"   OK
    // critical, "":         " "        " Exiting.\n"   OK
    // false, "":            "\n"       "\n"

    // critical, content:    ": %s\n"   ": %s\nExiting.\n"  OK
    // false, content:       ": %s\n"   ": %s\n"  OK
    // false, NULL:          ": "       ": "

    // if content -> ": %s\n"
    // if critical && !content -> " "
    //

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

void sprint_log(const char* format, ...)
{
    if (format == NULL) return;

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    va_end(args);
}