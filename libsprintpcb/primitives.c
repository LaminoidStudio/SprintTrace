//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "primitives.h"
#include "errors.h"
#include "token.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

sprint_error sprint_bool_output(bool val, sprint_output* output)
{
    return sprint_output_put_str(output, val ? SPRINT_TRUE_VALUE : SPRINT_FALSE_VALUE);
}

sprint_error sprint_int_output(int val, sprint_output* output)
{
    return sprint_output_put_int(output, val);
}

bool sprint_prim_format_valid(sprint_prim_format format)
{
    return format >= SPRINT_PRIM_FORMAT_RAW && format <= SPRINT_PRIM_FORMAT_ANGLE_WHOLE;
}

bool sprint_prim_format_cooked(sprint_prim_format format)
{
    switch (format) {
        case SPRINT_PRIM_FORMAT_RAW:
        case SPRINT_PRIM_FORMAT_ANGLE_FINE:
        case SPRINT_PRIM_FORMAT_ANGLE_COARSE:
        case SPRINT_PRIM_FORMAT_ANGLE_WHOLE:
            return false;

        case SPRINT_PRIM_FORMAT_COOKED:
        case SPRINT_PRIM_FORMAT_DIST_UM:
        case SPRINT_PRIM_FORMAT_DIST_MM:
        case SPRINT_PRIM_FORMAT_DIST_CM:
        case SPRINT_PRIM_FORMAT_DIST_TH:
        case SPRINT_PRIM_FORMAT_DIST_IN:
            return true;

        default:
            sprint_throw_format(false, "unknown primitive format: %d", format);
            return false;
    }
}

sprint_prim_format sprint_prim_format_of(sprint_prim_format format, bool cooked)
{
    switch (format) {
        case SPRINT_PRIM_FORMAT_RAW:
        case SPRINT_PRIM_FORMAT_COOKED:
            break;

        case SPRINT_PRIM_FORMAT_DIST_UM:
        case SPRINT_PRIM_FORMAT_DIST_MM:
        case SPRINT_PRIM_FORMAT_DIST_CM:
        case SPRINT_PRIM_FORMAT_DIST_TH:
        case SPRINT_PRIM_FORMAT_DIST_IN:
            return cooked ? format : SPRINT_PRIM_FORMAT_RAW;

        case SPRINT_PRIM_FORMAT_ANGLE_FINE:
        case SPRINT_PRIM_FORMAT_ANGLE_COARSE:
        case SPRINT_PRIM_FORMAT_ANGLE_WHOLE:
            return cooked ? SPRINT_PRIM_FORMAT_COOKED : format;

        default:
            sprint_throw_format(false, "unknown primitive format: %d", format);
    }

    return cooked ? SPRINT_PRIM_FORMAT_COOKED : SPRINT_PRIM_FORMAT_RAW;
}

sprint_error sprint_str_output(const char* str, sprint_output* output, sprint_prim_format format)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_prim_format_valid(format)) return SPRINT_ERROR_ARGUMENT_RANGE;
    if (str != NULL && strchr(str, SPRINT_STRING_DELIMITER) != NULL) return SPRINT_ERROR_ARGUMENT_FORMAT;

    // Default the string to empty, if it is null
    if (str == NULL)
        str = "";

    // Write the string based on the format
    if (sprint_prim_format_cooked(format))
        return sprint_rethrow(sprint_output_format(output, "\"%s\"", str));
    else
        return sprint_rethrow(sprint_output_format(output, "%c%s%c",
                                                   SPRINT_STRING_DELIMITER, str, SPRINT_STRING_DELIMITER));
}


const char* SPRINT_LAYER_NAMES[] = {
        [SPRINT_LAYER_COPPER_TOP] = "top copper",
        [SPRINT_LAYER_SILKSCREEN_TOP] = "top silkscreen",
        [SPRINT_LAYER_COPPER_BOTTOM] = "bottom copper",
        [SPRINT_LAYER_SILKSCREEN_BOTTOM] = "bottom silkscreen",
        [SPRINT_LAYER_COPPER_INNER1] = "inner copper 1",
        [SPRINT_LAYER_COPPER_INNER2] = "inner copper 2",
        [SPRINT_LAYER_MECHANICAL] = "mechanical"
};

