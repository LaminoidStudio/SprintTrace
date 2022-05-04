//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "plugin.h"

const char* SPRINT_OPERATION_SUCCEEDED_NAMES[] = {
        [SPRINT_OPERATION_NONE] = "no operation",
        [SPRINT_OPERATION_REPLACE_ABSOLUTE] = "replace absolute",
        [SPRINT_OPERATION_ADD_ABSOLUTE] = "add absolute",
        [SPRINT_OPERATION_REPLACE_RELATIVE] = "replace relative",
        [SPRINT_OPERATION_ADD_RELATIVE] = "add relative"
};

const char* SPRINT_OPERATION_FAILED_NAMES[] = {
        [SPRINT_OPERATION_FAILED_ARGUMENTS] = "failed processing arguments",
        [SPRINT_OPERATION_FAILED_INPUT] = "failed processing input",
        [SPRINT_OPERATION_FAILED_OUTPUT] = "failed processing output",
        [SPRINT_OPERATION_FAILED_INTERNAL] = "failed internally",
        [SPRINT_OPERATION_FAILED_PLUGIN] = "failed in plugin"
};
