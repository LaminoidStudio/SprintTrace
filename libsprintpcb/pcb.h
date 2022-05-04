//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_PCB_H
#define SPRINTPCB_PCB_H

#include "primitives.h"
#include "elements.h"

#include <stdbool.h>

typedef enum sprint_pcb_flags {
    // Ground plane enabled on the top copper layer.
    SPRINT_PCB_FLAG_PLANE_TOP = 1 << 0,

    // Ground plane enabled on the bottom copper layer.
    SPRINT_PCB_FLAG_PLANE_BOTTOM = 1 << 1,

    // Ground plane enabled on the first inner copper layer.
    SPRINT_PCB_FLAG_PLANE_INNER1 = 1 << 2,

    // Ground plane enabled on the second inner copper layer.
    SPRINT_PCB_FLAG_PLANE_INNER2 = 1 << 3,

    // The board has four instead of two layers.
    SPRINT_PCB_FLAG_MULTILAYER = 1 << 4
} sprint_pcb_flags;

typedef struct sprint_pcb {
    sprint_dist width;
    sprint_dist height;
    sprint_tuple origin;
    sprint_pcb_flags flags;
    sprint_group elements;
} sprint_pcb;

#endif //SPRINTPCB_PCB_H