//
// libsprintpcb: plugin representation and life-cycle management
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "plugin.h"
#include "token.h"
#include "parser.h"
#include "pcb.h"
#include "list.h"
#include "stringbuilder.h"
#include "errors.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#include <errno.h>
#endif

static bool sprint_plugin_parse_int_internal(int* output, const char* input);
static bool sprint_plugin_parse_language_internal(int* output, const char* input);
static sprint_error sprint_plugin_parse_flags_internal(int argc, const char* argv[]);
static sprint_error sprint_plugin_parse_input_internal();

const char* SPRINT_LANGUAGE_NAMES[] = {
        [SPRINT_LANGUAGE_ENGLISH] = "English",
        [SPRINT_LANGUAGE_GERMAN] = "German",
        [SPRINT_LANGUAGE_FRENCH] = "French"
};

bool sprint_language_valid(sprint_language language)
{
    return language >= SPRINT_LANGUAGE_ENGLISH && language <= SPRINT_LANGUAGE_FRENCH;
}

static struct sprint_plugin {
    sprint_plugin_state state;
    sprint_language language;
    sprint_operation operation;
    sprint_process_id process;
    sprint_pcb pcb;
    bool selection;
    const char* input;
    const char* output;
} sprint_plugin = {.state = SPRINT_PLUGIN_STATE_UNINITIALIZED};

const char* SPRINT_OPERATION_NAMES[] = {
        [SPRINT_OPERATION_NONE] = "no operation",
        [SPRINT_OPERATION_REPLACE_ABSOLUTE] = "replace absolute",
        [SPRINT_OPERATION_ADD_ABSOLUTE] = "add absolute",
        [SPRINT_OPERATION_REPLACE_RELATIVE] = "replace relative",
        [SPRINT_OPERATION_ADD_RELATIVE] = "add relative"
};

bool sprint_operation_valid(sprint_operation operation, bool failed)
{
    if (failed && operation >= SPRINT_OPERATION_FAILED_START && operation <= SPRINT_OPERATION_FAILED_END)
        return true;

    return operation >= SPRINT_OPERATION_NONE && operation <= SPRINT_OPERATION_ADD_RELATIVE;
}

const char* SPRINT_PLUGIN_STATE_NAMES[] = {
        [SPRINT_PLUGIN_STATE_UNINITIALIZED] = "uninitialized",
        [SPRINT_PLUGIN_STATE_PARSING_FLAGS] = "parsing flags",
        [SPRINT_PLUGIN_STATE_PARSING_INPUT] = "parsing input",
        [SPRINT_PLUGIN_STATE_PROCESSING] = "processing",
        [SPRINT_PLUGIN_STATE_WRITING_OUTPUT] = "writing output",
        [SPRINT_PLUGIN_STATE_COMPLETED] = "completed"
};

#ifdef WIN32
const char SPRINT_FLAG_PREFIX = '/';
#else
const char SPRINT_FLAG_PREFIX = '-';
#endif

const char SPRINT_FLAG_DELIMITER = ':';
const char* SPRINT_OUTPUT_SUFFIX = "_out";

static bool sprint_plugin_parse_int_internal(int* output, const char* input)
{
    if (*input == 0) return false;

    errno = 0;
    char* number_end = NULL;
    long number = strtol(input, &number_end, 10);
    if (errno != 0 || number_end == NULL || *number_end != 0)
        return false;

    *output = number;
    return true;
}

static bool sprint_plugin_parse_language_internal(int* output, const char* input)
{
    if (*input == 0) return false;

    if (strcasecmp(input, "UK") == 0)
        *output = SPRINT_LANGUAGE_ENGLISH;
    else if (strcasecmp(input, "DE") == 0)
        *output = SPRINT_LANGUAGE_GERMAN;
    else if (strcasecmp(input, "FR") == 0)
        *output = SPRINT_LANGUAGE_FRENCH;
    else
        return false;

    return true;
}

