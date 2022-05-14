//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "token.h"

#include <stdio.h>
#include <stdlib.h>

const char SPRINT_STATEMENT_SEPARATOR = ',';
const char SPRINT_STATEMENT_TERMINATOR = ';';
const char SPRINT_VALUE_SEPARATOR = '=';
const char SPRINT_TUPLE_SEPARATOR = '/';
const char SPRINT_STRING_DELIMITER = '|';
const char* SPRINT_TRUE_VALUE = "true";
const char* SPRINT_FALSE_VALUE = "false";

void sprint_tokenizer_count_internal(sprint_tokenizer* tokenizer, char chr);
sprint_error sprint_tokenizer_read_str_internal(sprint_tokenizer* tokenizer, char* result);
sprint_error sprint_tokenizer_read_file_internal(sprint_tokenizer* tokenizer, char* result);
bool sprint_tokenizer_close_str_internal(sprint_tokenizer* tokenizer);
bool sprint_tokenizer_close_file_internal(sprint_tokenizer* tokenizer);

bool sprint_tokenizer_state_valid(sprint_tokenizer_state state)
{
    return state >= SPRINT_SLICER_STATE_SCANNING && state <= SPRINT_SLICER_STATE_STATEMENT_TERMINATOR;
}

sprint_tokenizer_state sprint_tokenizer_first_state(char first_chr)
{
    return sprint_tokenizer_next_state(SPRINT_SLICER_STATE_SCANNING, first_chr);
}

sprint_tokenizer_state sprint_tokenizer_next_state(sprint_tokenizer_state current_state, char next_chr)
{
    if (!sprint_assert(false, sprint_tokenizer_state_valid(current_state)))
        return SPRINT_SLICER_STATE_INVALID;

    if (current_state == SPRINT_SLICER_STATE_STRING_START || current_state == SPRINT_SLICER_STATE_STRING)
        return next_chr == SPRINT_STRING_DELIMITER || next_chr == '\n' || next_chr == '\r' ?
            SPRINT_SLICER_STATE_STRING_END : SPRINT_SLICER_STATE_STRING;

    if (next_chr >= 'A' && next_chr <= 'Z' || next_chr >= 'a' && next_chr <= 'z' || next_chr == '_')
        return SPRINT_SLICER_STATE_WORD;
    if (next_chr >= '0' && next_chr <= '9' || next_chr == '-' &&
        current_state != SPRINT_SLICER_STATE_NUMBER && current_state != SPRINT_SLICER_STATE_WORD)
        return SPRINT_SLICER_STATE_NUMBER;
    if (next_chr == SPRINT_VALUE_SEPARATOR)
        return SPRINT_SLICER_STATE_VALUE_SEPARATOR;
    if (next_chr == SPRINT_TUPLE_SEPARATOR)
        return SPRINT_SLICER_STATE_TUPLE_SEPARATOR;
    if (next_chr == SPRINT_STATEMENT_SEPARATOR)
        return SPRINT_SLICER_STATE_STATEMENT_SEPARATOR;
    if (next_chr == SPRINT_STATEMENT_TERMINATOR)
        return SPRINT_SLICER_STATE_STATEMENT_TERMINATOR;
    if (next_chr == ' ' || next_chr == '\t' || next_chr == '\n' || next_chr == '\r')
        return SPRINT_SLICER_STATE_SCANNING;

    return SPRINT_SLICER_STATE_INVALID;
}

bool sprint_tokenizer_is_idle(sprint_tokenizer_state state)
{
    return state == SPRINT_SLICER_STATE_SCANNING;
}

bool sprint_tokenizer_is_recorded(sprint_tokenizer_state current_state, sprint_tokenizer_state next_state)
{
    if (!sprint_assert(false, sprint_tokenizer_state_valid(current_state)) ||
        sprint_assert(false, sprint_tokenizer_state_valid(next_state)))
        return false;

    switch (current_state) {
        case SPRINT_SLICER_STATE_INVALID:
        case SPRINT_SLICER_STATE_WORD:
        case SPRINT_SLICER_STATE_NUMBER:
        case SPRINT_SLICER_STATE_STRING:
        case SPRINT_SLICER_STATE_VALUE_SEPARATOR:
        case SPRINT_SLICER_STATE_TUPLE_SEPARATOR:
        case SPRINT_SLICER_STATE_STATEMENT_SEPARATOR:
        case SPRINT_SLICER_STATE_STATEMENT_TERMINATOR:
            return true;

        case SPRINT_SLICER_STATE_SCANNING:
        case SPRINT_SLICER_STATE_STRING_START:
        case SPRINT_SLICER_STATE_STRING_END:
        default:
            sprint_warning_format("unimplemented state: %d", current_state);
            return false;
    }
}

bool sprint_tokenizer_is_complete(sprint_tokenizer_state current_state, sprint_tokenizer_state next_state)
{
    if (!sprint_assert(false, sprint_tokenizer_state_valid(current_state)) ||
        sprint_assert(false, sprint_tokenizer_state_valid(next_state)))
        return true;

    switch (current_state) {
        case SPRINT_SLICER_STATE_SCANNING:
        case SPRINT_SLICER_STATE_STRING_START:
        case SPRINT_SLICER_STATE_STRING:
            return false;

        case SPRINT_SLICER_STATE_WORD:
        case SPRINT_SLICER_STATE_NUMBER:
        case SPRINT_SLICER_STATE_STRING_END:
            return current_state != next_state;

        case SPRINT_SLICER_STATE_INVALID:
        case SPRINT_SLICER_STATE_VALUE_SEPARATOR:
        case SPRINT_SLICER_STATE_TUPLE_SEPARATOR:
        case SPRINT_SLICER_STATE_STATEMENT_SEPARATOR:
        case SPRINT_SLICER_STATE_STATEMENT_TERMINATOR:
        default:
            sprint_warning_format("unimplemented state: %d", current_state);
            return true;
    }
}

