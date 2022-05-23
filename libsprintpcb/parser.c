//
// Created by Benedikt on 30.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "parser.h"
#include "primitives.h"
#include "elements.h"
#include "list.h"
#include "stringbuilder.h"
#include "token.h"
#include "errors.h"

#include <string.h>
#include <stdlib.h>

char* sprint_parser_statement_name(sprint_statement* statement)
{
    return statement != NULL ? statement->name : NULL;
}

bool sprint_parser_statement_flags(sprint_statement* statement, bool any, sprint_statement_flags flags)
{
    if (statement == NULL || flags < 1) return false;
    return any ? (statement->flags & flags) > 0 : (statement->flags & flags) == flags;
}

int sprint_parser_statement_index(sprint_statement* statement)
{
    return statement != NULL ? statement->index : 0;
}

sprint_error sprint_parser_statement_destroy(sprint_statement* statement)
{
    if (statement == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Free the name
    if (statement->name != NULL) {
        free(statement->name);
        statement->name = NULL;
    }

    return SPRINT_ERROR_NONE;
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
                if (error == SPRINT_ERROR_EOF || !sprint_check(error)) {
                    sprint_check(sprint_parser_statement_destroy(statement));
                    return sprint_rethrow(error);
                }
                parser->subsequent = true;
                switch (token->type) {
                    case SPRINT_TOKEN_TYPE_VALUE_SEPARATOR:
                        // Set the value flags and return success
                        parser->value = true;
                        statement->flags |= SPRINT_STATEMENT_FLAG_VALUE;
                        return SPRINT_ERROR_NONE;
                    case SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR:
                        // Clear the subsequent flag for the next word and set the last flag
                        parser->subsequent = false;
                        statement->flags |= SPRINT_STATEMENT_FLAG_LAST;
                        // fallthrough
                    case SPRINT_TOKEN_TYPE_STATEMENT_SEPARATOR:
                        // Clear the value flag
                        parser->value = false;
                        // At this point, a valid statement was found
                        return SPRINT_ERROR_NONE;
                    case SPRINT_TOKEN_TYPE_NUMBER:
                        // Store the index, set the flag and continue reading
                        if (!sprint_chain(error, sprint_token_int(token, parser->builder, &statement->index))) {
                            sprint_check(sprint_parser_statement_destroy(statement));
                            return sprint_rethrow(error);
                        }
                        statement->flags |= SPRINT_STATEMENT_FLAG_INDEX;
                        break;
                    default:
                        // The token is unexpected or invalid
                        sprint_check(sprint_parser_statement_destroy(statement));
                        sprint_check(sprint_token_unexpected_internal(parser, sync));
                        if (sync)
                            continue;
                        return SPRINT_ERROR_SYNTAX;
                }

                // Get the next token, make sure it is a value separator and reset the subsequent flag
                if (error == SPRINT_ERROR_NONE)
                    error = sprint_tokenizer_next(parser->tokenizer, token, parser->builder);
                if (error == SPRINT_ERROR_EOF || !sprint_check(error)) {
                    sprint_check(sprint_parser_statement_destroy(statement));
                    return sprint_rethrow(error);
                }
                parser->subsequent = true;
                switch (token->type) {
                    case SPRINT_TOKEN_TYPE_VALUE_SEPARATOR:
                        // If there was a value separator already, fail
                        if (sprint_parser_statement_flags(statement, false, SPRINT_STATEMENT_FLAG_VALUE))
                            break;
                        // Set the value flags and return that a valid statement was found
                        parser->value = true;
                        statement->flags |= SPRINT_STATEMENT_FLAG_VALUE;
                        return SPRINT_ERROR_NONE;
                    case SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR:
                        // Clear the subsequent flag for the next word and set the last flag
                        statement->flags |= SPRINT_STATEMENT_FLAG_LAST;
                        parser->subsequent = false;
                        // fallthrough
                    case SPRINT_TOKEN_TYPE_STATEMENT_SEPARATOR:
                        // Clear the value flag
                        parser->value = false;
                        // fallthrough
                    default:
                        // Fall through to an error
                        break;
                }
                // The token is unexpected or invalid
                sprint_check(sprint_parser_statement_destroy(statement));
                sprint_check(sprint_token_unexpected_internal(parser, sync));
                if (sync)
                    continue;
                return SPRINT_ERROR_SYNTAX;

            case SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR:
                // If not syncing, return an empty statement
                if (!sync) {
                    // Clear the statement
                    memset(statement, 0, sizeof(*statement));

                    // Set the last flag
                    statement->flags = SPRINT_STATEMENT_FLAG_LAST;

                    // Set the first flag, if applicable
                    if (!parser->subsequent)
                        statement->flags |= SPRINT_STATEMENT_FLAG_FIRST;

                    // Clear the subsequent flag for the next word and return success
                    parser->subsequent = false;
                    return SPRINT_ERROR_NONE;
                }

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

static sprint_error sprint_parser_next_internal(sprint_parser* parser, sprint_token_type type)
{
    // Get the next token and reset the subsequent flag
    sprint_token* token = &parser->token;
    sprint_error error = sprint_tokenizer_next(parser->tokenizer, token, parser->builder);
    if (error == SPRINT_ERROR_EOF || !sprint_check(error))
        return sprint_rethrow(error);
    parser->subsequent = true;

    // Handle the special conditions
    switch (token->type) {
        case SPRINT_TOKEN_TYPE_VALUE_SEPARATOR:
            // Set the value flag
            parser->value = true;
            break;
        case SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR:
            // Clear the subsequent flag for the next word
            parser->subsequent = false;
            // fallthrough
        case SPRINT_TOKEN_TYPE_STATEMENT_SEPARATOR:
            // Clear the value flag
            parser->value = false;
            break;
        default:
            // If the token type matches, it succeeded
            if (token->type == type)
                return SPRINT_ERROR_NONE;

            // Otherwise, fall through to a syntax error
            break;
    }

    // Throw the syntax error
    sprint_token_unexpected_internal(parser, false);
    return SPRINT_ERROR_SYNTAX;
}

sprint_error sprint_parser_next_bool(sprint_parser* parser, bool* val)
{
    if (parser == NULL || val == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_parser_next_internal(parser, SPRINT_TOKEN_TYPE_WORD));
    sprint_chain(error, sprint_token_bool(&parser->token, parser->builder, val));
    return sprint_rethrow(error);
}

sprint_error sprint_parser_next_int(sprint_parser* parser, int* val)
{
    if (parser == NULL || val == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_parser_next_internal(parser, SPRINT_TOKEN_TYPE_NUMBER));
    sprint_chain(error, sprint_token_int(&parser->token, parser->builder, val));
    return sprint_rethrow(error);
}

sprint_error sprint_parser_next_dist(sprint_parser* parser, sprint_dist* dist)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_chain(error, sprint_parser_next_int(parser, dist)) && !sprint_dist_valid(*dist))
        return SPRINT_ERROR_SYNTAX;
    return error;
}