static sprint_error sprint_plugin_parse_flags_internal(int argc, const char* argv[])
{
    // Check the state
    if (sprint_plugin.state != SPRINT_PLUGIN_STATE_PARSING_FLAGS)
        return SPRINT_ERROR_STATE_INVALID;

    // Check, if the number of arguments could be enough
    if (argc < 4) {
        sprint_throw(false, "too few arguments");
        return SPRINT_ERROR_PLUGIN_FLAGS_MISSING;
    }

    // Skip the first argument
    argc--;
    argv++;

    // Keep track of the flags passed (defaults: language=UK, x=0, y=0, all=false, pid=0; rest is required)
    bool found_language = false, found_width = false, found_height = false, found_x = false, found_y = false,
            found_grid = false, found_flags = false, found_all = false, found_pid = false;

    // Get the input file name
    const char* input_path = NULL;
    if (*argv != NULL && **argv != 0 && **argv != SPRINT_FLAG_PREFIX)
        input_path = *argv;

    // Parse the flags
    int language = 0, pcb_width = 0, pcb_height = 0, pcb_origin_x = 0, pcb_origin_y = 0, grid = 0, flags = 0, pid = 0;
    for (argc--, argv++; argc > 0; argc--, argv++) {
        // Skip arguments that are null or empty
        if (*argv == NULL || **argv == 0) continue;

        // Make sure that the flag prefix is always present
        if (**argv != SPRINT_FLAG_PREFIX) {
            sprint_throw_format(false, "unexpected argument: %s", *argv);
            return SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX;
        }

        // Get the flag character and convert it to upper-case
        char flag_chr = (*argv)[1];
        if (flag_chr >= 'a' && flag_chr <= 'z') {
            flag_chr -= 'a';
            flag_chr += 'A';
        }

        // Make sure that the flag character is a letter
        if (flag_chr < 'A' || flag_chr > 'Z') {
            sprint_throw_format(false, "%s flag: %s", flag_chr == 0 ? "incomplete" : "invalid", *argv);
            return SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX;
        }

        bool already_found = false;
        int* target_value = NULL;
        bool (*flag_parser)(int* output, const char* input) = sprint_plugin_parse_int_internal;
        switch (flag_chr) {
            case 'W':
                if (found_width)
                    already_found = true;
                target_value = &pcb_width;
                found_width = true;
                break;

            case 'H':
                if (found_height)
                    already_found = true;
                target_value = &pcb_height;
                found_height = true;
                break;

            case 'L':
                if (found_language)
                    already_found = true;
                target_value = &language;
                flag_parser = sprint_plugin_parse_language_internal;
                found_language = true;
                break;

            case 'X':
                if (found_x)
                    already_found = true;
                target_value = &pcb_origin_x;
                found_x = true;
                break;

            case 'Y':
                if (found_y)
                    already_found = true;
                target_value = &pcb_origin_y;
                found_y = true;
                break;

            case 'R':
                if (found_grid)
                    already_found = true;
                target_value = &grid;
                found_grid = true;
                break;

            case 'M':
                if (found_flags)
                    already_found = true;
                target_value = &flags;
                found_flags = true;
                break;

            case 'P':
                if (found_pid)
                    already_found = true;
                target_value = &pid;
                found_pid = true;
                break;

            case 'A':
                if (found_all)
                    already_found = true;
                else
                    sprint_debug("importing entire PCB");
                found_all = true;
                break;

            default:
                sprint_warning_format("unknown flag: %s", *argv);
                return SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX;
        }

        // Emit a warning, if the flag has already been read
        if (already_found)
            sprint_warning_format("%s flag: %s", target_value == NULL ? "duplicate" : "overwriting", *argv);

        // Skip flag parsing for value-less flags
        if (target_value == NULL) {
            // Make sure that the flag ends after the letter
            if ((*argv)[2] != 0)
                sprint_warning_format("ignored flag value: %s", *argv);

            // And just move on to the next flag
            continue;
        }

        // Make sure that there is a delimiter
        if ((*argv)[2] != SPRINT_FLAG_DELIMITER) {
            sprint_throw_format(false, "incomplete flag: %s", *argv);
            return SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX;
        }

        // Parse the remainder of the flag as its value using the parser
        const char* raw_value = *argv + 3;
        if (!flag_parser(target_value, raw_value)) {
            sprint_throw_format(false, "invalid flag value: %s", *argv);
            return SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX;
        }
    }

    // The error_separator in the error or warning output between missing arguments
    const char* error_separator = ", ";

    // Make sure the required parameters are present and emit an error, if not
    sprint_stringbuilder* builder = sprint_stringbuilder_create(32);
    if (!sprint_assert(false, builder != NULL))
        return SPRINT_ERROR_ASSERTION;
    sprint_error error = SPRINT_ERROR_NONE;
    if (input_path == NULL)
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "input file"));
    if (!found_width) {
        if (builder->count > 0)
            sprint_chain(error, sprint_stringbuilder_put_str(builder, error_separator));
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "width (W)"));
    }
    if (!found_height) {
        if (builder->count > 0)
            sprint_chain(error, sprint_stringbuilder_put_str(builder, error_separator));
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "height (H)"));
    }
    if (error != SPRINT_ERROR_NONE) {
        sprint_check(sprint_stringbuilder_destroy(builder));
        return sprint_rethrow(error);
    }
    if (builder->count > 0) {
        char* flags_str = sprint_stringbuilder_complete(builder);
        if (sprint_assert(false, flags_str != NULL)) {
            sprint_throw_format(false, "could not find required argument(s): %s", flags_str);
            free(flags_str);
        }
        return input_path == NULL ? SPRINT_ERROR_PLUGIN_INPUT_MISSING : SPRINT_ERROR_PLUGIN_FLAGS_MISSING;
    }

    // Emit a warning for all missing optional flags except for /A
    if (!found_language) {
        language = SPRINT_LANGUAGE_ENGLISH;
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "language (L)"));
    }
    if (!found_x) {
        if (builder->count > 0)
            sprint_chain(error, sprint_stringbuilder_put_str(builder, error_separator));
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "origin X (X)"));
    }
    if (!found_y) {
        if (builder->count > 0)
            sprint_chain(error, sprint_stringbuilder_put_str(builder, error_separator));
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "origin Y (Y)"));
    }
    if (!found_grid) {
        grid = sprint_dist_um(1270);
        if (builder->count > 0)
            sprint_chain(error, sprint_stringbuilder_put_str(builder, error_separator));
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "grid (R)"));
    }
    if (!found_flags) {
        if (builder->count > 0)
            sprint_chain(error, sprint_stringbuilder_put_str(builder, error_separator));
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "flags (M)"));
    }
    if (!found_pid) {
        if (builder->count > 0)
            sprint_chain(error, sprint_stringbuilder_put_str(builder, error_separator));
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "process ID (P)"));
    }
    if (error != SPRINT_ERROR_NONE) {
        sprint_check(sprint_stringbuilder_destroy(builder));
        return sprint_rethrow(error);
    }
    if (builder->count > 0) {
        char* flags_str = sprint_stringbuilder_complete(builder);
        if (sprint_assert(false, flags_str != NULL)) {
            sprint_warning_format("defaulting missing argument(s): %s", flags_str);
            free(flags_str);
        }

        // Create a new builder
        builder = sprint_stringbuilder_create(32);
        if (!sprint_assert(false, builder != NULL))
            return SPRINT_ERROR_ASSERTION;
    }

    // Make sure the values are valid
    bool all_valid = true;
    if (!sprint_language_valid(language)) {
        all_valid = false;
        sprint_throw(false, "language invalid");
    }
    if (!sprint_size_valid(pcb_width)) {
        all_valid = false;
        sprint_throw(false, "width invalid");
    }
    if (!sprint_size_valid(pcb_height)) {
        all_valid = false;
        sprint_throw(false, "height invalid");
    }
    if (!sprint_dist_valid(pcb_origin_x)) {
        all_valid = false;
        sprint_throw(false, "origin x invalid");
    }
    if (!sprint_dist_valid(pcb_origin_y)) {
        all_valid = false;
        sprint_throw(false, "origin y invalid");
    }
    if (!all_valid) {
        sprint_check(sprint_stringbuilder_destroy(builder));
        return SPRINT_ERROR_PLUGIN_FLAGS_SYNTAX;
    }

    // Determine the last index of a slash or dot in the input path
    const char *input_ptr, *input_tail = NULL;
    bool input_tail_slash = false;
    for (input_ptr = input_path; *input_ptr != 0; input_ptr++)
        switch (*input_ptr) {
            case '\\':
            case '/':
                input_tail_slash = true;
                input_tail = input_ptr;
                break;

            case '.':
                if (input_tail != NULL && !input_tail_slash)
                    continue;

                input_tail_slash = false;
                input_tail = input_ptr;
                break;

            default:
                if (input_tail != NULL || input_ptr[1] != 0)
                    continue;

                input_tail = input_ptr;
                input_tail_slash = true;
                break;
        }

    // Copy the first part of the path up to and excluding the dot or up to and including the slash
    int input_head_length = (int) (input_tail - input_path);
    if (input_tail_slash)
        input_head_length++;
    if (input_head_length > 0 &&
        !sprint_chain(error, sprint_stringbuilder_put_str_range(builder, input_path, input_head_length)))
    {
        sprint_check(sprint_stringbuilder_destroy(builder));
        return sprint_rethrow(error);
    }

    // Append the output filename suffix
    if (!sprint_chain(error, sprint_stringbuilder_put_str(builder, SPRINT_OUTPUT_SUFFIX))) {
        sprint_check(sprint_stringbuilder_destroy(builder));
        return sprint_rethrow(error);
    }

    // Append the rest of the filename
    int input_tail_length = (int) (input_ptr - input_path) - input_head_length;
    if (!sprint_assert(false, input_tail_length >= 0))
    {
        sprint_check(sprint_stringbuilder_destroy(builder));
        return SPRINT_ERROR_ASSERTION;
    }
    if (input_tail_length > 0 &&
        !sprint_chain(error, sprint_stringbuilder_put_str(builder, input_path + input_head_length)))
    {
        sprint_check(sprint_stringbuilder_destroy(builder));
        return sprint_rethrow(error);
    }

    // Complete the builder and store the input and output path
    sprint_plugin.input = input_path;
    sprint_plugin.output = sprint_stringbuilder_complete(builder);
    if (!sprint_assert(false, sprint_plugin.output != NULL))
        return SPRINT_ERROR_ASSERTION;

    // Store the values into the struct
    sprint_plugin.language = language;
    sprint_plugin.process = pid;
    sprint_plugin.selection = !found_all;
    sprint_plugin.pcb.width = pcb_width;
    sprint_plugin.pcb.height = pcb_height;
    sprint_plugin.pcb.grid = sprint_grid_of(sprint_tuple_of(pcb_origin_x, pcb_origin_y), grid, grid);
    sprint_plugin.pcb.flags = flags;

    return SPRINT_ERROR_NONE;
}

