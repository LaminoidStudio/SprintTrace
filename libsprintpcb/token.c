//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "token.h"
#include "stringbuilder.h"
#include "errors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

const char* SPRINT_TOKENIZER_STATE_NAMES[] = {
        [SPRINT_SLICER_STATE_SCANNING] = "scanning",
        [SPRINT_SLICER_STATE_INVALID] = "invalid",
        [SPRINT_SLICER_STATE_COMMENT] = "comment",
        [SPRINT_SLICER_STATE_WORD] = "word",
        [SPRINT_SLICER_STATE_NUMBER] = "number",
        [SPRINT_SLICER_STATE_STRING_START] = "string start",
        [SPRINT_SLICER_STATE_STRING] = "string",
        [SPRINT_SLICER_STATE_STRING_END] = "string end",
        [SPRINT_SLICER_STATE_VALUE_SEPARATOR] = "value separator",
        [SPRINT_SLICER_STATE_TUPLE_SEPARATOR] = "tuple separator",
        [SPRINT_SLICER_STATE_STATEMENT_SEPARATOR] = "statement separator",
        [SPRINT_SLICER_STATE_STATEMENT_TERMINATOR] = "statement terminator"
};

const char* SPRINT_TOKEN_TYPE_NAMES[] = {
        [SPRINT_TOKEN_TYPE_NONE] = "none",
        [SPRINT_TOKEN_TYPE_INVALID] = "invalid",
        [SPRINT_TOKEN_TYPE_WORD] = "word",
        [SPRINT_TOKEN_TYPE_NUMBER] = "number",
        [SPRINT_TOKEN_TYPE_STRING] = "string",
        [SPRINT_TOKEN_TYPE_VALUE_SEPARATOR] = "value separator",
        [SPRINT_TOKEN_TYPE_TUPLE_SEPARATOR] = "tuple separator",
        [SPRINT_TOKEN_TYPE_STATEMENT_SEPARATOR] = "statement separator",
        [SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR] = "terminator"
};

const char SPRINT_COMMENT_PREFIX = '#';
const char SPRINT_STATEMENT_SEPARATOR = ',';
const char SPRINT_STATEMENT_TERMINATOR = ';';
const char SPRINT_VALUE_SEPARATOR = '=';
const char SPRINT_TUPLE_SEPARATOR = '/';
const char SPRINT_STRING_DELIMITER = '|';
const char* SPRINT_TRUE_VALUE = "true";
const char* SPRINT_FALSE_VALUE = "false";

static void sprint_tokenizer_count_internal(sprint_tokenizer* tokenizer, char chr);
static bool sprint_tokenizer_read_str_internal(sprint_tokenizer* tokenizer);
static bool sprint_tokenizer_read_file_internal(sprint_tokenizer* tokenizer);
static bool sprint_tokenizer_close_str_internal(sprint_tokenizer* tokenizer);
static bool sprint_tokenizer_close_file_internal(sprint_tokenizer* tokenizer);

bool sprint_tokenizer_state_valid(sprint_tokenizer_state state)
{
    return state >= SPRINT_SLICER_STATE_SCANNING && state <= SPRINT_SLICER_STATE_STATEMENT_TERMINATOR;
}

sprint_tokenizer_state sprint_tokenizer_state_first(char first_chr)
{
    return sprint_tokenizer_state_next(SPRINT_SLICER_STATE_SCANNING, first_chr);
}

sprint_tokenizer_state sprint_tokenizer_state_next(sprint_tokenizer_state current_state, char next_chr)
{
    if (!sprint_assert(false, sprint_tokenizer_state_valid(current_state)))
        return SPRINT_SLICER_STATE_INVALID;

    // Handle enclosed strings
    if (current_state == SPRINT_SLICER_STATE_STRING_START || current_state == SPRINT_SLICER_STATE_STRING)
        return next_chr == SPRINT_STRING_DELIMITER || next_chr == '\n' || next_chr == '\r' ?
            SPRINT_SLICER_STATE_STRING_END : SPRINT_SLICER_STATE_STRING;

    // Handle comments that end at the end of the line
    if (current_state == SPRINT_SLICER_STATE_COMMENT && next_chr != '\n' && next_chr != '\r' ||
        next_chr == SPRINT_COMMENT_PREFIX)
        return SPRINT_SLICER_STATE_COMMENT;

    // Handle scanning through white-space
    if (next_chr == ' ' || next_chr == '\t' || next_chr == '\n' || next_chr == '\r')
        return SPRINT_SLICER_STATE_SCANNING;

    // Handle words
    if (next_chr >= 'A' && next_chr <= 'Z' || next_chr >= 'a' && next_chr <= 'z' || next_chr == '_')
        return SPRINT_SLICER_STATE_WORD;

    // Handle numbers
    if (next_chr >= '0' && next_chr <= '9' || next_chr == '-' &&
        current_state != SPRINT_SLICER_STATE_NUMBER && current_state != SPRINT_SLICER_STATE_WORD)
        return SPRINT_SLICER_STATE_NUMBER;

    // Handle the single digit separators and terminator
    if (next_chr == SPRINT_VALUE_SEPARATOR)
        return SPRINT_SLICER_STATE_VALUE_SEPARATOR;
    if (next_chr == SPRINT_TUPLE_SEPARATOR)
        return SPRINT_SLICER_STATE_TUPLE_SEPARATOR;
    if (next_chr == SPRINT_STATEMENT_SEPARATOR)
        return SPRINT_SLICER_STATE_STATEMENT_SEPARATOR;
    if (next_chr == SPRINT_STATEMENT_TERMINATOR)
        return SPRINT_SLICER_STATE_STATEMENT_TERMINATOR;

    return SPRINT_SLICER_STATE_INVALID;
}

