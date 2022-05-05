//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "errors.h"

#include <stdbool.h>
#include <stdio.h>

const char* SPRINT_ERROR_NAMES[] = {
        [SPRINT_ERROR_NONE] = "none",
        [SPRINT_ERROR_ASSERTION] = "assertion",
        [SPRINT_ERROR_UNDERFLOW] = "underflow",
        [SPRINT_ERROR_OVERFLOW] = "overflow",
        [SPRINT_ERROR_MEMORY] = "memory",
        [SPRINT_ERROR_IO] = "I/O",
        [SPRINT_ERROR_EOF] = "EOF",
        [SPRINT_ERROR_STATE_INVALID] = "state invalid",
        [SPRINT_ERROR_ARGUMENT_NULL] = "argument null",
        [SPRINT_ERROR_ARGUMENT_RANGE] = "argument range",
        [SPRINT_ERROR_ARGUMENT_FORMAT] = "argument format"
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

