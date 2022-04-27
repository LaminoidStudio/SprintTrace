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

typedef enum {
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

typedef signed int sprint_angle;
extern const sprint_angle SPRINT_ANGLE_WHOLE;
extern const sprint_angle SPRINT_ANGLE_COARSE;
extern const sprint_angle SPRINT_ANGLE_FINE;
extern const sprint_angle SPRINT_ANGLE_NATIVE;
extern const sprint_angle SPRINT_ANGLE_MAX;
extern const sprint_angle SPRINT_ANGLE_MIN;
#define sprint_angle_deg(a) ((sprint_angle)((a) * SPRINT_ANGLE_NATIVE))
#define sprint_angle_rad(r) sprint_angle_deg((r) * M_PI / 180d)

typedef struct {
    int x;
    int y;
} sprint_tuple;
sprint_tuple sprint_tuple_of(int x, int y);
sprint_error sprint_tuple_print(sprint_tuple* tuple, FILE* stream);
sprint_error sprint_tuple_string(sprint_tuple* tuple, sprint_stringbuilder* builder);

#endif //SPRINTPCB_PRIMITIVES_H
