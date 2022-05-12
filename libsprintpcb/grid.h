//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_GRID_H
#define SPRINTPCB_GRID_H

#include "primitives.h"

typedef struct sprint_grid {
    sprint_tuple origin;
    sprint_dist width;
    sprint_dist height;
} sprint_grid;

sprint_grid sprint_grid_of(sprint_tuple origin, sprint_dist width, sprint_dist height);
bool sprint_grid_valid(sprint_grid* grid);
sprint_error sprint_grid_print(sprint_grid* grid, FILE* stream);
sprint_error sprint_grid_string(sprint_grid* grid, sprint_stringbuilder* builder);

#endif //SPRINTPCB_GRID_H