sprint_error sprint_parser_next_size(sprint_parser* parser, sprint_dist* size)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_chain(error, sprint_parser_next_int(parser, size)) && !sprint_size_valid(*size))
        return SPRINT_ERROR_SYNTAX;
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
    return sprint_angle_valid(*angle) ? SPRINT_ERROR_NONE : SPRINT_ERROR_SYNTAX;
}

sprint_error sprint_parser_next_tuple(sprint_parser* parser, sprint_tuple* tuple)
{
    if (parser == NULL || tuple == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Read the x-component
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_dist dist_x = 0;
    sprint_chain(error, sprint_parser_next_dist(parser, &dist_x));

    // Match the tuple separator
    sprint_chain(error, sprint_parser_next_internal(parser, SPRINT_TOKEN_TYPE_TUPLE_SEPARATOR));

    // Read the y-component
    sprint_dist dist_y = 0;
    sprint_chain(error, sprint_parser_next_dist(parser, &dist_y));

    // Create the tuple and return
    *tuple = sprint_tuple_of(dist_x, dist_y);
    return sprint_rethrow(error);
}

sprint_error sprint_parser_next_str(sprint_parser* parser, char** str)
{
    if (parser == NULL || str == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_parser_next_internal(parser, SPRINT_TOKEN_TYPE_STRING));
    sprint_chain(error, sprint_token_str(&parser->token, parser->builder, str));
    return sprint_rethrow(error);
}

static sprint_error sprint_parser_next_uint(sprint_parser* parser, int* val)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_chain(error, sprint_parser_next_int(parser, val)) && *val < 0)
        return SPRINT_ERROR_SYNTAX;
    return error;
}