bool sprint_layer_valid(sprint_layer layer)
{
    return layer >= SPRINT_LAYER_COPPER_TOP && layer <= SPRINT_LAYER_MECHANICAL;
}

sprint_error sprint_layer_output(sprint_layer layer, sprint_output* output, sprint_prim_format format)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_layer_valid(layer) || !sprint_prim_format_valid(format)) return SPRINT_ERROR_ARGUMENT_RANGE;

    // Write the string based on the format
    const char* layer_name;
    if (sprint_prim_format_cooked(format)) {
        layer_name = SPRINT_LAYER_NAMES[layer];
        if (!sprint_assert(false, layer_name != NULL))
            return SPRINT_ERROR_ASSERTION;
        return sprint_rethrow(sprint_output_put_str(output, layer_name));
    } else
        return sprint_rethrow(sprint_output_put_int(output, layer));
}

const sprint_dist SPRINT_DIST_PER_UM    = 10;
const sprint_dist SPRINT_DIST_PER_MM    = SPRINT_DIST_PER_UM * 1000;
const sprint_dist SPRINT_DIST_PER_CM    = SPRINT_DIST_PER_MM * 10;
const sprint_dist SPRINT_DIST_PER_TH    = 254;
const sprint_dist SPRINT_DIST_PER_IN    = SPRINT_DIST_PER_TH * 1000;
const int SPRINT_DIST_PRECISION_UM      = 1;
const int SPRINT_DIST_PRECISION_MM      = SPRINT_DIST_PRECISION_UM + 3;
const int SPRINT_DIST_PRECISION_CM      = SPRINT_DIST_PRECISION_MM + 2;
const int SPRINT_DIST_PRECISION_TH      = 3;
const int SPRINT_DIST_PRECISION_IN      = SPRINT_DIST_PRECISION_TH + 2;
const sprint_dist SPRINT_DIST_MAX       = 50 * SPRINT_DIST_PER_CM;
const sprint_dist SPRINT_DIST_MIN       = -SPRINT_DIST_MAX;

bool sprint_dist_valid(sprint_dist dist)
{
    return dist >= SPRINT_DIST_MIN && dist <= SPRINT_DIST_MAX;
}

bool sprint_size_valid(sprint_dist size)
{
    return size >= 0 && size <= SPRINT_DIST_MAX;
}

sprint_error sprint_dist_output(sprint_dist dist, sprint_output* output, sprint_prim_format format)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_prim_format_valid(format)) return SPRINT_ERROR_ARGUMENT_RANGE;

    int dist_per_unit;
    int dist_precision;
    const char* dist_suffix;
    switch (format) {
        case SPRINT_PRIM_FORMAT_COOKED:
        case SPRINT_PRIM_FORMAT_DIST_MM:
            dist_per_unit = SPRINT_DIST_PER_MM;
            dist_precision = SPRINT_DIST_PRECISION_MM;
            dist_suffix = "mm";
            break;

        case SPRINT_PRIM_FORMAT_DIST_UM:
            dist_per_unit = SPRINT_DIST_PER_UM;
            dist_precision = SPRINT_DIST_PRECISION_UM;
            dist_suffix = "um";
            break;

        case SPRINT_PRIM_FORMAT_DIST_CM:
            dist_per_unit = SPRINT_DIST_PER_CM;
            dist_precision = SPRINT_DIST_PRECISION_CM;
            dist_suffix = "cm";
            break;

        case SPRINT_PRIM_FORMAT_DIST_TH:
            dist_per_unit = SPRINT_DIST_PER_TH;
            dist_precision = SPRINT_DIST_PRECISION_TH;
            dist_suffix = "th";
            break;

        case SPRINT_PRIM_FORMAT_DIST_IN:
            dist_per_unit = SPRINT_DIST_PER_IN;
            dist_precision = SPRINT_DIST_PRECISION_IN;
            dist_suffix = "in";
            break;

        default:
            if (!sprint_assert(false, !sprint_prim_format_cooked(format)))
                return SPRINT_ERROR_ASSERTION;

            return sprint_rethrow(sprint_output_put_int(output, dist));
    }

    // Append the integer part, decimal point and mantissa
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_output_format(output, "%d.%0*d", dist / dist_per_unit, dist_precision,
                                             abs(dist % dist_per_unit)));

    // Append the unit suffix
    sprint_chain(error, sprint_output_put_str(output, dist_suffix));
    return sprint_rethrow(error);
}

