//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "plugin.h"
#include "stringbuilder.h"

#include <stdbool.h>
#include <errno.h>

bool sprint_plugin_parse_int_internal(int* output, const char* input);
bool sprint_plugin_parse_language_internal(int* output, const char* input);
sprint_error sprint_plugin_parse_flags_internal(int argc, const char* argv[]);
sprint_error sprint_plugin_parse_input_internal();

const char* SPRINT_LANGUAGE_NAMES[] = {
        [SPRINT_LANGUAGE_ENGLISH] = "English",
        [SPRINT_LANGUAGE_GERMAN] = "German",
        [SPRINT_LANGUAGE_FRENCH] = "French"
};

static struct sprint_plugin {
    sprint_plugin_state state;
    sprint_language language;
    sprint_operation operation;
    sprint_process_id process;
    sprint_pcb pcb;
    bool selection;
    FILE* input;
    char* output;
} sprint_plugin = {0};

const char* SPRINT_OPERATION_NAMES[] = {
        [SPRINT_OPERATION_NONE] = "no operation",
        [SPRINT_OPERATION_REPLACE_ABSOLUTE] = "replace absolute",
        [SPRINT_OPERATION_ADD_ABSOLUTE] = "add absolute",
        [SPRINT_OPERATION_REPLACE_RELATIVE] = "replace relative",
        [SPRINT_OPERATION_ADD_RELATIVE] = "add relative"
};

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

bool sprint_plugin_parse_int_internal(int* output, const char* input)
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

bool sprint_plugin_parse_language_internal(int* output, const char* input)
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

sprint_error sprint_plugin_parse_flags_internal(int argc, const char* argv[])
{
    // Check, if the number of arguments could be enough
    if (argc < 3 || argc > 10) {
        sprint_throw(false, "bad number of arguments");
        return SPRINT_ERROR_PLUGIN_FLAGS_MISSING;
    }

    // Skip the first argument
    argc--;
    argv++;

    // Keep track of the flags passed (defaults: language=UK, x=0, y=0, all=false, pid=0; rest is required)
    bool found_language = false, found_width = false, found_height = false, found_x = false, found_y = false,
            found_flags = false, found_all = false, found_pid = false;

    // Get the input file name
    const char* input_path = NULL;
    if (*argv != NULL && **argv != 0 && **argv != SPRINT_FLAG_PREFIX)
        input_path = *argv;

    // Parse the flags
    int language = 0, pcb_width = 0, pcb_height = 0, pcb_origin_x = 0, pcb_origin_y = 0, flags = 0, pid = 0;
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
                sprint_throw_format(false, "unknown flag: %s", *argv);
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
    if (builder->count > 0) {
        char* flags_str = sprint_stringbuilder_complete(builder);
        sprint_assert(true, flags_str != NULL);
        sprint_throw_format(false, "could not find required argument(s): %s", flags_str);
        free(flags_str);
        return input_path == NULL ? SPRINT_ERROR_PLUGIN_INPUT_MISSING : SPRINT_ERROR_PLUGIN_FLAGS_MISSING;
    }

    // Emit a warning for all missing optional flags except for /A
    if (!found_language)
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "language (L)"));
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
    if (builder->count > 0) {
        char* flags_str = sprint_stringbuilder_complete(builder);
        sprint_assert(true, flags_str != NULL);
        sprint_warning_format("defaulting missing argument(s): %s", flags_str);
        free(flags_str);
    }

    // Try to open the input file
    sprint_plugin.input;

    // Determine the name of the output file
    sprint_plugin.output;

    // Store the values into the struct
    sprint_plugin.language = language;
    sprint_plugin.process = pid;
    sprint_plugin.pcb.width = pcb_width;
    sprint_plugin.pcb.height = pcb_height;
    sprint_plugin.pcb.origin = sprint_tuple_of(pcb_origin_x, pcb_origin_y);
    sprint_plugin.pcb.flags = flags;

    return SPRINT_ERROR_NONE;
}

sprint_error sprint_plugin_parse_input_internal()
{
    // TODO
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_plugin_begin(int argc, const char* argv[])
{
    if (argv == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // TODO: close output, if state is not uninitialized; or open output only in the end (better)

    // Clear the plugin struct
    memset(&sprint_plugin, 0, sizeof(sprint_plugin));

    // Parse the flags
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_plugin.state = SPRINT_PLUGIN_STATE_PARSING_FLAGS;
    if (!sprint_chain(error, sprint_plugin_parse_flags_internal(argc, argv)))
        return error;

    // Parse the input
    sprint_plugin.state = SPRINT_PLUGIN_STATE_PARSING_INPUT;
    if (!sprint_chain(error, sprint_plugin_parse_input_internal()))
        return error;

    // Finally, update the state to processing and allow the plugin to run
    sprint_plugin.state = SPRINT_PLUGIN_STATE_PROCESSING;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_plugin_print(FILE* stream)
{
    if (stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(63);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = sprint_plugin_string(builder);
    if (error == SPRINT_ERROR_NONE)
        return sprint_stringbuilder_flush(builder, stream);

    sprint_stringbuilder_destroy(builder);
    return error;
}

sprint_error sprint_plugin_string(sprint_stringbuilder* builder)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    int initial_count = builder->count;
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_stringbuilder_put_str(builder, "sprint_plugin{state="));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, SPRINT_PLUGIN_STATE_NAMES[sprint_plugin.state]));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, ", language="));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, SPRINT_LANGUAGE_NAMES[sprint_plugin.language]));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, ", operation="));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, SPRINT_OPERATION_NAMES[sprint_plugin.operation]));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, ", pcb="));
    sprint_chain(error, sprint_pcb_string(&sprint_plugin.pcb, builder));
    sprint_chain(error, sprint_stringbuilder_format(builder, ", process=%p", sprint_plugin.process));
    if (!sprint_chain(error, sprint_stringbuilder_put_chr(builder, '}')))
        builder->count = initial_count;

    return error;
}

sprint_pcb* sprint_plugin_get_pcb(void)
{
    return &sprint_plugin.pcb;
}

sprint_plugin_state sprint_plugin_get_state(void)
{
    if (sprint_plugin.state <= SPRINT_PLUGIN_STATE_UNINITIALIZED ||
        sprint_plugin.state > SPRINT_PLUGIN_STATE_COMPLETED)
        return SPRINT_PLUGIN_STATE_UNINITIALIZED;

    return sprint_plugin.state;
}

int sprint_plugin_get_exit_code(void)
{
    return (int) (SPRINT_OPERATION_FAILED_LIBRARY + sprint_plugin_get_state());
}
