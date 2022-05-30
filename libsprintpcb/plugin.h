//
// libsprintpcb: plugin representation and life-cycle management
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_PLUGIN_H
#define SPRINTPCB_PLUGIN_H

#include "pcb.h"
#include "stringbuilder.h"
#include "elements.h"
#include "output.h"
#include "errors.h"

#include <stdio.h>
#include <stdbool.h>

typedef enum sprint_language {
    SPRINT_LANGUAGE_ENGLISH,
    SPRINT_LANGUAGE_GERMAN,
    SPRINT_LANGUAGE_FRENCH
} sprint_language;
extern const char* SPRINT_LANGUAGE_NAMES[];
bool sprint_language_valid(sprint_language language);

typedef enum sprint_operation {
    // Perform no operation. No output should be written.
    SPRINT_OPERATION_NONE,

    // Replace the input elements with the output elements.
    SPRINT_OPERATION_REPLACE_ABSOLUTE,

    // Add the output elements to the circuit board at their specified positions.
    SPRINT_OPERATION_ADD_ABSOLUTE,

    // Remove the input elements and allow the user to place the output elements freely.
    SPRINT_OPERATION_REPLACE_RELATIVE,

    // Allow the user to place the new output elements freely on the circuit board.
    SPRINT_OPERATION_ADD_RELATIVE,

    // The processing failed, so perform no operation. The first error code.
    SPRINT_OPERATION_FAILED_START = 128,

    // The processing failed, so perform no operation. The first library specific error code.
    SPRINT_OPERATION_FAILED_LIBRARY = SPRINT_OPERATION_FAILED_START,

    // The processing failed, so perform no operation. The first plugin specific error code.
    SPRINT_OPERATION_FAILED_PLUGIN = 144,

    // The processing failed, so perform no operation. The last allowed error code.
    SPRINT_OPERATION_FAILED_END = 255
} sprint_operation;
extern const char* SPRINT_OPERATION_NAMES[];
bool sprint_operation_valid(sprint_operation operation, bool failed);

typedef enum sprint_plugin_state {
    SPRINT_PLUGIN_STATE_UNINITIALIZED,
    SPRINT_PLUGIN_STATE_PARSING_FLAGS,
    SPRINT_PLUGIN_STATE_PARSING_INPUT,
    SPRINT_PLUGIN_STATE_PROCESSING,
    SPRINT_PLUGIN_STATE_WRITING_OUTPUT,
    SPRINT_PLUGIN_STATE_COMPLETED
} sprint_plugin_state;
extern const char* SPRINT_PLUGIN_STATE_NAMES[];

#ifdef WIN32
#include <windows.h>
typedef DWORD sprint_process_id;
#else
#include <unistd.h>
#include <sys/types.h>
typedef pid_t sprint_process_id;
#endif

extern const char SPRINT_FLAG_PREFIX;
extern const char SPRINT_FLAG_DELIMITER;
extern const char* SPRINT_OUTPUT_SUFFIX;

sprint_error sprint_plugin_begin(int argc, const char* argv[]);
sprint_error sprint_plugin_output(sprint_output* output);
void sprint_plugin_bail(int error);
sprint_error sprint_plugin_end(sprint_operation operation);
sprint_pcb* sprint_plugin_get_pcb(void);
bool sprint_plugin_is_selection(void);
sprint_plugin_state sprint_plugin_get_state(void);
int sprint_plugin_get_exit_code(void);

#endif //SPRINTPCB_PLUGIN_H
