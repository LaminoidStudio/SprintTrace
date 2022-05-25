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
        sprint_chain(error, sprint_stringbuilder_format(builder, ": %s", contents));

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

                // Set the name flag
                statement->flags |= SPRINT_STATEMENT_FLAG_NAME;

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
                        parser->subsequent = false;
                        statement->flags |= SPRINT_STATEMENT_FLAG_LAST;
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
                if (token->type == SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR)
                    return SPRINT_ERROR_EOS;
                if (sync)
                    continue;
                return SPRINT_ERROR_SYNTAX;

            case SPRINT_TOKEN_TYPE_STATEMENT_TERMINATOR:
                // Clear the subsequent flag for the next word and return an EOS error
                parser->subsequent = false;
                return SPRINT_ERROR_EOS;
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
    if (sprint_chain(error, sprint_parser_next_int(parser, (int*) layer)) && !sprint_layer_valid(*layer))
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

static sprint_error sprint_parser_next_value_internal(sprint_parser* parser, sprint_statement* statement, bool* salvaged)
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
        } else if (error == SPRINT_ERROR_EOS || !sprint_check(error))
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
    sprint_list* list = sprint_list_create(sizeof(*element->track.points), 16);
    if (list == NULL)
        return SPRINT_ERROR_MEMORY;

    // Read all element properties
    sprint_statement statement;
    while (parser->subsequent) {
        // Read the next value statement
        error = sprint_parser_next_value_internal(parser, &statement, salvaged);
        if (error == SPRINT_ERROR_EOS) {
            error = SPRINT_ERROR_NONE;
            break;
        }
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
            found_layer |= sprint_chain(error, sprint_parser_next_layer(parser, &element->track.layer));
        } else if (strcasecmp(statement.name, "WIDTH") == 0) {
            if (found_width)
                already_found = true;
            found_width |= sprint_chain(error, sprint_parser_next_size(parser, &element->track.width));
        } else if (strcasecmp(statement.name, "CLEAR") == 0) {
            if (found_clear)
                already_found = true;
            found_clear |= sprint_chain(error, sprint_parser_next_size(parser, &element->track.clear));
        } else if (strcasecmp(statement.name, "CUTOUT") == 0) {
            if (found_cutout)
                already_found = true;
            found_cutout |= sprint_chain(error, sprint_parser_next_bool(parser, &element->track.cutout));
        } else if (strcasecmp(statement.name, "SOLDERMASK") == 0) {
            if (found_soldermask)
                already_found = true;
            found_soldermask |= sprint_chain(error, sprint_parser_next_bool(parser, &element->track.soldermask));
        } else if (strcasecmp(statement.name, "FLATSTART") == 0) {
            if (found_flat_start)
                already_found = true;
            found_flat_start |= sprint_chain(error, sprint_parser_next_bool(parser, &element->track.flat_start));
        } else if (strcasecmp(statement.name, "FLATEND") == 0) {
            if (found_flat_end)
                already_found = true;
            found_flat_end |= sprint_chain(error, sprint_parser_next_bool(parser, &element->track.flat_end));
        } else {
            error = SPRINT_ERROR_SYNTAX;
            sprint_throw_format(false, "unknown property: %s", statement.name);
        }

        // Handle already found properties
        if (already_found)
            sprint_warning_format("overwriting duplicate property: %s", statement.name);

        // Destroy the statement
        sprint_check(sprint_parser_statement_destroy(&statement));

        // Handle syntax errors by enabling salvaged mode and ignoring the property
        if (error == SPRINT_ERROR_SYNTAX) {
            sprint_check(sprint_token_unexpected_internal(parser, false));
            *salvaged = true;
            continue;
        }

        // All other errors stop processing
        if (error != SPRINT_ERROR_NONE) {
            sprint_check(sprint_list_destroy(list));
            return sprint_rethrow(error);
        }
    }

    // Make sure that there are at least two points and all properties
    if (!found_layer | !found_width | sprint_list_count(list) < 2) {
        sprint_throw_format(false, "incomplete element: %s", sprint_element_type_to_keyword(SPRINT_ELEMENT_TRACK, false));
        error = SPRINT_ERROR_SYNTAX;
    }

    // Complete the list and check verify full validity
    if (sprint_chain(error, sprint_list_complete(list, &element->track.num_points, (void*) &element->track.points)) &&
        !sprint_assert(false, sprint_track_valid(&element->track)))
        error = SPRINT_ERROR_ASSERTION;

    return sprint_rethrow(error);
}