sprint_token_type sprint_tokenizer_state_type(sprint_tokenizer_state state)
{
    if (!sprint_assert(false, sprint_tokenizer_state_valid(state)))
        return SPRINT_TOKEN_TYPE_NONE;

    switch (state) {
        case SPRINT_SLICER_STATE_SCANNING:
        case SPRINT_SLICER_STATE_INVALID:
            return SPRINT_TOKEN_TYPE_NONE;
        case SPRINT_SLICER_STATE_WORD:
            return SPRINT_TOKEN_TYPE_WORD;
        case SPRINT_SLICER_STATE_NUMBER:
            return SPRINT_TOKEN_TYPE_NUMBER;
        case SPRINT_SLICER_STATE_STRING_START:
        case SPRINT_SLICER_STATE_STRING:
        case SPRINT_SLICER_STATE_STRING_END:
            return SPRINT_TOKEN_TYPE_STRING;
        case SPRINT_SLICER_STATE_VALUE_SEPARATOR:
            return SPRINT_TOKEN_TYPE_VALUE_SEPARATOR;
        case SPRINT_SLICER_STATE_TUPLE_SEPARATOR:
            return SPRINT_TOKEN_TYPE_TUPLE_SEPARATOR;
        case SPRINT_SLICER_STATE_STATEMENT_SEPARATOR:
            return SPRINT_TOKEN_TYPE_STATEMENT_SEPARATOR;
        case SPRINT_SLICER_STATE_STATEMENT_TERMINATOR:
            return SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR;
        default:
            sprint_throw_format(false, "unimplemented state: %d", state);
            return SPRINT_TOKEN_TYPE_NONE;
    }
}

sprint_tokenizer* sprint_tokenizer_from_str(const char* str, bool free)
{
    if (str == NULL) return NULL;

    // Allocate the tokenizer
    sprint_tokenizer* tokenizer = calloc(1, sizeof(*tokenizer));
    tokenizer->str = str;
    tokenizer->read = sprint_tokenizer_read_str_internal;
    tokenizer->close = free ? sprint_tokenizer_close_str_internal : NULL;

    return tokenizer;
}

sprint_tokenizer* sprint_tokenizer_from_file(const char* path)
{
    if (path == NULL) return NULL;

    // Try to open the file
    FILE* file = fopen(path, "r");
    if (file == NULL)
        return NULL;

    // Allocate the tokenizer
    sprint_tokenizer* tokenizer = calloc(1, sizeof(*tokenizer));
    tokenizer->file = file;
    tokenizer->read = sprint_tokenizer_read_file_internal;
    tokenizer->close = sprint_tokenizer_close_file_internal;
    tokenizer->origin.source = path;

    return tokenizer;
}

sprint_error sprint_tokenizer_destroy(sprint_tokenizer* tokenizer)
{
    if (tokenizer == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Close the resource if possible
    bool success = true;
    if (tokenizer->close != NULL) {
        success = tokenizer->close(tokenizer);
        tokenizer->close = NULL;
    }

    // Free the memory
    tokenizer->read = NULL;
    free(tokenizer);
    return success ? SPRINT_ERROR_NONE : SPRINT_ERROR_IO;
}

void sprint_tokenizer_count_internal(sprint_tokenizer* tokenizer, char chr)
{
    if (tokenizer == NULL) return;

    // Determine, if there has been a line-ending and update the last state accordingly
    bool current_cr = chr == '\r', current_lf = chr == '\n';
    tokenizer->last_cr |= current_cr;

    // Decide, whether to update the position or line
    if ((current_cr || current_lf) && !(current_lf & tokenizer->last_cr)) {
        tokenizer->origin.line++;
        tokenizer->origin.pos = 0;
    } else
        tokenizer->origin.pos++;

    // Clear the last carriage return flag, if it is on and this is not one
    if (!current_cr & tokenizer->last_cr)
        tokenizer->last_cr = false;
}

sprint_error sprint_tokenizer_read_str_internal(sprint_tokenizer* tokenizer, char* result)
{
    if (tokenizer == NULL || result == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (tokenizer->str == NULL) return SPRINT_ERROR_STATE_INVALID;

    // Read the character and check, if the EOF is reached
    char chr = *tokenizer->str;
    if (chr == 0)
        return SPRINT_ERROR_EOF;

    // Increment the pointer
    tokenizer->str++;

    // Count and store the character
    sprint_tokenizer_count_internal(tokenizer, chr);
    *result = chr;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_tokenizer_read_file_internal(sprint_tokenizer* tokenizer, char* result)
{
    if (tokenizer == NULL || result == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (tokenizer->file == NULL) return SPRINT_ERROR_STATE_INVALID;

    // Read the character and check, if the EOF is reached
    int raw_chr = fgetc(tokenizer->file);
    if (raw_chr == EOF)
        return SPRINT_ERROR_EOF;
    char chr = (char) raw_chr;

    // Count and store the character
    sprint_tokenizer_count_internal(tokenizer, chr);
    *result = chr;
    return SPRINT_ERROR_NONE;
}

bool sprint_tokenizer_close_str_internal(sprint_tokenizer* tokenizer)
{
    if (tokenizer->str != NULL) {
        free((void *) tokenizer->str);
        tokenizer->str = NULL;
    }
    return true;
}

bool sprint_tokenizer_close_file_internal(sprint_tokenizer* tokenizer)
{
    bool success = true;
    if (tokenizer->file != NULL) {
        success = fclose(tokenizer->file) == 0;
        tokenizer->file = NULL;
    }
    return success;
}