static sprint_error sprint_parser_next_layer(sprint_parser* parser, sprint_layer* layer)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_chain(error, sprint_parser_next_int(parser, (int*) layer)) && !sprint_dist_valid(*layer))
        return SPRINT_ERROR_SYNTAX;
    return error;
}

static sprint_error sprint_parser_next_tht_form(sprint_parser* parser, sprint_pad_tht_form* form)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_chain(error, sprint_parser_next_int(parser, (int*) form)) && !sprint_pad_tht_form_valid(*form))
        return SPRINT_ERROR_SYNTAX;
    return error;
}

static sprint_error sprint_parser_next_text_style(sprint_parser* parser, sprint_text_style* style)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_chain(error, sprint_parser_next_int(parser, (int*) style)) && !sprint_text_style_valid(*style))
        return SPRINT_ERROR_SYNTAX;
    return error;
}

static sprint_error sprint_parser_next_text_thickness(sprint_parser* parser, sprint_text_thickness* thickness)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_chain(error, sprint_parser_next_int(parser, (int*) thickness)) &&
        !sprint_text_thickness_valid(*thickness))
        return SPRINT_ERROR_SYNTAX;
    return error;
}

static sprint_error sprint_parser_value_internal(sprint_parser* parser, sprint_statement* statement, bool* salvaged)
{
    // Keep reading until a valid statement or a terminator is found
    while (parser->subsequent) {
        // Read the next statement
        sprint_error error = sprint_parser_next_statement(parser, statement, *salvaged);
        if (error == SPRINT_ERROR_SYNTAX) {
            // Destroy the statement
            sprint_check(sprint_parser_statement_destroy(statement));

            // Set the salvaged flag, emit a warning and skip this statement
            *salvaged = true;
            sprint_check(sprint_token_unexpected_internal(parser, true));
            continue;
        } else if (!sprint_check(error))
            return sprint_rethrow(error);

        // Make sure the flags are valid
        const sprint_statement_flags required_flags = SPRINT_STATEMENT_FLAG_NAME | SPRINT_STATEMENT_FLAG_VALUE;
        if (sprint_parser_statement_flags(statement, false, required_flags))
            return SPRINT_ERROR_NONE;

        // Destroy the statement
        sprint_check(sprint_parser_statement_destroy(statement));

        // If the last flag is present, stop processing
        if (sprint_parser_statement_flags(statement, true, SPRINT_STATEMENT_FLAG_LAST))
            break;

        // Set the salvaged flag, emit a warning and skip this statement
        *salvaged = true;
        sprint_check(sprint_token_unexpected_internal(parser, true));
    }

    // Return an end of statement error
    return SPRINT_ERROR_EOS;
}