static sprint_error sprint_parser_next_pad_tht_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
    // Initialize the element
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_pad_tht_default(element, true)))
        return sprint_rethrow(error);
    element->parsed = true;

    // Keep track of found properties
    bool found_layer = false, found_position = false, found_size = false, found_drill = false, found_form = false,
            found_id = false, found_clear = false, found_soldermask = false, found_rotation = false, found_via = false,
            found_thermal = false, found_thermal_tracks = false, found_thermal_tracks_width = false,
            found_thermal_tracks_individual = false;

    // Keep a list of connections
    sprint_list* list = sprint_list_create(sizeof(*element->pad_tht.link.connections), 8);
    if (list == NULL)
        return SPRINT_ERROR_MEMORY;

    // Read all element properties
    sprint_statement statement;
    while (parser->subsequent) {
        // Read the next value statement
        error = sprint_parser_next_value_internal(parser, &statement, salvaged);
        if (error == SPRINT_ERROR_EOS) {
            error = SPRINT_ERROR_NONE;
            break;
        }
        if (!sprint_check(error)) {
            sprint_check(sprint_list_destroy(list));
            return sprint_rethrow(error);
        }

        // Determine the statement name
        bool already_found = false;
        if (strcasecmp(statement.name, "CON") == 0) {
            int id = 0;
            if (sprint_parser_statement_flags(&statement, true, SPRINT_STATEMENT_FLAG_INDEX) &&
                sprint_parser_statement_index(&statement) == sprint_list_count(list) &&
                sprint_chain(error, sprint_parser_next_uint(parser, &id)))
                sprint_chain(error, sprint_list_add(list, &id));
        } else if (strcasecmp(statement.name, "LAYER") == 0) {
            if (found_layer)
                already_found = true;
            found_layer |= sprint_chain(error, sprint_parser_next_layer(parser, &element->pad_tht.layer));
        } else if (strcasecmp(statement.name, "POS") == 0) {
            if (found_position)
                already_found = true;
            found_position |= sprint_chain(error, sprint_parser_next_tuple(parser, &element->pad_tht.position));
        } else if (strcasecmp(statement.name, "SIZE") == 0) {
            if (found_size)
                already_found = true;
            found_size |= sprint_chain(error, sprint_parser_next_size(parser, &element->pad_tht.size));
        } else if (strcasecmp(statement.name, "DRILL") == 0) {
            if (found_drill)
                already_found = true;
            found_drill |= sprint_chain(error, sprint_parser_next_size(parser, &element->pad_tht.drill));
        } else if (strcasecmp(statement.name, "FORM") == 0) {
            if (found_form)
                already_found = true;
            found_form |= sprint_chain(error, sprint_parser_next_tht_form(parser, &element->pad_tht.form));
        } else if (strcasecmp(statement.name, "PAD_ID") == 0) {
            if (found_id)
                already_found = true;
            found_id |= sprint_chain(error, sprint_parser_next_uint(parser, &element->pad_tht.link.id));
            element->pad_tht.link.has_id = found_id;
        } else if (strcasecmp(statement.name, "CLEAR") == 0) {
            if (found_clear)
                already_found = true;
            found_clear |= sprint_chain(error, sprint_parser_next_size(parser, &element->pad_tht.clear));
        } else if (strcasecmp(statement.name, "SOLDERMASK") == 0) {
            if (found_soldermask)
                already_found = true;
            found_soldermask |= sprint_chain(error, sprint_parser_next_bool(parser, &element->pad_tht.soldermask));
        } else if (strcasecmp(statement.name, "ROTATION") == 0) {
            if (found_rotation)
                already_found = true;
            found_rotation |= sprint_chain(error, sprint_parser_next_angle(parser, &element->pad_tht.rotation, SPRINT_PRIM_FORMAT_ANGLE_COARSE));
        } else if (strcasecmp(statement.name, "VIA") == 0) {
            if (found_via)
                already_found = true;
            found_via |= sprint_chain(error, sprint_parser_next_bool(parser, &element->pad_tht.via));
        } else if (strcasecmp(statement.name, "THERMAL") == 0) {
            if (found_thermal)
                already_found = true;
            found_thermal |= sprint_chain(error, sprint_parser_next_bool(parser, &element->pad_tht.thermal));
        } else if (strcasecmp(statement.name, "THERMAL_TRACKS") == 0) {
            if (found_thermal_tracks)
                already_found = true;
            found_thermal_tracks |= sprint_chain(error, sprint_parser_next_int(parser, &element->pad_tht.thermal_tracks));
        } else if (strcasecmp(statement.name, "THERMAL_TRACKS_WIDTH") == 0) {
            if (found_thermal_tracks_width)
                already_found = true;
            found_thermal_tracks_width |= sprint_chain(error, sprint_parser_next_uint(parser, &element->pad_tht.thermal_tracks_width));
        } else if (strcasecmp(statement.name, "THERMAL_TRACKS_INDIVIDUAL") == 0) {
            if (found_thermal_tracks_individual)
                already_found = true;
            found_thermal_tracks_individual |= sprint_chain(error, sprint_parser_next_bool(parser, &element->pad_tht.thermal_tracks_individual));
        } else {
            error = SPRINT_ERROR_SYNTAX;
            sprint_throw_format(false, "unknown property: %s", statement.name);
        }

        // Handle already found properties
        if (already_found)
            sprint_warning_format("overwriting duplicate property: %s", statement.name);

        // Destroy the statement
        sprint_check(sprint_parser_statement_destroy(&statement));

        // Handle syntax errors by enabling salvaged mode and ignoring the property
        if (error == SPRINT_ERROR_SYNTAX) {
            sprint_check(sprint_token_unexpected_internal(parser, false));
            *salvaged = true;
            continue;
        }

        // All other errors stop processing
        if (error != SPRINT_ERROR_NONE) {
            sprint_check(sprint_list_destroy(list));
            return sprint_rethrow(error);
        }
    }

    // Make sure that there are all properties
    if (!found_layer | !found_position | !found_size | !found_drill | !found_form |
        (element->pad_tht.thermal & (!found_thermal_tracks | !found_thermal_tracks_width | !found_thermal_tracks_individual))) {
        sprint_throw_format(false, "incomplete element: %s", sprint_element_type_to_keyword(SPRINT_ELEMENT_PAD_THT, false));
        error = SPRINT_ERROR_SYNTAX;
    }

    // Complete the list and check verify full validity
    if (sprint_chain(error, sprint_list_complete(list, &element->pad_tht.link.num_connections,
                                                 (void*) &element->pad_tht.link.connections)) &&
                        !sprint_assert(false, sprint_pad_tht_valid(&element->pad_tht)))
        error = SPRINT_ERROR_ASSERTION;

    return sprint_rethrow(error);
}

