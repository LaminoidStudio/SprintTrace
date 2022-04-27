//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "primitives.h"
#include "errors.h"
#include "stringbuilder.h"

#include <stdio.h>
#include <string.h>

const sprint_dist SPRINT_DIST_PER_UM    = 10;
const sprint_dist SPRINT_DIST_PER_MM    = SPRINT_DIST_PER_UM * 1000;
const sprint_dist SPRINT_DIST_PER_CM    = SPRINT_DIST_PER_MM * 10;
const sprint_dist SPRINT_DIST_PER_TH    = 254;
const sprint_dist SPRINT_DIST_PER_IN    = SPRINT_DIST_PER_TH * 1000;
const sprint_dist SPRINT_DIST_MAX       = 50 * SPRINT_DIST_PER_CM;
const sprint_dist SPRINT_DIST_MIN       = -SPRINT_DIST_MAX;

const sprint_angle SPRINT_ANGLE_WHOLE   = 1;
const sprint_angle SPRINT_ANGLE_COARSE  = 100;
const sprint_angle SPRINT_ANGLE_FINE    = 1000;
const sprint_angle SPRINT_ANGLE_NATIVE  = SPRINT_ANGLE_FINE;
const sprint_angle SPRINT_ANGLE_MAX     = 360 * SPRINT_ANGLE_NATIVE;
const sprint_angle SPRINT_ANGLE_MIN     = -SPRINT_DIST_MAX;


sprint_tuple sprint_tuple_of(int x, int y)
{
    sprint_tuple tuple;
    memset(&tuple, 0, sizeof(sprint_tuple));
    tuple.x = x;
    tuple.y = y;
    return tuple;
}

sprint_error sprint_tuple_print(sprint_tuple* tuple, FILE* stream)
{
    return sprint_stringbuilder_flush(sprint_stringbuilder_create(16), stream);
}

sprint_error sprint_tuple_string(sprint_tuple* tuple, sprint_stringbuilder* builder)
{
    if (tuple == NULL || builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    return sprint_stringbuilder_format(builder, "%d/%d", tuple->x, tuple->y);
}