static sprint_error sprint_parser_next_track_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
    // Initialize the element
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_track_default(element, true)))
        return sprint_rethrow(error);
    element->parsed = true;

    // Keep track of found properties
    bool found_layer = false, found_width = false, found_clear = false, found_cutout = false, found_soldermask = false,
            found_flat_start = false, found_flat_end = false;

    // Keep a list of points
    sprint_list* list = sprint_list_create(sizeof(*element->track.points), 15);
    if (list == NULL)
        return SPRINT_ERROR_MEMORY;

    // Read all element properties
    sprint_statement statement;
    while (parser->subsequent) {
        // Read the next value statement
        error = sprint_parser_value_internal(parser, &statement, salvaged);
        if (error == SPRINT_ERROR_EOS)
            break;
        if (!sprint_check(error)) {
            sprint_check(sprint_list_destroy(list));
            return sprint_rethrow(error);
        }

        // Determine the statement name
        bool already_found = false;
        if (strcasecmp(statement.name, "P") == 0) {
            sprint_tuple tuple;
            if (sprint_parser_statement_flags(&statement, true, SPRINT_STATEMENT_FLAG_INDEX) &&
                    sprint_parser_statement_index(&statement) == sprint_list_count(list) &&
                    sprint_chain(error, sprint_parser_next_tuple(parser, &tuple)))
                    sprint_chain(error, sprint_list_add(list, &tuple));
        } else if (strcasecmp(statement.name, "LAYER") == 0) {
            if (found_layer)
                already_found = true;
            sprint_chain(error, sprint_parser_next_layer(parser, &element->track.layer));
            found_layer = true;
        } else if (strcasecmp(statement.name, "WIDTH") == 0) {
            if (found_width)
                already_found = true;
            sprint_chain(error, sprint_parser_next_size(parser, &element->track.width));
            found_width = true;
        } else if (strcasecmp(statement.name, "CLEAR") == 0) {
            if (found_clear)
                already_found = true;
            sprint_chain(error, sprint_parser_next_size(parser, &element->track.clear));
            found_clear = true;
        } else if (strcasecmp(statement.name, "CUTOUT") == 0) {
            if (found_cutout)
                already_found = true;
            sprint_chain(error, sprint_parser_next_bool(parser, &element->track.cutout));
            found_cutout = true;
        } else if (strcasecmp(statement.name, "SOLDERMASK") == 0) {
            if (found_soldermask)
                already_found = true;
            sprint_chain(error, sprint_parser_next_bool(parser, &element->track.soldermask));
            found_soldermask = true;
        } else if (strcasecmp(statement.name, "FLATSTART") == 0) {
            if (found_flat_start)
                already_found = true;
            sprint_chain(error, sprint_parser_next_bool(parser, &element->track.flat_start));
            found_flat_start = true;
        } else if (strcasecmp(statement.name, "FLATEND") == 0) {
            if (found_flat_end)
                already_found = true;
            sprint_chain(error, sprint_parser_next_bool(parser, &element->track.flat_end));
            found_flat_end = true;
        } else {
            error = SPRINT_ERROR_SYNTAX;
            sprint_throw_format("unknown property: %s", statement.name);
        }

        // Destroy the statement
        sprint_check(sprint_parser_statement_destroy(&statement));

        // Handle syntax errors by enabling salvaged mode and ignoring the property
        if (error == SPRINT_ERROR_SYNTAX) {
            *salvaged = true;
            continue;
        }

        // All other errors stop processing
        if (error != SPRINT_ERROR_NONE) {
            sprint_check(sprint_list_destroy(list));
            return sprint_rethrow(error);
        }

        // Handle already found properties
        if (already_found)
            sprint_warning_format("overwriting duplicate property: %s", statement.name);
    }

    // Make sure that there are at least two points and all flags
    if (!found_layer | !found_width | sprint_list_count(list) < 2) {
        sprint_throw_format("incomplete element: %s", sprint_element_type_to_keyword(SPRINT_ELEMENT_TRACK, false));
        error = SPRINT_ERROR_SYNTAX;
    }

    // Complete the list and check verify full validity
    if (sprint_chain(error, sprint_list_complete(list, &element->track.num_points, (void*) &element->track.points)) &&
        !sprint_assert(false, sprint_track_valid(&element->track)))
        error = SPRINT_ERROR_ASSERTION;

    // Check, if everything went well
    if (error != SPRINT_ERROR_NONE)
        sprint_check(sprint_element_destroy(element));
    return sprint_rethrow(error);
}

static sprint_error sprint_parser_next_pad_tht_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
}

static sprint_error sprint_parser_next_pad_smt_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
}

static sprint_error sprint_parser_next_zone_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
}

static sprint_error sprint_parser_next_text_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
}

static sprint_error sprint_parser_next_circle_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
static sprint_error sprint_parser_next_component_internal(sprint_parser* parser, sprint_element* element,
                                                          bool* salvaged, int depth)
{
}

static sprint_error sprint_parser_next_group_internal(sprint_parser* parser, sprint_element* element,
                                                      bool* salvaged, int depth)
{
}

