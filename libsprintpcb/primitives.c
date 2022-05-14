//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "primitives.h"
#include "errors.h"
#include "stringbuilder.h"
#include "token.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

sprint_error sprint_bool_print(bool val, FILE* stream)
{
    if (stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(7);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_bool_string(val, builder));
    if (!sprint_chain(error, sprint_stringbuilder_flush(builder, stream)))
        sprint_stringbuilder_destroy(builder);

    return sprint_rethrow(error);
}

sprint_error sprint_bool_string(bool val, sprint_stringbuilder* builder)
{
    return sprint_stringbuilder_put_str(builder, val ? SPRINT_TRUE_VALUE : SPRINT_FALSE_VALUE);
}

sprint_error sprint_int_print(int val, FILE* stream)
{
    if (stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(7);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_int_string(val, builder));
    if (!sprint_chain(error, sprint_stringbuilder_flush(builder, stream)))
        sprint_stringbuilder_destroy(builder);

    return sprint_rethrow(error);
}

sprint_error sprint_int_string(int val, sprint_stringbuilder* builder)
{
    return sprint_stringbuilder_put_int(builder, val);
}

sprint_error sprint_str_print(const char* str, FILE* stream, sprint_prim_format format)
{
    if (str == NULL || stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(15);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_str_string(str, builder, format));
    if (!sprint_chain(error, sprint_stringbuilder_flush(builder, stream)))
        sprint_stringbuilder_destroy(builder);

    return sprint_rethrow(error);
}

sprint_error sprint_str_string(const char* str, sprint_stringbuilder* builder, sprint_prim_format format)
{
    if (str == NULL || builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Write the string based on the format
    switch (format) {
        case SPRINT_PRIM_FORMAT_RAW:
            return sprint_rethrow(sprint_stringbuilder_format(builder, "%c%s%c",
                                               SPRINT_STRING_DELIMITER, str, SPRINT_STRING_DELIMITER));

        case SPRINT_PRIM_FORMAT_COOKED:
            return sprint_rethrow(sprint_stringbuilder_format(builder, "\"%s\"", str));

        default:
            return SPRINT_ERROR_ARGUMENT_RANGE;
    }
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

sprint_error sprint_layer_print(sprint_layer layer, FILE* stream, sprint_prim_format format)
{
    if (stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(7);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_layer_string(layer, builder, format));
    if (!sprint_chain(error, sprint_stringbuilder_flush(builder, stream)))
        sprint_stringbuilder_destroy(builder);

    return sprint_rethrow(error);
}

sprint_error sprint_layer_string(sprint_layer layer, sprint_stringbuilder* builder, sprint_prim_format format)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_layer_valid(layer)) return SPRINT_ERROR_ARGUMENT_RANGE;

    // Write the string based on the format
    const char* layer_name;
    switch (format) {
        case SPRINT_PRIM_FORMAT_RAW:
            return sprint_rethrow(sprint_stringbuilder_put_int(builder, layer));

        case SPRINT_PRIM_FORMAT_COOKED:
            layer_name = SPRINT_LAYER_NAMES[layer];
            if (sprint_assert(false, layer_name != NULL))
                return SPRINT_ERROR_ASSERTION;
            return sprint_rethrow(sprint_stringbuilder_put_str(builder, layer_name));

        default:
            return SPRINT_ERROR_ARGUMENT_RANGE;
    }
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
    return dist >= SPRINT_DIST_MIN && dist <= SPRINT_DIST_MIN;
}

sprint_error sprint_dist_print(sprint_dist dist, FILE* stream, sprint_prim_format format)
{
    if (stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(7);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_dist_string(dist, builder, format));
    if (!sprint_chain(error, sprint_stringbuilder_flush(builder, stream)))
        sprint_stringbuilder_destroy(builder);

    return sprint_rethrow(error);
}

sprint_error sprint_dist_string(sprint_dist dist, sprint_stringbuilder* builder, sprint_prim_format format)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    int dist_per_unit;
    int dist_precision;
    const char* dist_suffix;
    switch (format) {
        case SPRINT_PRIM_FORMAT_RAW:
            return sprint_rethrow(sprint_stringbuilder_put_int(builder, dist));

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
            return SPRINT_ERROR_ARGUMENT_RANGE;
    }

    // Keep track of encountered errors and then restore the initial builder count
    sprint_error error = SPRINT_ERROR_NONE;
    int initial_count = builder->count;

    // Append the integer part, decimal point and mantissa
    sprint_chain(error, sprint_stringbuilder_format(builder, "%d.%0*d",
                                                dist / dist_per_unit, dist_precision, abs(dist % dist_per_unit)));

    // Append the unit suffix or reset the string builder on error
    if (!sprint_chain(error, sprint_stringbuilder_put_str(builder, dist_suffix)))
        builder->count = initial_count;

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

sprint_error sprint_angle_print(sprint_angle angle, FILE* stream, sprint_prim_format format)
{
    if (stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(7);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_angle_string(angle, builder, format));
    if (!sprint_chain(error, sprint_stringbuilder_flush(builder, stream)))
        sprint_stringbuilder_destroy(builder);

    return sprint_rethrow(error);
}

sprint_error sprint_angle_string(sprint_angle angle, sprint_stringbuilder* builder, sprint_prim_format format)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Keep track of encountered errors and then restore the initial builder count
    sprint_error error = SPRINT_ERROR_NONE;
    int initial_count = builder->count;

    // Write the string based on the format
    switch (format) {
        case SPRINT_PRIM_FORMAT_RAW:
            return sprint_rethrow(sprint_stringbuilder_put_int(builder, angle));

        case SPRINT_PRIM_FORMAT_COOKED:
            // Append the integer part, decimal point and mantissa part
            sprint_chain(error, sprint_stringbuilder_format(builder, "%d.%0*d", angle / SPRINT_ANGLE_NATIVE,
                                                        SPRINT_ANGLE_PRECISION, abs(angle % SPRINT_ANGLE_NATIVE)));

            // Append the unit suffix or reset the string builder on error
            if (!sprint_chain(error, sprint_stringbuilder_put_str(builder, "deg")))
                builder->count = initial_count;

            return sprint_rethrow(error);

        default:
            return SPRINT_ERROR_ARGUMENT_RANGE;
    }
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

sprint_error sprint_tuple_print(sprint_tuple tuple, FILE* stream, sprint_prim_format format)
{
    if (stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(23);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_tuple_string(tuple, builder, format));
    if (!sprint_chain(error, sprint_stringbuilder_flush(builder, stream)))
        sprint_stringbuilder_destroy(builder);

    return sprint_rethrow(error);
}

sprint_error sprint_tuple_string(sprint_tuple tuple, sprint_stringbuilder* builder, sprint_prim_format format)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Store the initial builder count to be restored on error and try to append the first distance
    int initial_count = builder->count;
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_dist_string(tuple.x, builder, format));

    // Try to append the separator
    sprint_chain(error, sprint_stringbuilder_put_chr(builder, SPRINT_TUPLE_SEPARATOR));

    // Finally, try to append the second tuple or reset the string builder on error
    if (!sprint_chain(error, sprint_dist_string(tuple.y, builder, format)))
        builder->count = initial_count;

    return sprint_rethrow(error);
}
