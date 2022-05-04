//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_GRID_H
#define SPRINTPCB_GRID_H

#include "primitives.h"

typedef struct sprint_grid {
    sprint_dist width;
    sprint_dist height;
    sprint_tuple origin;
} sprint_grid;

#endif //SPRINTPCB_GRID_H
