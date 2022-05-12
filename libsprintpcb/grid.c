//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "grid.h"

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

sprint_error sprint_grid_print(sprint_grid* grid, FILE* stream)
{
    if (grid == NULL || stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(15);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = sprint_grid_string(grid, builder);
    if (error == SPRINT_ERROR_NONE)
        return sprint_stringbuilder_flush(builder, stream);

    sprint_stringbuilder_destroy(builder);
    return error;
}

sprint_error sprint_grid_string(sprint_grid* grid, sprint_stringbuilder* builder)
{
    if (grid == NULL || builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Store the initial builder count to be restored on error
    int initial_count = builder->count;
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_stringbuilder_put_str(builder, "sprint_grid{origin="));
    sprint_chain(error, sprint_tuple_string(grid->origin, builder, SPRINT_PRIM_FORMAT_COOKED));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, ", width="));
    sprint_chain(error, sprint_dist_string(grid->width, builder, SPRINT_PRIM_FORMAT_COOKED));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, ", height="));
    sprint_chain(error, sprint_dist_string(grid->height, builder, SPRINT_PRIM_FORMAT_COOKED));
    if (!sprint_chain(error, sprint_stringbuilder_put_chr(builder, '}')))
        builder->count = initial_count;

    return error;
}