static sprint_error sprint_plugin_parse_input_internal()
{
    // Check the state
    if (sprint_plugin.state != SPRINT_PLUGIN_STATE_PARSING_INPUT)
        return SPRINT_ERROR_STATE_INVALID;

    // Open the input file
    FILE* file = fopen(sprint_plugin.input, "r");
    if (file == NULL) {
        sprint_throw_format(false, "error opening file for reading: %s", strerror(errno));
        return SPRINT_ERROR_IO;
    }

    // Create the tokenizer
    sprint_tokenizer* tokenizer = sprint_tokenizer_from_file(file, sprint_plugin.input, true);
    sprint_assert(true, tokenizer != NULL);

    // Create the parser
    sprint_parser* parser = sprint_parser_create(tokenizer);
    sprint_assert(true, parser != NULL);

    // Create an element list
    sprint_list* list = sprint_list_create(sizeof(*sprint_plugin.pcb.elements), 64);
    sprint_assert(true, list);

    // Clear the salvaged flag
    sprint_plugin.pcb.salvaged = false;

    // Parse all elements
    sprint_error error;
    sprint_element* element;
    while (true) {
        // Read the element
        error = sprint_parser_next_element(parser, &element, &sprint_plugin.pcb.salvaged);

        // Handle EOF
        if (error == SPRINT_ERROR_EOF) {
            error = SPRINT_ERROR_NONE;
            break;
        }

        // Handle other errors and add the element
        if (!sprint_check(error) || !sprint_chain(error, sprint_list_add(list, element)))
            break;
    }

    // Complete the element list, if it succeeded
    if (error == SPRINT_ERROR_NONE)
        sprint_require(sprint_list_complete(list, &sprint_plugin.pcb.num_elements, (void*) &sprint_plugin.pcb.elements));
    else
        sprint_require(sprint_list_destroy(list));

    // Destroy the parser (and thus also the tokenizer, and close the file)
    sprint_require(sprint_parser_destroy(parser, true, NULL));

    return sprint_rethrow(error);
}