static sprint_error sprint_parser_next_pad_smt_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
    // Initialize the element
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_pad_smt_default(element, true)))
        return sprint_rethrow(error);
    element->parsed = true;

    // Keep track of found properties
    bool found_layer = false, found_position = false, found_width = false, found_height = false, found_id = false,
            found_clear = false, found_soldermask = false, found_rotation = false, found_thermal = false,
            found_thermal_tracks = false, found_thermal_tracks_width = false;

    // Keep a list of connections
    sprint_list* list = sprint_list_create(sizeof(*element->pad_smt.link.connections), 8);
    if (list == NULL)
        return SPRINT_ERROR_MEMORY;

    // Read all element properties
    sprint_statement statement;
    while (parser->subsequent) {
        // Read the next value statement
        error = sprint_parser_next_value_internal(parser, &statement, salvaged);
        if (error == SPRINT_ERROR_EOS) {
            error = SPRINT_ERROR_NONE;
            break;
        }
        if (!sprint_check(error)) {
            sprint_check(sprint_list_destroy(list));
            return sprint_rethrow(error);
        }

        // Determine the statement name
        bool already_found = false;
        if (strcasecmp(statement.name, "CON") == 0) {
            int id = 0;
            if (sprint_parser_statement_flags(&statement, true, SPRINT_STATEMENT_FLAG_INDEX) &&
                sprint_parser_statement_index(&statement) == sprint_list_count(list) &&
                sprint_chain(error, sprint_parser_next_uint(parser, &id)))
                sprint_chain(error, sprint_list_add(list, &id));
        } else if (strcasecmp(statement.name, "LAYER") == 0) {
            if (found_layer)
                already_found = true;
            found_layer |= sprint_chain(error, sprint_parser_next_layer(parser, &element->pad_smt.layer));
        } else if (strcasecmp(statement.name, "POS") == 0) {
            if (found_position)
                already_found = true;
            found_position |= sprint_chain(error, sprint_parser_next_tuple(parser, &element->pad_smt.position));
        } else if (strcasecmp(statement.name, "SIZE_X") == 0) {
            if (found_width)
                already_found = true;
            found_width |= sprint_chain(error, sprint_parser_next_size(parser, &element->pad_smt.width));
        } else if (strcasecmp(statement.name, "SIZE_Y") == 0) {
            if (found_height)
                already_found = true;
            found_height |= sprint_chain(error, sprint_parser_next_size(parser, &element->pad_smt.height));
        } else if (strcasecmp(statement.name, "PAD_ID") == 0) {
            if (found_id)
                already_found = true;
            found_id |= sprint_chain(error, sprint_parser_next_uint(parser, &element->pad_smt.link.id));
            element->pad_smt.link.has_id = found_id;
        } else if (strcasecmp(statement.name, "CLEAR") == 0) {
            if (found_clear)
                already_found = true;
            found_clear |= sprint_chain(error, sprint_parser_next_size(parser, &element->pad_smt.clear));
        } else if (strcasecmp(statement.name, "SOLDERMASK") == 0) {
            if (found_soldermask)
                already_found = true;
            found_soldermask |= sprint_chain(error, sprint_parser_next_bool(parser, &element->pad_smt.soldermask));
        } else if (strcasecmp(statement.name, "ROTATION") == 0) {
            if (found_rotation)
                already_found = true;
            found_rotation |= sprint_chain(error, sprint_parser_next_angle(parser, &element->pad_smt.rotation, SPRINT_PRIM_FORMAT_ANGLE_COARSE));
        } else if (strcasecmp(statement.name, "THERMAL") == 0) {
            if (found_thermal)
                already_found = true;
            found_thermal |= sprint_chain(error, sprint_parser_next_bool(parser, &element->pad_smt.thermal));
        } else if (strcasecmp(statement.name, "THERMAL_TRACKS") == 0) {
            if (found_thermal_tracks)
                already_found = true;
            found_thermal_tracks |= sprint_chain(error, sprint_parser_next_int(parser, &element->pad_smt.thermal_tracks));
        } else if (strcasecmp(statement.name, "THERMAL_TRACKS_WIDTH") == 0) {
            if (found_thermal_tracks_width)
                already_found = true;
            found_thermal_tracks_width |= sprint_chain(error, sprint_parser_next_uint(parser, &element->pad_smt.thermal_tracks_width));
        } else {
            error = SPRINT_ERROR_SYNTAX;
            sprint_throw_format(false, "unknown property: %s", statement.name);
        }

        // Handle already found properties
        if (already_found)
            sprint_warning_format("overwriting duplicate property: %s", statement.name);

        // Destroy the statement
        sprint_check(sprint_parser_statement_destroy(&statement));

        // Handle syntax errors by enabling salvaged mode and ignoring the property
        if (error == SPRINT_ERROR_SYNTAX) {
            sprint_check(sprint_token_unexpected_internal(parser, false));
            *salvaged = true;
            continue;
        }

        // All other errors stop processing
        if (error != SPRINT_ERROR_NONE) {
            sprint_check(sprint_list_destroy(list));
            return sprint_rethrow(error);
        }
    }

    // Make sure that there are all properties
    if (!found_layer | !found_position | !found_width | !found_height | (element->pad_smt.thermal & (!found_thermal_tracks | !found_thermal_tracks_width))) {
        sprint_throw_format(false, "incomplete element: %s", sprint_element_type_to_keyword(SPRINT_ELEMENT_PAD_SMT, false));
        error = SPRINT_ERROR_SYNTAX;
    }

    // Complete the list and check verify full validity
    if (sprint_chain(error, sprint_list_complete(list, &element->pad_smt.link.num_connections,
                                                 (void*) &element->pad_smt.link.connections)) &&
        !sprint_assert(false, sprint_pad_smt_valid(&element->pad_smt)))
        error = SPRINT_ERROR_ASSERTION;

    return sprint_rethrow(error);
}

