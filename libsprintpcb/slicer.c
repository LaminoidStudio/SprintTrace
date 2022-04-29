//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "slicer.h"

#include <stdio.h>
#include <stdlib.h>

const char SPRINT_STATEMENT_SEPARATOR = ',';
const char SPRINT_STATEMENT_TERMINATOR = ';';
const char SPRINT_VALUE_SEPARATOR = '=';
const char SPRINT_TUPLE_SEPARATOR = '/';
const char SPRINT_STRING_DELIMITER = '|';
const char* SPRINT_TRUE_VALUE = "true";
const char* SPRINT_FALSE_VALUE = "false";

sprint_error sprint_slicer_read_str_internal(sprint_slicer* slicer, char* result);
sprint_error sprint_slicer_read_file_internal(sprint_slicer* slicer, char* result);
void sprint_slicer_count_internal(sprint_slicer* slicer, char chr);

sprint_slicer* sprint_slicer_create_str(const char* str)
{
    if (str == NULL) return NULL;

    // Allocate the slicer
    sprint_slicer* slicer = calloc(1, sizeof(*slicer));
    slicer->str = str;
    slicer->read = sprint_slicer_read_str_internal;

    return slicer;
}

sprint_slicer* sprint_slicer_create_file(FILE* file)
{
    if (file == NULL) return NULL;

    // Allocate the slicer
    sprint_slicer* slicer = calloc(1, sizeof(*slicer));
    slicer->file = file;
    slicer->read = sprint_slicer_read_file_internal;

    return slicer;
}

void sprint_slicer_count_internal(sprint_slicer* slicer, char chr)
{
    if (slicer == NULL) return;

    // Determine, if there has been a line-ending and update the last state accordingly
    bool current_cr = chr == '\r', current_lf = chr == '\n';
    slicer->last_cr |= current_cr;

    // Decide, whether to update the position or line
    if ((current_cr || current_lf) && !(current_lf & slicer->last_cr)) {
        slicer->line++;
        slicer->pos = 0;
    } else
        slicer->pos++;

    // Clear the last carriage return flag, if it is on and this is not one
    if (!current_cr & slicer->last_cr)
        slicer->last_cr = false;
}

sprint_error sprint_slicer_read_str_internal(sprint_slicer* slicer, char* result)
{
    if (slicer == NULL || result == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (slicer->str == NULL) return SPRINT_ERROR_STATE_INVALID;

    // Read the character and check, if the EOF is reached
    char chr = *slicer->str;
    if (chr == 0)
        return SPRINT_ERROR_EOF;

    // Increment the pointer
    slicer->str++;

    // Count and store the character
    sprint_slicer_count_internal(slicer, chr);
    *result = chr;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_slicer_read_file_internal(sprint_slicer* slicer, char* result)
{
    if (slicer == NULL || result == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (slicer->file == NULL) return SPRINT_ERROR_STATE_INVALID;

    // Read the character and check, if the EOF is reached
    int raw_chr = fgetc(slicer->file);
    if (raw_chr == EOF)
        return SPRINT_ERROR_EOF;
    char chr = (char) raw_chr;

    // Count and store the character
    sprint_slicer_count_internal(slicer, chr);
    *result = chr;
    return SPRINT_ERROR_NONE;
}
