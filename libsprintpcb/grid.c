//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "grid.h"
#include "primitives.h"
#include "output.h"
#include "errors.h"

sprint_grid sprint_grid_of(sprint_tuple origin, sprint_dist width, sprint_dist height)
{
    sprint_grid grid;
    memset(&grid, 0, sizeof(sprint_grid));
    grid.origin = origin;
    grid.width = width;
    grid.height = height;
    return grid;
}

bool sprint_grid_valid(sprint_grid* grid)
{
    return sprint_tuple_valid(grid->origin) && sprint_dist_valid(grid->width) &&
        sprint_dist_valid(grid->height);
}

sprint_error sprint_grid_output(sprint_grid* grid, sprint_output* output)
{
    if (grid == NULL || output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_output_put_str(output, "sprint_grid{origin="));
    sprint_chain(error, sprint_tuple_output(grid->origin, output, SPRINT_PRIM_FORMAT_COOKED));
    sprint_chain(error, sprint_output_put_str(output, ", width="));
    sprint_chain(error, sprint_dist_output(grid->width, output, SPRINT_PRIM_FORMAT_COOKED));
    sprint_chain(error, sprint_output_put_str(output, ", height="));
    sprint_chain(error, sprint_dist_output(grid->height, output, SPRINT_PRIM_FORMAT_COOKED));
    sprint_chain(error, sprint_output_put_chr(output, '}'));

    return sprint_rethrow(error);
}
