//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_PRIMITIVES_H
#define SPRINTPCB_PRIMITIVES_H

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
const sprint_dist SPRINT_DIST_PER_MM    = 10000;
const sprint_dist SPRINT_DIST_MAX       = 500 * SPRINT_DIST_PER_MM;
const sprint_dist SPRINT_DIST_MIN       = -SPRINT_DIST_MAX;

typedef signed int sprint_angle;
const sprint_angle SPRINT_ANGLE_WHOLE   = 1;
const sprint_angle SPRINT_ANGLE_COARSE  = 100;
const sprint_angle SPRINT_ANGLE_FINE    = 1000;
const sprint_angle SPRINT_ANGLE_NATIVE  = SPRINT_ANGLE_FINE;
const sprint_angle SPRINT_ANGLE_MAX     = 360 * SPRINT_ANGLE_NATIVE;
const sprint_angle SPRINT_ANGLE_MIN     = -SPRINT_DIST_MAX;

typedef struct {
    int x;
    int y;
} sprint_tuple;

#endif //SPRINTPCB_PRIMITIVES_H