static sprint_error sprint_parser_next_zone_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
    // Initialize the element
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_zone_default(element, true)))
        return sprint_rethrow(error);
    element->parsed = true;

    // Keep track of found properties
    bool found_layer = false, found_width = false, found_clear = false, found_cutout = false, found_soldermask = false,
            found_hatch = false, found_hatch_auto = false, found_hatch_width = false;

    // Keep a list of points
    sprint_list* list = sprint_list_create(sizeof(*element->zone.points), 16);
    if (list == NULL)
        return SPRINT_ERROR_MEMORY;

    // Read all element properties
    sprint_statement statement;
    while (parser->subsequent) {
        // Read the next value statement
        error = sprint_parser_next_value_internal(parser, &statement, salvaged);
        if (error == SPRINT_ERROR_EOS) {
            error = SPRINT_ERROR_NONE;
            break;
        }
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
            found_layer |= sprint_chain(error, sprint_parser_next_layer(parser, &element->zone.layer));
        } else if (strcasecmp(statement.name, "WIDTH") == 0) {
            if (found_width)
                already_found = true;
            found_width |= sprint_chain(error, sprint_parser_next_size(parser, &element->zone.width));
        } else if (strcasecmp(statement.name, "CLEAR") == 0) {
            if (found_clear)
                already_found = true;
            found_clear |= sprint_chain(error, sprint_parser_next_size(parser, &element->zone.clear));
        } else if (strcasecmp(statement.name, "CUTOUT") == 0) {
            if (found_cutout)
                already_found = true;
            found_cutout |= sprint_chain(error, sprint_parser_next_bool(parser, &element->zone.cutout));
        } else if (strcasecmp(statement.name, "SOLDERMASK") == 0) {
            if (found_soldermask)
                already_found = true;
            found_soldermask |= sprint_chain(error, sprint_parser_next_bool(parser, &element->zone.soldermask));
        } else if (strcasecmp(statement.name, "HATCH") == 0) {
            if (found_hatch)
                already_found = true;
            found_hatch |= sprint_chain(error, sprint_parser_next_bool(parser, &element->zone.hatch));
        } else if (strcasecmp(statement.name, "HATCH_AUTO") == 0) {
            if (found_hatch_auto)
                already_found = true;
            found_hatch_auto |= sprint_chain(error, sprint_parser_next_bool(parser, &element->zone.hatch_auto));
        } else if (strcasecmp(statement.name, "HATCH_WIDTH") == 0) {
            if (found_hatch_width)
                already_found = true;
            found_hatch_width |= sprint_chain(error, sprint_parser_next_size(parser, &element->zone.hatch_width));
        } else {
            error = SPRINT_ERROR_SYNTAX;
            sprint_throw_format(false, "unknown property: %s", statement.name);
        }

        // Handle already found properties
        if (already_found)
            sprint_warning_format("overwriting duplicate property: %s", statement.name);

        // Destroy the statement
        sprint_check(sprint_parser_statement_destroy(&statement));

        // Handle syntax errors by enabling salvaged mode and ignoring the property
        if (error == SPRINT_ERROR_SYNTAX) {
            sprint_check(sprint_token_unexpected_internal(parser, false));
            *salvaged = true;
            continue;
        }

        // All other errors stop processing
        if (error != SPRINT_ERROR_NONE) {
            sprint_check(sprint_list_destroy(list));
            return sprint_rethrow(error);
        }
    }

    // Make sure that there are at least two points and all properties
    if (!found_layer | !found_width | sprint_list_count(list) < 2 | !element->zone.hatch_auto & !found_hatch_width) {
        sprint_throw_format(false, "incomplete element: %s", sprint_element_type_to_keyword(SPRINT_ELEMENT_ZONE, false));
        error = SPRINT_ERROR_SYNTAX;
    }

    // Complete the list and check verify full validity
    if (sprint_chain(error, sprint_list_complete(list, &element->zone.num_points, (void*) &element->zone.points)) &&
        !sprint_assert(false, sprint_zone_valid(&element->zone)))
        error = SPRINT_ERROR_ASSERTION;

    return sprint_rethrow(error);
}

