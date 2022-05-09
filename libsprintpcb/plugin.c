//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "plugin.h"
#include "stringbuilder.h"

#include <stdbool.h>

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
    FILE* output;
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
        [SPRINT_PLUGIN_STATE_EXITING] = "exiting"
};

sprint_error sprint_plugin_parse_internal(int argc, char* argv[])
{
    if (argv == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (argc < 9 || argc > 10) return SPRINT_ERROR_PLUGIN_FLAGS_MISSING;

    // Skip the first argument
    argc--;
    argv++;

    // Keep track of the parameters passed (defaults: language=UK, x=0, y=0, all=false, pid=0; rest is required)
    bool found_language = false, found_width = false, found_height = false, found_x = false, found_y = false,
        found_flags = false, found_all = false, found_pid = false, found_input = false;


    // Make sure the required parameters are present and emit an error, if not
    sprint_stringbuilder* builder = sprint_stringbuilder_create(32);
    sprint_error error = SPRINT_ERROR_NONE;
    if (!found_input)
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "input file"));
    if (!found_width) {
        if (builder->count > 0)
            sprint_chain(error, sprint_stringbuilder_put_chr(builder, '|'));
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "width (/W)"));
    }
    if (!found_height) {
        if (builder->count > 0)
            sprint_chain(error, sprint_stringbuilder_put_chr(builder, '|'));
        sprint_chain(error, sprint_stringbuilder_put_str(builder, "height (/H)"));
    }
    if (builder->count > 0) {
        char* flags_str = sprint_stringbuilder_complete(builder);
        sprint_assert(true, flags_str != NULL);
        sprint_throw(false, "could not find required arguments");
        fprintf(stderr, "Error: could not find required arguments: %s\n", flags_str);
        free(flags_str);
        return found_input ? SPRINT_ERROR_PLUGIN_INPUT_MISSING : SPRINT_ERROR_PLUGIN_FLAGS_MISSING;
    }

    // Emit a warning for all missing optional flags except for /A
    //if (!found_language)
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
        sprint_plugin.state > SPRINT_PLUGIN_STATE_EXITING)
        return SPRINT_PLUGIN_STATE_UNINITIALIZED;

    return sprint_plugin.state;
}

int sprint_plugin_get_exit_code(void)
{
    return (int) (SPRINT_OPERATION_FAILED_LIBRARY + sprint_plugin_get_state());
}