sprint_error sprint_plugin_begin(int argc, const char* argv[])
{
    if (argv == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Clear the plugin struct
    memset(&sprint_plugin, 0, sizeof(sprint_plugin));

    // Parse the flags
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_plugin.state = SPRINT_PLUGIN_STATE_PARSING_FLAGS;
    if (!sprint_chain(error, sprint_plugin_parse_flags_internal(argc, argv)))
        return sprint_rethrow(error);

    // Parse the input
    sprint_plugin.state = SPRINT_PLUGIN_STATE_PARSING_INPUT;
    if (!sprint_chain(error, sprint_plugin_parse_input_internal()))
        return sprint_rethrow(error);

    // Finally, update the state to processing and allow the plugin to run
    sprint_plugin.state = SPRINT_PLUGIN_STATE_PROCESSING;
    return SPRINT_ERROR_NONE;
}

void sprint_plugin_bail(int error)
{
    // Clamp the error code
    sprint_operation operation = SPRINT_OPERATION_FAILED_PLUGIN + error;
    if (!sprint_operation_valid(operation, true) || operation < SPRINT_OPERATION_FAILED_PLUGIN)
        operation = SPRINT_OPERATION_FAILED_END;

    // Update the state
    sprint_plugin.state = SPRINT_PLUGIN_STATE_COMPLETED;

    // And exit
    exit(operation);
}

sprint_error sprint_plugin_end(sprint_operation operation)
{
    // Check the operation
    if (!sprint_operation_valid(operation, false))
        return SPRINT_ERROR_ARGUMENT_RANGE;

    // Check the state
    if (sprint_plugin.state != SPRINT_PLUGIN_STATE_PROCESSING)
        return SPRINT_ERROR_STATE_INVALID;

    // Update the state
    sprint_plugin.state = SPRINT_PLUGIN_STATE_WRITING_OUTPUT;

    // Handle operations that output data
    if (operation != SPRINT_OPERATION_NONE) {
        // Open the output file
        FILE* file = fopen(sprint_plugin.output, "w");
        if (file == NULL) {
            sprint_throw_format(false, "error opening file for writing: %s", strerror(errno));
            return SPRINT_ERROR_IO;
        }

        // Create the output
        sprint_output* output = sprint_output_create_file(file, true);
        sprint_assert(true, output != NULL);

        // Output all elements
        sprint_error error = SPRINT_ERROR_NONE;
        for (int index = 0; index < sprint_plugin.pcb.num_elements; index++)
            if (!sprint_chain(error, sprint_element_output(&sprint_plugin.pcb.elements[index], output, SPRINT_PRIM_FORMAT_RAW)))
                break;

        // Destroy the output (and thus close the file)
        sprint_require(sprint_output_destroy(output, NULL));

        // Check, if writing succeeded
        if (error == SPRINT_ERROR_SYNTAX)
            error = SPRINT_ERROR_PLUGIN_INPUT_SYNTAX;
        if (error != SPRINT_ERROR_NONE)
            return sprint_rethrow(error);
    }

    // Update the state
    sprint_plugin.state = SPRINT_PLUGIN_STATE_COMPLETED;

    // And exit
    exit(operation);
}

sprint_error sprint_plugin_output(sprint_output* output)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_output_put_str(output, "sprint_plugin{state="));
    sprint_chain(error, sprint_output_put_str(output, SPRINT_PLUGIN_STATE_NAMES[sprint_plugin.state]));
    sprint_chain(error, sprint_output_put_str(output, ", language="));
    sprint_chain(error, sprint_output_put_str(output, SPRINT_LANGUAGE_NAMES[sprint_plugin.language]));
    sprint_chain(error, sprint_output_put_str(output, ", operation="));
    sprint_chain(error, sprint_output_put_str(output, SPRINT_OPERATION_NAMES[sprint_plugin.operation]));
    sprint_chain(error, sprint_output_put_str(output, ", selection="));
    sprint_chain(error, sprint_bool_output(sprint_plugin.selection, output));
    sprint_chain(error, sprint_output_put_str(output, ", pcb="));
    sprint_chain(error, sprint_pcb_output(&sprint_plugin.pcb, output));
    sprint_chain(error, sprint_output_format(output, ", process=%p", sprint_plugin.process));
    sprint_chain(error, sprint_output_format(output, ", input=%s", sprint_plugin.input));
    sprint_chain(error, sprint_output_format(output, ", output=%s", sprint_plugin.output));
    sprint_chain(error, sprint_output_put_chr(output, '}'));

    return sprint_rethrow(error);
}

sprint_pcb* sprint_plugin_get_pcb(void)
{
    return &sprint_plugin.pcb;
}

bool sprint_plugin_is_selection(void)
{
    return sprint_plugin.selection;
}

sprint_plugin_state sprint_plugin_get_state(void)
{
    if (sprint_plugin.state <= SPRINT_PLUGIN_STATE_UNINITIALIZED ||
        sprint_plugin.state > SPRINT_PLUGIN_STATE_COMPLETED)
        return SPRINT_PLUGIN_STATE_UNINITIALIZED;

    return sprint_plugin.state;
}

sprint_language sprint_plugin_get_language(void)
{
    return sprint_plugin.language;
}

sprint_process_id sprint_plugin_get_process(void)
{
    return sprint_plugin.process;
}

const char* sprint_plugin_get_input(void)
{
    return sprint_plugin.input;
}

const char* sprint_plugin_get_output(void)
{
    return sprint_plugin.output;
}

int sprint_plugin_get_exit_code(void)
{
    return (int) (SPRINT_OPERATION_FAILED_LIBRARY + sprint_plugin_get_state());
}