static sprint_error sprint_parser_next_text_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
    // Initialize the element
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_text_default(element, true)))
        return sprint_rethrow(error);
    element->parsed = true;

    // Keep track of found properties
    bool found_layer = false, found_position = false, found_height = false, found_text = false, found_clear = false,
            found_cutout = false, found_soldermask = false, found_style = false, found_thickness = false,
            found_rotation = false, found_mirror_horizontal = false, found_mirror_vertical = false, found_visible = false;

    // Read all element properties
    sprint_statement statement;
    while (parser->subsequent) {
        // Read the next value statement
        error = sprint_parser_next_value_internal(parser, &statement, salvaged);
        if (error == SPRINT_ERROR_EOS) {
            error = SPRINT_ERROR_NONE;
            break;
        }
        if (!sprint_check(error))
            return sprint_rethrow(error);

        // Determine the statement name
        bool already_found = false;
        if (strcasecmp(statement.name, "LAYER") == 0) {
            if (found_layer)
                already_found = true;
            found_layer |= sprint_chain(error, sprint_parser_next_layer(parser, &element->text.layer));
        } else if (strcasecmp(statement.name, "POS") == 0) {
            if (found_position)
                already_found = true;
            found_position |= sprint_chain(error, sprint_parser_next_tuple(parser, &element->text.position));
        } else if (strcasecmp(statement.name, "HEIGHT") == 0) {
            if (found_height)
                already_found = true;
            found_height |= sprint_chain(error, sprint_parser_next_size(parser, &element->text.height));
        } else if (strcasecmp(statement.name, "TEXT") == 0) {
            if (found_text)
                already_found = true;
            found_text |= sprint_chain(error, sprint_parser_next_str(parser, &element->text.text));
        } else if (strcasecmp(statement.name, "CLEAR") == 0) {
            if (found_clear)
                already_found = true;
            found_clear |= sprint_chain(error, sprint_parser_next_size(parser, &element->text.clear));
        } else if (strcasecmp(statement.name, "CUTOUT") == 0) {
            if (found_cutout)
                already_found = true;
            found_cutout |= sprint_chain(error, sprint_parser_next_bool(parser, &element->text.cutout));
        } else if (strcasecmp(statement.name, "SOLDERMASK") == 0) {
            if (found_soldermask)
                already_found = true;
            found_soldermask |= sprint_chain(error, sprint_parser_next_bool(parser, &element->text.soldermask));
        } else if (strcasecmp(statement.name, "STYLE") == 0) {
            if (found_style)
                already_found = true;
            found_style |= sprint_chain(error, sprint_parser_next_text_style(parser, &element->text.style));
        } else if (strcasecmp(statement.name, "THICKNESS") == 0) {
            if (found_thickness)
                already_found = true;
            found_thickness |= sprint_chain(error, sprint_parser_next_text_thickness(parser, &element->text.thickness));
        } else if (strcasecmp(statement.name, "FLATEND") == 0) {
            if (found_rotation)
                already_found = true;
            found_rotation |= sprint_chain(error, sprint_parser_next_angle(parser, &element->text.rotation, SPRINT_PRIM_FORMAT_ANGLE_WHOLE));
        } else if (strcasecmp(statement.name, "MIRROR_HORZ") == 0) {
            if (found_mirror_horizontal)
                already_found = true;
            found_mirror_horizontal |= sprint_chain(error, sprint_parser_next_bool(parser, &element->text.mirror_horizontal));
        } else if (strcasecmp(statement.name, "MIRROR_VERT") == 0) {
            if (found_mirror_vertical)
                already_found = true;
            found_mirror_vertical |= sprint_chain(error, sprint_parser_next_bool(parser, &element->text.mirror_vertical));
        } else if (strcasecmp(statement.name, "VISIBLE") == 0) {
            if (found_visible)
                already_found = true;
            found_visible |= sprint_chain(error, sprint_parser_next_bool(parser, &element->text.visible));
        } else {
            error = SPRINT_ERROR_SYNTAX;
            sprint_throw_format(false, "unknown property: %s", statement.name);
        }

        // Handle already found properties
        if (already_found)
            sprint_warning_format("overwriting duplicate property: %s", statement.name);

        // Destroy the statement
        sprint_check(sprint_parser_statement_destroy(&statement));

        // Handle syntax errors by enabling salvaged mode and ignoring the property
        if (error == SPRINT_ERROR_SYNTAX) {
            sprint_check(sprint_token_unexpected_internal(parser, false));
            *salvaged = true;
            continue;
        }

        // All other errors stop processing
        if (error != SPRINT_ERROR_NONE)
            return sprint_rethrow(error);
    }

    // Make sure that there are all properties
    if (!found_layer | !found_position | !found_height | !found_text) {
        sprint_throw_format(false, "incomplete element: %s", sprint_element_type_to_keyword(SPRINT_ELEMENT_TEXT, false));
        error = SPRINT_ERROR_SYNTAX;
    }

    // Verify full validity
    if (!sprint_assert(false, sprint_text_valid(&element->text)))
        error = SPRINT_ERROR_ASSERTION;

    return sprint_rethrow(error);
}

