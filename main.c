//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include <stdio.h>
#include <string.h>

#include "libsprintpcb/list.h"
#include "libsprintpcb/primitives.h"
#include "libsprintpcb/elements.h"
#include "libsprintpcb/stringbuilder.h"
#include "libsprintpcb/plugin.h"
#include "libsprintpcb/errors.h"

int main() {
    sprint_element circle = sprint_circle_create(
            SPRINT_LAYER_MECHANICAL,
            10,
            sprint_tuple_of(sprint_dist_um(1), sprint_dist_um(20)),
            10);

    sprint_prim_format format_dist = SPRINT_PRIM_FORMAT_DIST_UM;
    sprint_prim_format format_angle = format_dist == SPRINT_PRIM_FORMAT_RAW ? format_dist : SPRINT_PRIM_FORMAT_COOKED;
    sprint_prim_format format_layer = format_angle;

    sprint_plugin_print(stdout);
    fputc('\n', stdout);

    sprint_stringbuilder* builder = sprint_stringbuilder_of("Circle and builder test:\n");
    sprint_stringbuilder_put_str(builder, "layer: ");
    sprint_layer_string(circle.circle.layer, builder, format_layer);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "width: ");
    sprint_dist_string(circle.circle.width, builder, format_dist);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "center: ");
    sprint_tuple_string(circle.circle.center, builder, format_dist);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "radius: ");
    sprint_dist_string(circle.circle.radius, builder, format_dist);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "clear: ");
    sprint_dist_string(circle.circle.clear, builder, format_dist);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "(cutout): ");
    sprint_bool_string(circle.circle.cutout, builder);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "(soldermask): ");
    sprint_bool_string(circle.circle.soldermask, builder);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "(start): ");
    sprint_angle_string(circle.circle.start, builder, format_angle);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "(stop): ");
    sprint_angle_string(circle.circle.stop, builder, format_angle);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "(fill): ");
    sprint_bool_string(circle.circle.fill, builder);
    sprint_stringbuilder_put_chr(builder, '\n');
    sprint_element_destroy(&circle);

    sprint_stringbuilder_put_str(builder, "String test: ");
    sprint_str_string("my string raw", builder, SPRINT_PRIM_FORMAT_RAW);
    sprint_stringbuilder_put_chr(builder, ' ');
    sprint_str_string("my string cooked", builder, SPRINT_PRIM_FORMAT_COOKED);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_flush(builder, stdout);

    sprint_tuple tuple1 = sprint_tuple_of(1, 2), tuple2 = sprint_tuple_of(3, 4);

    printf("track %d\n", (int) sizeof(sprint_track));
    printf("pad_tht %d\n", (int) sizeof(sprint_pad_tht));
    printf("pad_smt %d\n", (int) sizeof(sprint_pad_smt));
    printf("zone %d\n", (int) sizeof(sprint_zone));
    printf("text %d\n", (int) sizeof(sprint_text));
    printf("circle %d\n", (int) sizeof(sprint_circle));
    printf("component %d\n", (int) sizeof(sprint_component));
    printf("group %d\n\n", (int) sizeof(sprint_group));
    printf("element %d\n", (int) sizeof(sprint_element));

    sprint_list* list = sprint_list_create(sizeof(sprint_tuple), 0);
    sprint_list_add(list, &tuple1);
    sprint_list_add(list, &tuple2);
    sprint_list_add(list, &tuple2);

    for (int i = 0; i < list->count; i++) {
        sprint_tuple tuple = *((sprint_tuple*)sprint_list_get(list, i));
        sprint_tuple_print(tuple, stdout, SPRINT_PRIM_FORMAT_COOKED);
        putchar('\n');
    }

    sprint_list_destroy(list);

    return 0;
}
