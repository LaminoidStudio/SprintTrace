//
// Created by Benedikt on 04.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "pcb.h"

const char* SPRINT_PCB_FLAG_NAMES[] = {
        "top fill",
        "bottom fill",
        "inner fill 1",
        "inner fill 2",
        "multilayer"
};

sprint_error sprint_pcb_flags_print(sprint_pcb_flags flags, FILE* stream)
{
    if (stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(23);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = sprint_pcb_flags_string(flags, builder);
    if (error == SPRINT_ERROR_NONE)
        return sprint_stringbuilder_flush(builder, stream);

    sprint_stringbuilder_destroy(builder);
    return error;
}

sprint_error sprint_pcb_flags_string(sprint_pcb_flags flags, sprint_stringbuilder* builder)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    bool first = true, found = false;
    int initial_count = builder->count;
    sprint_error error = SPRINT_ERROR_NONE;
    for (int bit = 0; bit < sizeof(SPRINT_PCB_FLAG_NAMES) / sizeof(const char*); bit++) {
        if (!((flags >> bit) & 1)) continue;
        if (!first)
            sprint_chain(error, sprint_stringbuilder_put_chr(builder, '|'));
        else
            first = false;

        if (!sprint_chain(error, sprint_stringbuilder_put_str(builder, SPRINT_PCB_FLAG_NAMES[bit]))) {
            builder->count = initial_count;
            break;
        } else
            found |= true;
    }

    if (!found)
        sprint_chain(error, sprint_stringbuilder_put_str(builder, flags == 0 ? "none" : "invalid"));

    return error;
}

sprint_error sprint_pcb_print(sprint_pcb* pcb, FILE* stream)
{
    if (pcb == NULL || stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(31);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = sprint_pcb_string(pcb, builder);
    if (error == SPRINT_ERROR_NONE)
        return sprint_stringbuilder_flush(builder, stream);

    sprint_stringbuilder_destroy(builder);
    return error;
}

sprint_error sprint_pcb_string(sprint_pcb* pcb, sprint_stringbuilder* builder)
{
    if (pcb == NULL || builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    int initial_count = builder->count;
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_stringbuilder_put_str(builder, "sprint_pcb{width="));
    sprint_chain(error, sprint_dist_string(pcb->width, builder, SPRINT_PRIM_FORMAT_COOKED));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, ", height="));
    sprint_chain(error, sprint_dist_string(pcb->height, builder, SPRINT_PRIM_FORMAT_COOKED));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, ", origin="));
    sprint_chain(error, sprint_tuple_string(pcb->origin, builder, SPRINT_PRIM_FORMAT_COOKED));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, ", flags="));
    sprint_chain(error, sprint_pcb_flags_string(pcb->flags, builder));
    sprint_chain(error, sprint_stringbuilder_put_str(builder, ", grid="));

    sprint_chain(error, sprint_stringbuilder_put_str(builder, ", elements="));

    if (!sprint_chain(error, sprint_stringbuilder_put_chr(builder, '}')))
        builder->count = initial_count;

    return error;
}