static sprint_error sprint_parser_next_circle_internal(sprint_parser* parser, sprint_element* element, bool* salvaged)
{
    // Initialize the element
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_circle_default(element, true)))
        return sprint_rethrow(error);
    element->parsed = true;

    // Keep track of found properties
    bool found_layer = false, found_width = false, found_center = false, found_radius = false, found_clear = false,
            found_cutout = false, found_soldermask = false, found_start = false, found_stop = false, found_fill = false;

    // Read all element properties
    sprint_statement statement;
    while (parser->subsequent) {
        // Read the next value statement
        error = sprint_parser_next_value_internal(parser, &statement, salvaged);
        if (error == SPRINT_ERROR_EOS) {
            error = SPRINT_ERROR_NONE;
            break;
        }
        if (!sprint_check(error))
            return sprint_rethrow(error);

        // Determine the statement name
        bool already_found = false;
        if (strcasecmp(statement.name, "LAYER") == 0) {
            if (found_layer)
                already_found = true;
            found_layer |= sprint_chain(error, sprint_parser_next_layer(parser, &element->circle.layer));
        } else if (strcasecmp(statement.name, "WIDTH") == 0) {
            if (found_width)
                already_found = true;
            found_width |= sprint_chain(error, sprint_parser_next_size(parser, &element->circle.width));
        } else if (strcasecmp(statement.name, "CENTER") == 0) {
            if (found_center)
                already_found = true;
            found_center |= sprint_chain(error, sprint_parser_next_tuple(parser, &element->circle.center));
        } else if (strcasecmp(statement.name, "RADIUS") == 0) {
            if (found_radius)
                already_found = true;
            found_radius |= sprint_chain(error, sprint_parser_next_size(parser, &element->circle.radius));
        } else if (strcasecmp(statement.name, "CLEAR") == 0) {
            if (found_clear)
                already_found = true;
            found_clear |= sprint_chain(error, sprint_parser_next_size(parser, &element->circle.clear));
        } else if (strcasecmp(statement.name, "CUTOUT") == 0) {
            if (found_cutout)
                already_found = true;
            found_cutout |= sprint_chain(error, sprint_parser_next_bool(parser, &element->circle.cutout));
        } else if (strcasecmp(statement.name, "SOLDERMASK") == 0) {
            if (found_soldermask)
                already_found = true;
            found_soldermask |= sprint_chain(error, sprint_parser_next_bool(parser, &element->circle.soldermask));
        } else if (strcasecmp(statement.name, "START") == 0) {
            if (found_start)
                already_found = true;
            found_start |= sprint_chain(error, sprint_parser_next_angle(parser, &element->circle.start, SPRINT_PRIM_FORMAT_ANGLE_FINE));
        } else if (strcasecmp(statement.name, "STOP") == 0) {
            if (found_stop)
                already_found = true;
            found_stop |= sprint_chain(error, sprint_parser_next_angle(parser, &element->circle.stop, SPRINT_PRIM_FORMAT_ANGLE_FINE));
        } else if (strcasecmp(statement.name, "FILL") == 0) {
            if (found_fill)
                already_found = true;
            found_fill |= sprint_chain(error, sprint_parser_next_bool(parser, &element->circle.fill));
        } else {
            error = SPRINT_ERROR_SYNTAX;
            sprint_throw_format(false, "unknown property: %s", statement.name);
        }

        // Handle already found properties
        if (already_found)
            sprint_warning_format("overwriting duplicate property: %s", statement.name);

        // Destroy the statement
        sprint_check(sprint_parser_statement_destroy(&statement));

        // Handle syntax errors by enabling salvaged mode and ignoring the property
        if (error == SPRINT_ERROR_SYNTAX) {
            sprint_check(sprint_token_unexpected_internal(parser, false));
            *salvaged = true;
            continue;
        }

        // All other errors stop processing
        if (error != SPRINT_ERROR_NONE)
            return sprint_rethrow(error);
    }

    // Make sure that there are all properties
    if (!found_layer | !found_width | !found_center | !found_radius) {
        sprint_throw_format(false, "incomplete element: %s", sprint_element_type_to_keyword(SPRINT_ELEMENT_CIRCLE, false));
        error = SPRINT_ERROR_SYNTAX;
    }

    // Verify full validity
    if (!sprint_assert(false, sprint_circle_valid(&element->circle)))
        error = SPRINT_ERROR_ASSERTION;

    return sprint_rethrow(error);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
static sprint_error sprint_parser_next_element_internal(sprint_parser* parser, sprint_element** element,
                                                        bool* salvaged, sprint_element_type parent, int depth);

