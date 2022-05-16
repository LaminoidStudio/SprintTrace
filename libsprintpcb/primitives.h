//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_PRIMITIVES_H
#define SPRINTPCB_PRIMITIVES_H

#include "errors.h"
#include "stringbuilder.h"

#include <math.h>
#include <stdio.h>
#include <stdbool.h>

sprint_error sprint_bool_print(bool val, FILE* stream);
sprint_error sprint_bool_string(bool val, sprint_stringbuilder* builder);

sprint_error sprint_int_print(int val, FILE* stream);
sprint_error sprint_int_string(int val, sprint_stringbuilder* builder);

typedef enum sprint_prim_format {
    SPRINT_PRIM_FORMAT_RAW,
    SPRINT_PRIM_FORMAT_COOKED,
    SPRINT_PRIM_FORMAT_DIST_UM,
    SPRINT_PRIM_FORMAT_DIST_MM,
    SPRINT_PRIM_FORMAT_DIST_CM,
    SPRINT_PRIM_FORMAT_DIST_TH,
    SPRINT_PRIM_FORMAT_DIST_IN
} sprint_prim_format;
bool sprint_prim_format_valid(sprint_prim_format format);

sprint_error sprint_str_print(const char* str, FILE* stream, sprint_prim_format format);
sprint_error sprint_str_string(const char* str, sprint_stringbuilder* builder, sprint_prim_format format);

typedef enum sprint_layer {
    // The top copper layer (C1)
    SPRINT_LAYER_COPPER_TOP = 1,

    // The top silkscreen layer (S1)
    SPRINT_LAYER_SILKSCREEN_TOP,

    // The bottom copper layer (C2)
    SPRINT_LAYER_COPPER_BOTTOM,

    // The bottom silkscreen layer (S2)
    SPRINT_LAYER_SILKSCREEN_BOTTOM,

    // The first (usually top) inner copper layer (I1)
    SPRINT_LAYER_COPPER_INNER1,

    // The second (usually bottom) inner copper layer (I2)
    SPRINT_LAYER_COPPER_INNER2,

    // The mechanical outline layer (O)
    SPRINT_LAYER_MECHANICAL
} sprint_layer;
extern const char* SPRINT_LAYER_NAMES[];
bool sprint_layer_valid(sprint_layer layer);
sprint_error sprint_layer_print(sprint_layer layer, FILE* stream, sprint_prim_format format);
sprint_error sprint_layer_string(sprint_layer layer, sprint_stringbuilder* builder, sprint_prim_format format);

typedef signed int sprint_dist;
extern const sprint_dist SPRINT_DIST_PER_UM;
extern const sprint_dist SPRINT_DIST_PER_MM;
extern const sprint_dist SPRINT_DIST_PER_CM;
extern const sprint_dist SPRINT_DIST_PER_TH;
extern const sprint_dist SPRINT_DIST_PER_IN;
extern const sprint_dist SPRINT_DIST_MAX;
extern const sprint_dist SPRINT_DIST_MIN;
#define sprint_dist_um(d) ((sprint_dist)((d) * SPRINT_DIST_PER_UM))
#define sprint_dist_mm(d) ((sprint_dist)((d) * SPRINT_DIST_PER_MM))
#define sprint_dist_cm(d) ((sprint_dist)((d) * SPRINT_DIST_PER_CM))
#define sprint_dist_th(d) ((sprint_dist)((d) * SPRINT_DIST_PER_TH))
#define sprint_dist_in(d) ((sprint_dist)((d) * SPRINT_DIST_PER_IN))
bool sprint_dist_valid(sprint_dist dist);
bool sprint_size_valid(sprint_dist size);
sprint_error sprint_dist_print(sprint_dist dist, FILE* stream, sprint_prim_format format);
sprint_error sprint_dist_string(sprint_dist dist, sprint_stringbuilder* builder, sprint_prim_format format);

typedef signed int sprint_angle;
extern const sprint_angle SPRINT_ANGLE_WHOLE;
extern const sprint_angle SPRINT_ANGLE_COARSE;
extern const sprint_angle SPRINT_ANGLE_FINE;
extern const sprint_angle SPRINT_ANGLE_NATIVE;
extern const sprint_angle SPRINT_ANGLE_MAX;
extern const sprint_angle SPRINT_ANGLE_MIN;
#define sprint_angle_deg(a) ((sprint_angle)((a) * SPRINT_ANGLE_NATIVE))
#define sprint_angle_rad(r) sprint_angle_deg((r) * M_PI / 180d)
bool sprint_angle_valid(sprint_angle angle);
sprint_error sprint_angle_print(sprint_angle angle, FILE* stream, sprint_prim_format format);
sprint_error sprint_angle_string(sprint_angle angle, sprint_stringbuilder* builder, sprint_prim_format format);

typedef struct sprint_tuple {
    sprint_dist x;
    sprint_dist y;
} sprint_tuple;
sprint_tuple sprint_tuple_of(sprint_dist x, sprint_dist y);
bool sprint_tuple_valid(sprint_tuple tuple);
sprint_error sprint_tuple_print(sprint_tuple tuple, FILE* stream, sprint_prim_format format);
sprint_error sprint_tuple_string(sprint_tuple tuple, sprint_stringbuilder* builder, sprint_prim_format format);

#endif //SPRINTPCB_PRIMITIVES_H