static sprint_error sprint_parser_next_element_internal(sprint_parser* parser, sprint_element* element,
                                                        bool* salvaged, sprint_element_type parent, int depth)
{
    if (parser == NULL || element == NULL || salvaged == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (depth < 0 || depth > SPRINT_ELEMENT_DEPTH) return SPRINT_ERROR_RECURSION;

    // Clear the salvaged flag
    *salvaged = false;

    // Keep reading statements until there is one that can be read
    sprint_statement statement;
    sprint_error error;
    while (true) {
        // Read the next statement
        error = sprint_parser_next_statement(parser, &statement, *salvaged);
        if (error == SPRINT_ERROR_EOF || !sprint_check(error))
            return sprint_rethrow(error);

        const sprint_statement_flags bad_flags = SPRINT_STATEMENT_FLAG_VALUE,
            required_flags = SPRINT_STATEMENT_FLAG_FIRST | SPRINT_STATEMENT_FLAG_NAME;

        // Check, if the flags are valid for a keyword
        if (sprint_parser_statement_flags(&statement, true, bad_flags) ||
            !sprint_parser_statement_flags(&statement, false, required_flags))
        {
            // If they aren't, destroy the statement
            sprint_check(sprint_parser_statement_destroy(&statement));

            // Emit a warning, turn on salvaged mode and move on to the next element
            *salvaged = true;
            sprint_check(sprint_token_unexpected_internal(parser, true));
            continue;
        }

        // Clear the element
        memset(element, 0, sizeof(*element));

        // If the depth is at least one and the parent a component, try to match the other text types first
        bool element_salvaged = false;
        sprint_text_type text_type = 0;
        if (depth > 0 && parent == SPRINT_ELEMENT_COMPONENT &&
            sprint_text_type_from_keyword(&text_type, statement.name)) {
            // Destroy the statement, read the text and update the subtype
            sprint_check(sprint_parser_statement_destroy(&statement));
            if (sprint_chain(error, sprint_parser_next_text_internal(parser, element, &element_salvaged)))
                element->text.subtype = text_type;

            // Update the salvaged flag
            *salvaged |= element_salvaged;

            // For any status other than a syntax error, stop the loop
            if (error != SPRINT_ERROR_SYNTAX)
                break;

            // Emit a warning, turn on salvaged mode and move on to the next element, if there was a syntax error
            *salvaged = true;
            sprint_token_unexpected_internal(parser, true);
            continue;
        }

        // Otherwise, determine the type of the element and destroy the statement
        sprint_element_type type = 0;
        bool closing = false;
        bool success = sprint_element_type_from_keyword(&type, &closing, statement.name);
        sprint_check(sprint_parser_statement_destroy(&statement));
        if (!success || closing) {
            // Emit a warning, turn on salvaged mode and move on to the next element
            *salvaged = true;
            sprint_token_unexpected_internal(parser, true);
            continue;
        }

        // And dispatch to the correct parser
        switch (type) {
            case SPRINT_ELEMENT_TRACK:
                sprint_chain(error, sprint_parser_next_track_internal(parser, element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_PAD_THT:
                sprint_chain(error, sprint_parser_next_pad_tht_internal(parser, element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_PAD_SMT:
                sprint_chain(error, sprint_parser_next_pad_smt_internal(parser, element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_ZONE:
                sprint_chain(error, sprint_parser_next_zone_internal(parser, element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_TEXT:
                sprint_chain(error, sprint_parser_next_text_internal(parser, element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_CIRCLE:
                sprint_chain(error, sprint_parser_next_circle_internal(parser, element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_COMPONENT:
                sprint_chain(error, sprint_parser_next_component_internal(parser, element, &element_salvaged, depth));
                break;
            case SPRINT_ELEMENT_GROUP:
                sprint_chain(error, sprint_parser_next_group_internal(parser, element, &element_salvaged, depth));
                break;
            default:
                sprint_throw_format(false, "element type unknown: %d", type);
                return SPRINT_ERROR_INTERNAL;
        }

        // Update the salvaged flag
        *salvaged |= element_salvaged;

        // For any status other than a syntax error, stop the loop
        if (error != SPRINT_ERROR_SYNTAX)
            break;

        // Emit a warning, turn on salvaged mode and move on to the next element, if there was a syntax error
        *salvaged = true;
        sprint_token_unexpected_internal(parser, true);
    }

    // And just return
    return sprint_rethrow(error);
}
#pragma clang diagnostic pop

sprint_error sprint_parser_next_element(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
    return sprint_parser_next_element_internal(parser, element, salvaged, 0, 0);
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