static sprint_error sprint_parser_next_component_internal(sprint_parser* parser, sprint_element* element,
                                                          bool* salvaged, int depth)
{
    // Initialize the element
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_component_default(element, true)))
        return sprint_rethrow(error);
    element->parsed = true;

    // Keep track of found properties
    bool found_comment = false, found_use_pickplace = false, found_package = false, found_rotation = false;

    // Read all element properties
    sprint_statement statement;
    while (parser->subsequent) {
        // Read the next value statement
        error = sprint_parser_next_value_internal(parser, &statement, salvaged);
        if (error == SPRINT_ERROR_EOS) {
            error = SPRINT_ERROR_NONE;
            break;
        }
        if (!sprint_check(error))
            return sprint_rethrow(error);

        // Determine the statement name
        bool already_found = false;
        if (strcasecmp(statement.name, "COMMENT") == 0) {
            if (found_comment)
                already_found = true;
            found_comment |= sprint_chain(error, sprint_parser_next_str(parser, &element->component.comment));
        } else if (strcasecmp(statement.name, "USE_PICKPLACE") == 0) {
            if (found_use_pickplace)
                already_found = true;
            found_use_pickplace |= sprint_chain(error, sprint_parser_next_bool(parser, &element->component.use_pickplace));
        } else if (strcasecmp(statement.name, "PACKAGE") == 0) {
            if (found_package)
                already_found = true;
            found_package |= sprint_chain(error, sprint_parser_next_str(parser, &element->component.package));
        } else if (strcasecmp(statement.name, "ROTATION") == 0) {
            if (found_rotation)
                already_found = true;
            found_rotation |= sprint_chain(error, sprint_parser_next_angle(parser, &element->component.rotation, SPRINT_PRIM_FORMAT_ANGLE_WHOLE));
        } else {
            error = SPRINT_ERROR_SYNTAX;
            sprint_throw_format(false, "unknown property: %s", statement.name);
        }

        // Handle already found properties
        if (already_found)
            sprint_warning_format("overwriting duplicate property: %s", statement.name);

        // Destroy the statement
        sprint_check(sprint_parser_statement_destroy(&statement));

        // Handle syntax errors by enabling salvaged mode and ignoring the property
        if (error == SPRINT_ERROR_SYNTAX) {
            sprint_check(sprint_token_unexpected_internal(parser, false));
            *salvaged = true;
            continue;
        }

        // All other errors stop processing
        if (error != SPRINT_ERROR_NONE)
            return sprint_rethrow(error);
    }

    // Read the elements
    bool found_text_id = false, found_text_value = false;
    sprint_list* list = sprint_list_create(sizeof(sprint_element), 16);
    if (list == NULL)
        return SPRINT_ERROR_MEMORY;
    sprint_element* child = NULL;
    bool child_salvaged = false;
    while (true) {
        // Read the next element
        error = sprint_parser_next_element_internal(parser, &child, &child_salvaged, SPRINT_ELEMENT_COMPONENT, depth + 1);
        if (error == SPRINT_ERROR_EOE || !sprint_check(error))
            break;

        // Check, if the element is an ID or value text
        if (child->type == SPRINT_ELEMENT_TEXT) {
            bool special_text_found = true, already_found = false;
            sprint_text_type subtype = child->text.subtype;
            switch (subtype) {
                case SPRINT_TEXT_ID:
                    if (found_text_id)
                        already_found = true;
                    found_text_id = true;
                    element->component.text_id = child;
                    break;
                case SPRINT_TEXT_VALUE:
                    if (found_text_value)
                        already_found = true;
                    found_text_value = true;
                    element->component.text_value = child;
                    break;
                default:
                    special_text_found = false;
                    break;
            }

            // Handle already found properties
            if (already_found)
                sprint_warning_format("overwriting duplicate property: %s", statement.name);

            // Go to the next element, if a special text was found
            if (special_text_found)
                continue;
        }

        // Add the element
        sprint_check(sprint_list_add(list, child));
    }

    // Ignore end of element errors and destroy the list on other errors
    if (error == SPRINT_ERROR_EOE)
        error = SPRINT_ERROR_NONE;
    else if (error != SPRINT_ERROR_NONE)
        sprint_check(sprint_list_destroy(list));

    // Make sure that there are all properties
    if (!found_text_id || !found_text_value) {
        sprint_throw_format(false, "incomplete element: %s", sprint_element_type_to_keyword(SPRINT_ELEMENT_COMPONENT, false));
        error = SPRINT_ERROR_SYNTAX;
    }

    // Complete the list and check verify full validity
    if (sprint_chain(error, sprint_list_complete(list, &element->component.num_elements, (void*) &element->component.elements)) &&
        !sprint_assert(false, sprint_component_valid(&element->component)))
        error = SPRINT_ERROR_ASSERTION;

    return sprint_rethrow(error);
}

