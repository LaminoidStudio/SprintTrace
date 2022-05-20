//
// Created by Benedikt on 30.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "parser.h"
#include "primitives.h"
#include "elements.h"
#include "stringbuilder.h"
#include "token.h"
#include "errors.h"

#include <string.h>
#include <stdlib.h>

char* sprint_parser_statement_name(sprint_statement* statement)
{
    return statement != NULL ? statement->name : NULL;
}

bool sprint_parser_statement_flags(sprint_statement* statement, sprint_statement_flags flags)
{
    return statement != NULL && (statement->flags & flags) > 0;
}

int sprint_parser_statement_index(sprint_statement* statement)
{
    return statement != NULL ? statement->index : 0;
}

sprint_parser* sprint_parser_create(sprint_tokenizer* tokenizer)
{
    if (tokenizer == NULL || tokenizer->read == NULL) return NULL;

    // Create the builder
    sprint_stringbuilder* builder = sprint_stringbuilder_create(15);
    if (builder == NULL)
        return NULL;

    // Create the parser
    sprint_parser* parser = calloc(1, sizeof(*tokenizer));
    if (parser == NULL) {
        sprint_check(sprint_stringbuilder_destroy(builder));
        return NULL;
    }

    // Assign tokenizer and builder
    parser->tokenizer = tokenizer;
    parser->builder = builder;
    return parser;
}

sprint_error sprint_parser_token(sprint_parser* parser, sprint_token* token)
{
    if (parser == NULL || token == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    *token = parser->token;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_parser_origin(sprint_parser* parser, sprint_source_origin* origin)
{
    if (parser == NULL || origin == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    *origin = parser->token.origin;
    return SPRINT_ERROR_NONE;
}

static sprint_error sprint_token_unexpected_internal(sprint_parser* parser, bool warning)
{
    sprint_token* token = &parser->token;
    bool invalid = token->type == SPRINT_TOKEN_TYPE_INVALID;
    sprint_stringbuilder* builder = sprint_stringbuilder_of(invalid ? "invalid" : "unexpected");
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    // Assemble the string further
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_stringbuilder_put_str(builder, " token "));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, warning ? "skipped" : "hit"));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, " at "));
    if (token->origin.source != NULL)
        sprint_chain(error, sprint_stringbuilder_format(builder, "%s:", token->origin.source));
    sprint_chain(error, sprint_stringbuilder_format(builder, "%d:%d", token->origin.line, token->origin.pos));

    // Append the contents
    char* contents = NULL;
    if (sprint_check(sprint_token_contents(token, parser->builder, &contents)) && contents != NULL)
        sprint_chain(error, sprint_stringbuilder_format(builder, ": \"%s\"", contents));

    // Complete the builder
    contents = sprint_stringbuilder_complete(builder);
    if (error != SPRINT_ERROR_NONE)
        return sprint_rethrow(error);
    if (contents == NULL)
        return SPRINT_ERROR_INTERNAL;

    // Emit the warning or error
    if (warning)
        sprint_warning(contents);
    else
        sprint_throw(false, contents);

    return SPRINT_ERROR_NONE;
}