bool sprint_tokenizer_state_idle(sprint_tokenizer_state state)
{
    return state == SPRINT_SLICER_STATE_SCANNING;
}

bool sprint_tokenizer_state_recorded(sprint_tokenizer_state state)
{
    if (!sprint_assert(false, sprint_tokenizer_state_valid(state)))
        return false;

    switch (state) {
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
        case SPRINT_SLICER_STATE_COMMENT:
        case SPRINT_SLICER_STATE_STRING_START:
        case SPRINT_SLICER_STATE_STRING_END:
            return false;

        default:
            sprint_warning_format("unimplemented state: %d", state);
            return false;
    }
}

bool sprint_tokenizer_state_complete(sprint_tokenizer_state current_state, sprint_tokenizer_state next_state)
{
    if (!sprint_assert(false, sprint_tokenizer_state_valid(current_state)) ||
        !sprint_assert(false, sprint_tokenizer_state_valid(next_state)))
        return true;

    switch (current_state) {
        case SPRINT_SLICER_STATE_SCANNING:
        case SPRINT_SLICER_STATE_COMMENT:
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
            return true;

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
        case SPRINT_SLICER_STATE_COMMENT:
            return SPRINT_TOKEN_TYPE_NONE;
        case SPRINT_SLICER_STATE_INVALID:
            return SPRINT_TOKEN_TYPE_INVALID;
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

sprint_error sprint_token_word(sprint_token* token, sprint_stringbuilder* builder, char** word)
{
    if (token == NULL || builder == NULL || word == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (token->type != SPRINT_TOKEN_TYPE_WORD) return SPRINT_ERROR_ARGUMENT_FORMAT;
    if (sprint_stringbuilder_count(builder) < 1) return SPRINT_ERROR_ARGUMENT_INCOMPLETE;

    // Allocate a dynamic buffer
    size_t size = sprint_stringbuilder_count(builder) + 1;
    *word = malloc(size);
    if (*word == NULL)
        return SPRINT_ERROR_MEMORY;

    // Copy the value
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_stringbuilder_output(builder, *word, size));
    return sprint_rethrow(error);
}

sprint_error sprint_token_bool(sprint_token* token, sprint_stringbuilder* builder, bool* val)
{
    if (token == NULL || builder == NULL || val == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (token->type != SPRINT_TOKEN_TYPE_WORD) return SPRINT_ERROR_ARGUMENT_FORMAT;
    if (sprint_stringbuilder_count(builder) < 1) return SPRINT_ERROR_ARGUMENT_INCOMPLETE;

    // Copy the value
    size_t size = sprint_stringbuilder_count(builder) + 1;
    char buffer[size];
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_stringbuilder_output(builder, buffer, size)))
        return sprint_rethrow(error);

    // Determine the boolean value
    if (strcasecmp(buffer, SPRINT_TRUE_VALUE) == 0)
        *val = true;
    else if (strcasecmp(buffer, SPRINT_FALSE_VALUE) == 0)
        *val = false;
    else
        return SPRINT_ERROR_ARGUMENT_FORMAT;

    return SPRINT_ERROR_NONE;
}

sprint_error sprint_token_int(sprint_token* token, sprint_stringbuilder* builder, int* val)
{
    if (token == NULL || builder == NULL || val == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (token->type != SPRINT_TOKEN_TYPE_NUMBER) return SPRINT_ERROR_ARGUMENT_FORMAT;
    if (sprint_stringbuilder_count(builder) < 1) return SPRINT_ERROR_ARGUMENT_INCOMPLETE;

    // Copy the value
    size_t size = sprint_stringbuilder_count(builder) + 1;
    char buffer[size];
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_stringbuilder_output(builder, buffer, size)))
        return sprint_rethrow(error);

    // Convert the number
    errno = 0;
    char* number_end = NULL;
    long number = strtol(buffer, &number_end, 10);
    if (errno != 0 || number_end == NULL || *number_end != 0)
        return SPRINT_ERROR_ARGUMENT_FORMAT;

    // Assign the converted number
    *val = number;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_token_str(sprint_token* token, sprint_stringbuilder* builder, char** str)
{
    if (token == NULL || builder == NULL || str == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (token->type != SPRINT_TOKEN_TYPE_STRING) return SPRINT_ERROR_ARGUMENT_FORMAT;

    // Allocate a dynamic buffer
    size_t size = sprint_stringbuilder_count(builder) + 1;
    *str = malloc(size);
    if (*str == NULL)
        return SPRINT_ERROR_MEMORY;

    // Copy the value
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_stringbuilder_output(builder, *str, size));
    return sprint_rethrow(error);
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

/**
 * Reads the next token from the tokenizer.
 * @param tokenizer The tokenizer instance.
 * @param token The target reference to write the read token to.
 * @param builder The builder to write the contents of the token to.
 * @return No error returned on success. At the end of input, returns EOF in combination with an empty or invalid token.
 */
sprint_error sprint_tokenizer_next(sprint_tokenizer* tokenizer, sprint_token* token, sprint_stringbuilder* builder)
{
    if (tokenizer == NULL || token == NULL || builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (tokenizer->read == NULL) return SPRINT_ERROR_STATE_INVALID;

    // Clear the token
    memset(token, 0, sizeof(*token));

    // Make sure the builder is empty
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_stringbuilder_count(builder) > 0 && !sprint_chain(error, sprint_stringbuilder_clear(builder)))
        return sprint_rethrow(error);

    // Make sure the tokenizer is preloaded by reading the first character and determining the first state
    if (!tokenizer->preloaded && tokenizer->read(tokenizer))
        tokenizer->next_state = sprint_tokenizer_state_first(tokenizer->next_chr);

    // Process until reaching EOF
    bool scanning = true;
    while (!tokenizer->last_eof) {
        // Move to the next character and state
        char current_chr = tokenizer->next_chr;
        sprint_tokenizer_state current_state = tokenizer->next_state;

        // Store the origin, if at a transition
        if (scanning && sprint_tokenizer_state_type(current_state) != SPRINT_TOKEN_TYPE_NONE) {
            token->origin = tokenizer->origin;
            scanning = false;
        }

        // Read the next character (ignoring whether it succeeded) and determine the state
        tokenizer->read(tokenizer);
        tokenizer->next_state = sprint_tokenizer_state_next(current_state, tokenizer->next_chr);

        // Decide, whether to record it
        if (sprint_tokenizer_state_recorded(current_state) &&
            !sprint_chain(error, sprint_stringbuilder_put_chr(builder, current_chr)))
            return sprint_rethrow(error);

        // Check, whether the token is complete
        if (!sprint_tokenizer_state_complete(current_state, tokenizer->next_state))
            continue;

        // Update the type and return success
        token->type = sprint_tokenizer_state_type(current_state);
        return SPRINT_ERROR_NONE;
    }

    // If the EOF was reached without finding anything, token an empty token instead of an invalid one
    if (scanning) {
        token->type = SPRINT_TOKEN_TYPE_NONE;
        token->origin = tokenizer->origin;
    } else
        token->type = SPRINT_TOKEN_TYPE_INVALID;

    // And return an EOF error
    return SPRINT_ERROR_EOF;
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
    // Determine, if there has been a line-ending and update the last state accordingly
    bool current_cr = chr == '\r', current_lf = chr == '\n';

    // Update the position
    if (current_cr | current_lf & !tokenizer->last_cr) {
        tokenizer->origin.line++;
        tokenizer->origin.pos = 0;
    } else if (tokenizer->preloaded & !(current_cr | current_lf | tokenizer->last_cr | tokenizer->last_lf))
        tokenizer->origin.pos++;

    // Set the preload flag
    tokenizer->preloaded = true;

    // Update the flags
    tokenizer->last_cr = current_cr;
    tokenizer->last_lf = current_lf;
}

bool sprint_tokenizer_read_str_internal(sprint_tokenizer* tokenizer)
{
    if (tokenizer == NULL || tokenizer->str == NULL || tokenizer->last_eof) return false;

    // Read the character and check, if the EOF is reached
    char chr = *tokenizer->str;
    if (chr == 0) {
        // If so, store a new line, set the EOF flag and fail
        tokenizer->next_chr = '\n';
        tokenizer->last_eof = true;
        return false;
    }

    // Increment the pointer
    tokenizer->str++;

    // Count and store the character
    sprint_tokenizer_count_internal(tokenizer, chr);
    tokenizer->next_chr = chr;
    return true;
}

bool sprint_tokenizer_read_file_internal(sprint_tokenizer* tokenizer)
{
    if (tokenizer == NULL || tokenizer->file == NULL || tokenizer->last_eof) return false;

    // Read the character and check, if the EOF is reached
    int raw_chr = fgetc(tokenizer->file);
    if (raw_chr == EOF) {
        // If so, store a new line, set the EOF flag and fail
        tokenizer->next_chr = '\n';
        tokenizer->last_eof = true;
        return false;
    }
    char chr = (char) raw_chr;

    // Count and store the character
    sprint_tokenizer_count_internal(tokenizer, chr);
    tokenizer->next_chr = chr;
    return true;
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