static sprint_error sprint_parser_next_group_internal(sprint_parser* parser, sprint_element* element,
                                                      bool* salvaged, int depth)
{
    // Initialize the element
    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_group_default(element, true)))
        return sprint_rethrow(error);
    element->parsed = true;

    // Make sure that there are no properties
    if (parser->subsequent) {
        sprint_statement statement;
        error = sprint_parser_next_value_internal(parser, &statement, salvaged);
        if (error == SPRINT_ERROR_EOS) {
            error = SPRINT_ERROR_NONE;
        }
        if (sprint_check(error)) {
            error = SPRINT_ERROR_SYNTAX;
            sprint_throw_format(false, "unknown property: %s", statement.name);
        }

        // Destroy the statement
        sprint_check(sprint_parser_statement_destroy(&statement));

        // If something went wrong, rethrow
        return sprint_rethrow(error);
    }

    // Read the elements
    sprint_list* list = sprint_list_create(sizeof(sprint_element), 16);
    if (list == NULL)
        return SPRINT_ERROR_MEMORY;
    sprint_element* child = NULL;
    bool child_salvaged = false;
    while (true) {
        // Read the next element
        error = sprint_parser_next_element_internal(parser, &child, &child_salvaged, SPRINT_ELEMENT_GROUP, depth + 1);
        if (error == SPRINT_ERROR_EOE || !sprint_check(error))
            break;

        // Add the element
        sprint_check(sprint_list_add(list, child));
    }

    // Ignore end of element errors and destroy the list on other errors
    if (error == SPRINT_ERROR_EOE)
        error = SPRINT_ERROR_NONE;
    else if (error != SPRINT_ERROR_NONE)
        sprint_check(sprint_list_destroy(list));

    // Complete the list and check verify full validity
    if (sprint_chain(error, sprint_list_complete(list, &element->group.num_elements, (void*) &element->group.elements)) &&
        !sprint_assert(false, sprint_group_valid(&element->group)))
        error = SPRINT_ERROR_ASSERTION;

    return sprint_rethrow(error);
}

static sprint_error sprint_parser_next_element_internal(sprint_parser* parser, sprint_element** element,
                                                        bool* salvaged, sprint_element_type parent, int depth)
{
    if (parser == NULL || element == NULL || salvaged == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (depth < 0 || depth > SPRINT_ELEMENT_DEPTH) return SPRINT_ERROR_RECURSION;

    // Clear the salvaged flag
    *salvaged = false;

    // Allocate the element
    *element = malloc(sizeof(**element));
    if (*element == NULL)
        return SPRINT_ERROR_MEMORY;

    // Keep reading statements until there is one that can be read
    sprint_statement statement;
    sprint_error error;
    while (true) {
        // Read the next statement
        error = sprint_parser_next_statement(parser, &statement, *salvaged);
        if (error == SPRINT_ERROR_EOS)
            continue;
        if (error == SPRINT_ERROR_EOF || !sprint_check(error))
            break;

        // Check, if the flags are valid for a keyword
        const sprint_statement_flags bad_flags = SPRINT_STATEMENT_FLAG_VALUE,
            required_flags = SPRINT_STATEMENT_FLAG_FIRST | SPRINT_STATEMENT_FLAG_NAME;
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
        memset(*element, 0, sizeof(**element));

        // If the depth is at least one and the parent a component, try to match the other text types first
        bool element_salvaged = false;
        sprint_text_type text_type = 0;
        if (depth > 0 && parent == SPRINT_ELEMENT_COMPONENT &&
            sprint_text_type_from_keyword(&text_type, statement.name)) {
            // Destroy the statement, read the text and update the subtype
            sprint_check(sprint_parser_statement_destroy(&statement));
            if (sprint_chain(error, sprint_parser_next_text_internal(parser, *element, &element_salvaged)))
                (*element)->text.subtype = text_type;

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

        // If it failed, or if this is a closing keyword for the wrong element type, emit a warning
        if (!success || closing && depth < 1 || closing && parent != type) {
            // Emit a warning, turn on salvaged mode and move on to the next element
            *salvaged = true;
            sprint_token_unexpected_internal(parser, true);
            continue;
        }

        // For valid closing keywords, return an end of element error
        if (closing)
            return SPRINT_ERROR_EOE;

        // And dispatch to the correct parser
        switch (type) {
            case SPRINT_ELEMENT_TRACK:
                sprint_chain(error, sprint_parser_next_track_internal(parser, *element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_PAD_THT:
                sprint_chain(error, sprint_parser_next_pad_tht_internal(parser, *element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_PAD_SMT:
                sprint_chain(error, sprint_parser_next_pad_smt_internal(parser, *element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_ZONE:
                sprint_chain(error, sprint_parser_next_zone_internal(parser, *element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_TEXT:
                sprint_chain(error, sprint_parser_next_text_internal(parser, *element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_CIRCLE:
                sprint_chain(error, sprint_parser_next_circle_internal(parser, *element, &element_salvaged));
                break;
            case SPRINT_ELEMENT_COMPONENT:
                sprint_chain(error, sprint_parser_next_component_internal(parser, *element, &element_salvaged, depth));
                break;
            case SPRINT_ELEMENT_GROUP:
                sprint_chain(error, sprint_parser_next_group_internal(parser, *element, &element_salvaged, depth));
                break;
            default:
                sprint_check(sprint_element_destroy(*element));
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

    // Destroy the element, if there was an error
    if (error != SPRINT_ERROR_NONE)
        sprint_check(sprint_element_destroy(*element));

    // And just return
    return sprint_rethrow(error);
}
#pragma clang diagnostic pop

sprint_error sprint_parser_next_element(sprint_parser* parser, sprint_element** element, bool* salvaged)
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