sprint_error sprint_parser_next_statement(sprint_parser* parser, sprint_statement* statement, bool sync)
{
    if (parser == NULL || statement == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Keep seeking until a token is found
    sprint_error error = SPRINT_ERROR_NONE;
    bool skipped = false;
    while (true) {
        // Get the first or next token
        sprint_token* token = &parser->token;
        if (error == SPRINT_ERROR_NONE)
            error = sprint_tokenizer_next(parser->tokenizer, token, parser->builder);
        if (error == SPRINT_ERROR_EOF || !sprint_check(error))
            return sprint_rethrow(error);

        // Raise the subsequent flag, if not a word
        parser->subsequent |= token->type != SPRINT_TOKEN_TYPE_WORD;

        // Backup and update the value flag
        bool last_value = parser->value;
        parser->value = token->type == SPRINT_TOKEN_TYPE_VALUE_SEPARATOR;

        // Check the token type
        switch (token->type) {
            case SPRINT_TOKEN_TYPE_WORD:
                // If the word is part of a value, it is not valid, so decide whether to throw or skip
                if (last_value)
                    break;

                // Clear the statement
                memset(statement, 0, sizeof(*statement));

                // Now that a word was found, set the first flag if applicable and store the name
                if (!parser->subsequent)
                    statement->flags |= SPRINT_STATEMENT_FLAG_FIRST;
                if (!sprint_chain(error, sprint_token_word(token, parser->builder, &statement->name)))
                    return sprint_rethrow(error);

                // Get the next token, determine its type and reset the subsequent flag
                if (error == SPRINT_ERROR_NONE)
                    error = sprint_tokenizer_next(parser->tokenizer, token, parser->builder);
                if (error == SPRINT_ERROR_EOF || !sprint_check(error))
                    return sprint_rethrow(error);
                parser->subsequent = true;
                switch (token->type) {
                    case SPRINT_TOKEN_TYPE_VALUE_SEPARATOR:
                        // Set the value flags and return success
                        parser->value = true;
                        statement->flags |= SPRINT_STATEMENT_FLAG_VALUE;
                        return SPRINT_ERROR_NONE;
                    case SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR:
                        // Clear the subsequent flag for the next word
                        parser->subsequent = false;
                        // fallthrough
                    case SPRINT_TOKEN_TYPE_STATEMENT_SEPARATOR:
                        // Clear the value flag
                        parser->value = false;
                        // At this point, a valid statement was found
                        return SPRINT_ERROR_NONE;
                    case SPRINT_TOKEN_TYPE_NUMBER:
                        // Store the index, set the flag and continue reading
                        if (!sprint_chain(error, sprint_token_int(token, parser->builder, &statement->index)))
                            return sprint_rethrow(error);
                        statement->flags |= SPRINT_STATEMENT_FLAG_INDEX;
                        break;
                    default:
                        // The token is unexpected or invalid
                        sprint_check(sprint_token_unexpected_internal(parser, sync));
                        if (sync)
                            continue;
                        return SPRINT_ERROR_SYNTAX;
                }

                // Get the next token, make sure it is a value separator and reset the subsequent flag
                if (error == SPRINT_ERROR_NONE)
                    error = sprint_tokenizer_next(parser->tokenizer, token, parser->builder);
                if (error == SPRINT_ERROR_EOF || !sprint_check(error))
                    return sprint_rethrow(error);
                parser->subsequent = true;
                switch (token->type) {
                    case SPRINT_TOKEN_TYPE_VALUE_SEPARATOR:
                        // Set the value flags and return that a valid statement was found
                        parser->value = true;
                        statement->flags |= SPRINT_STATEMENT_FLAG_VALUE;
                        return SPRINT_ERROR_NONE;
                    case SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR:
                        // Clear the subsequent flag for the next word
                        parser->subsequent = false;
                        // fallthrough
                    case SPRINT_TOKEN_TYPE_STATEMENT_SEPARATOR:
                        // Clear the value flag
                        parser->value = false;
                        // fallthrough
                    default:
                        // The token is unexpected or invalid
                        sprint_check(sprint_token_unexpected_internal(parser, sync));
                        if (sync)
                            continue;
                        return SPRINT_ERROR_SYNTAX;
                }

            case SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR:
                // Clear the subsequent flag for the next word
                parser->subsequent = false;
                // fallthrough
            case SPRINT_TOKEN_TYPE_STATEMENT_SEPARATOR:
                // If there initially is a separator or terminator, skip it in any mode
                skipped = !skipped;
                if (skipped)
                    continue;
                // fallthrough
            case SPRINT_TOKEN_TYPE_NUMBER:
            case SPRINT_TOKEN_TYPE_STRING:
            case SPRINT_TOKEN_TYPE_VALUE_SEPARATOR:
            case SPRINT_TOKEN_TYPE_TUPLE_SEPARATOR:
                // Go to the end of the switch block and decide whether to throw or just skip
                break;

            case SPRINT_TOKEN_TYPE_NONE:
            case SPRINT_TOKEN_TYPE_INVALID:
            default:
                // The token is unexpected or invalid, so throw a syntax error
                sprint_check(sprint_token_unexpected_internal(parser, sync));

                // If syncing, skip over the error
                if (sync)
                    continue;

                // Otherwise, return with a syntax error
                return SPRINT_ERROR_SYNTAX;
        }

        // If syncing, just skip silently
        if (sync)
            continue;

        // Otherwise, throw and return a syntax error
        sprint_check(sprint_token_unexpected_internal(parser, false));
        return SPRINT_ERROR_SYNTAX;
    }
}

sprint_error sprint_parser_next_bool(sprint_parser* parser, bool* val)
{
}

sprint_error sprint_parser_next_int(sprint_parser* parser, int* val)
{
}

sprint_error sprint_parser_next_dist(sprint_parser* parser, sprint_dist* dist)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_chain(error, sprint_parser_next_int(parser, dist)) && !sprint_dist_valid(*dist))
        return SPRINT_ERROR_ARGUMENT_FORMAT;
    return error;
}

sprint_error sprint_parser_next_size(sprint_parser* parser, sprint_dist* size)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_chain(error, sprint_parser_next_int(parser, size)) && !sprint_size_valid(*size))
        return SPRINT_ERROR_ARGUMENT_FORMAT;
    return error;
}

sprint_error sprint_parser_next_angle(sprint_parser* parser, sprint_angle* angle, sprint_prim_format format)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_parser_next_int(parser, angle)))
        return error;

    // Handle the different raw precisions
    sprint_angle factor = sprint_angle_factor(format);
    if (factor < 1)
        return SPRINT_ERROR_ARGUMENT_RANGE;

    // Scale the angle
    *angle *= factor;

    // And make sure that it is valid
    return sprint_angle_valid(*angle) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ARGUMENT_FORMAT;
}

sprint_error sprint_parser_next_tuple(sprint_parser* parser, sprint_tuple* tuple)
{
}

sprint_error sprint_parser_next_str(sprint_parser* parser, char* str)
{
}

sprint_error sprint_parser_run(sprint_parser* parser, sprint_list* elements)
{
}

sprint_error sprint_parser_destroy(sprint_parser* parser, bool tokenizer, char** contents)
{
    if (parser == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Destroy the tokenizer, if desired
    if (tokenizer && parser->tokenizer != NULL)
        sprint_check(sprint_tokenizer_destroy(parser->tokenizer));
    parser->tokenizer = NULL;

    // Store the contents of the builder, if desired
    if (contents != NULL && parser->builder != NULL) {
        *contents = sprint_stringbuilder_complete(parser->builder);
        sprint_assert(false, *contents != NULL);
    }
    parser->builder = NULL;

    // Free the memory
    free(parser);
    return SPRINT_ERROR_NONE;
}
