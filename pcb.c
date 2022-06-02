//
// libsprintpcb: PCB representation
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "pcb.h"
#include "primitives.h"
#include "output.h"
#include "grid.h"
#include "elements.h"
#include "errors.h"

const char* SPRINT_PCB_FLAG_NAMES[] = {
        "top fill",
        "bottom fill",
        "inner fill 1",
        "inner fill 2",
        "multilayer"
};

sprint_error sprint_pcb_flags_output(sprint_pcb_flags flags, sprint_output* output)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    bool first = true, found = false;
    sprint_error error = SPRINT_ERROR_NONE;
    for (int bit = 0; bit < sizeof(SPRINT_PCB_FLAG_NAMES) / sizeof(const char*); bit++) {
        if (!((flags >> bit) & 1)) continue;
        if (!first)
            sprint_chain(error, sprint_output_put_chr(output, '|'));
        else
            first = false;

        if (!sprint_chain(error, sprint_output_put_str(output, SPRINT_PCB_FLAG_NAMES[bit])))
            break;
        else
            found |= true;
    }

    if (!found)
        sprint_chain(error, sprint_output_put_str(output, flags == 0 ? "none" : "invalid"));

    return sprint_rethrow(error);
}

sprint_error sprint_pcb_output(sprint_pcb* pcb, sprint_output* output)
{
    if (pcb == NULL || output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_output_put_str(output, "sprint_pcb{width="));
    sprint_chain(error, sprint_dist_output(pcb->width, output, SPRINT_PRIM_FORMAT_COOKED));
    sprint_chain(error, sprint_output_put_str(output, ", height="));
    sprint_chain(error, sprint_dist_output(pcb->height, output, SPRINT_PRIM_FORMAT_COOKED));
    sprint_chain(error, sprint_output_put_str(output, ", grid="));
    sprint_chain(error, sprint_grid_output(&pcb->grid, output));
    sprint_chain(error, sprint_output_put_str(output, ", flags="));
    sprint_chain(error, sprint_pcb_flags_output(pcb->flags, output));
    sprint_chain(error, sprint_output_put_str(output, ", elements="));
    sprint_chain(error, sprint_output_put_chr(output, '}'));

    return sprint_rethrow(error);
}