const sprint_angle SPRINT_ANGLE_WHOLE   = 1;
const sprint_angle SPRINT_ANGLE_COARSE  = 100;
const sprint_angle SPRINT_ANGLE_FINE    = 1000;
const sprint_angle SPRINT_ANGLE_NATIVE  = SPRINT_ANGLE_FINE;
const int SPRINT_ANGLE_PRECISION        = 3;
const sprint_angle SPRINT_ANGLE_MAX     = 360 * SPRINT_ANGLE_NATIVE;
const sprint_angle SPRINT_ANGLE_MIN     = -SPRINT_DIST_MAX;

bool sprint_angle_valid(sprint_angle angle)
{
    return angle >= SPRINT_ANGLE_MIN && angle <= SPRINT_ANGLE_MAX;
}

sprint_angle sprint_angle_factor(sprint_prim_format format)
{
    switch (format) {
        case SPRINT_PRIM_FORMAT_RAW:
            return 1;
        case SPRINT_PRIM_FORMAT_ANGLE_FINE:
            return SPRINT_ANGLE_NATIVE / SPRINT_ANGLE_FINE;
        case SPRINT_PRIM_FORMAT_ANGLE_COARSE:
            return SPRINT_ANGLE_NATIVE / SPRINT_ANGLE_COARSE;
        case SPRINT_PRIM_FORMAT_ANGLE_WHOLE:
            return SPRINT_ANGLE_NATIVE / SPRINT_ANGLE_WHOLE;
        default:
            return 0;
    }
}

sprint_error sprint_angle_output(sprint_angle angle, sprint_output* output, sprint_prim_format format)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_prim_format_valid(format)) return SPRINT_ERROR_ARGUMENT_RANGE;

    // Write the string based on the format
    sprint_error error = SPRINT_ERROR_NONE;
    if (sprint_prim_format_cooked(format)) {
        // Append the integer part, decimal point and mantissa part
        sprint_chain(error, sprint_output_format(output, "%d.%0*d", angle / SPRINT_ANGLE_NATIVE,
                                                        SPRINT_ANGLE_PRECISION, abs(angle % SPRINT_ANGLE_NATIVE)));

        // Append the unit suffix
        sprint_chain(error, sprint_output_put_str(output, "deg"));
        return sprint_rethrow(error);
    }

    // Handle the different raw precisions
    sprint_angle factor = sprint_angle_factor(format);
    if (factor < 1)
        return SPRINT_ERROR_ARGUMENT_RANGE;

    // And output the raw scaled value
    return sprint_rethrow(sprint_output_put_int(output, angle / factor));
}

sprint_tuple sprint_tuple_of(sprint_dist x, sprint_dist y)
{
    sprint_tuple tuple;
    memset(&tuple, 0, sizeof(sprint_tuple));
    tuple.x = x;
    tuple.y = y;
    return tuple;
}

bool sprint_tuple_valid(sprint_tuple tuple)
{
    return sprint_dist_valid(tuple.x) && sprint_dist_valid(tuple.y);
}

sprint_error sprint_tuple_output(sprint_tuple tuple, sprint_output* output, sprint_prim_format format)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Try to append the first distance
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_dist_output(tuple.x, output, format));

    // Try to append the separator
    sprint_chain(error, sprint_output_put_chr(output, SPRINT_TUPLE_SEPARATOR));

    // Finally, try to append the second tuple
    sprint_chain(error, sprint_dist_output(tuple.y, output, format));

    return sprint_rethrow(error);
}
