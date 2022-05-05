//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "plugin.h"
#include "stringbuilder.h"

const char* SPRINT_LANGUAGE_NAMES[] = {
        [SPRINT_LANGUAGE_ENGLISH] = "English",
        [SPRINT_LANGUAGE_GERMAN] = "German",
        [SPRINT_LANGUAGE_FRENCH] = "French"
};

static struct sprint_plugin {
    sprint_plugin_state state;
    sprint_language language;
    sprint_operation operation;
    sprint_pcb pcb;
    void* process;
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

sprint_error sprint_plugin_print(FILE* stream)
{
    if (stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(7);
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

    sprint_stringbuilder_put_str(builder, "sprint_plugin{state=");
    sprint_stringbuilder_put_str(builder, SPRINT_PLUGIN_STATE_NAMES[sprint_plugin.state]);
    sprint_stringbuilder_put_str(builder, ", language=");
    sprint_stringbuilder_put_str(builder, SPRINT_LANGUAGE_NAMES[sprint_plugin.language]);
    sprint_stringbuilder_put_str(builder, ", operation=");
    sprint_stringbuilder_put_str(builder, SPRINT_OPERATION_NAMES[sprint_plugin.operation]);
    sprint_stringbuilder_put_str(builder, ", pcb=");
    sprint_stringbuilder_format(builder, ", process=%p", sprint_plugin.process);
    sprint_stringbuilder_put_chr(builder, '}');

    sprint_plugin_state state;
    sprint_language language;
    sprint_operation operation;
    sprint_pcb pcb;
    void* process;
    FILE* input;
    FILE* output;
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
