//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_PLUGIN_H
#define SPRINTPCB_PLUGIN_H

#include "pcb.h"

#include <stdio.h>

typedef enum sprint_language {
    SPRINT_LANGUAGE_ENGLISH,
    SPRINT_LANGUAGE_GERMAN,
    SPRINT_LANGUAGE_FRENCH
} sprint_language;

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

    // The processing failed due to the command line arguments so perform no operation.
    SPRINT_OPERATION_FAILED_ARGUMENTS = SPRINT_OPERATION_FAILED_START,

    // The processing failed reading the element input, so perform no operation.
    SPRINT_OPERATION_FAILED_INPUT,

    // The processing failed writing the element output, so perform no operation.
    SPRINT_OPERATION_FAILED_OUTPUT,

    // The processing failed internally, so perform no operation.
    SPRINT_OPERATION_FAILED_INTERNAL,

    // The processing failed, so perform no operation. The first plugin specific error code.
    SPRINT_OPERATION_FAILED_PLUGIN = 144,

    // The processing failed, so perform no operation. The last allowed error code.
    SPRINT_OPERATION_FAILED_END = 255
} sprint_operation;

typedef struct sprint_plugin {
    sprint_language language;
    sprint_operation operation;
    void* process;
    FILE* input;
    FILE* output;
} sprint_plugin;

#endif //SPRINTPCB_PLUGIN_H
