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
#include "libsprintpcb/token.h"
#include "libsprintpcb/errors.h"

int main(int argc, const char* argv[]) {
    sprint_output* output = sprint_output_create_file(stdout, false);
    sprint_tuple points[2] = {sprint_tuple_of(300, 400), sprint_tuple_of(500, 600)};

    sprint_element track;
    sprint_require(sprint_track_create(&track,
                                       SPRINT_LAYER_COPPER_BOTTOM,
                                       sprint_dist_mm(10),
                                       2, points));
    track.track.flat_start = true;
    sprint_element_output(&track, output, SPRINT_PRIM_FORMAT_RAW);
    sprint_element_output(&track, output, SPRINT_PRIM_FORMAT_DIST_MM);
    sprint_output_put_chr(output, '\n');
    sprint_element_destroy(&track);

    sprint_element text;
    sprint_require(sprint_text_create(&text,
                                      SPRINT_TEXT_VALUE,
                                      SPRINT_LAYER_COPPER_TOP,
                                      sprint_tuple_of(sprint_dist_mm(10), sprint_dist_mm(20)),
                                      sprint_dist_mm(10),
                                      "Hello, world!"));
    text.text.style = SPRINT_TEXT_STYLE_NARROW;
    text.text.rotation = sprint_angle_deg(90);
    text.text.visible = false;
    sprint_element_output(&text, output, SPRINT_PRIM_FORMAT_RAW);
    sprint_element_output(&text, output, SPRINT_PRIM_FORMAT_DIST_MM);
    sprint_output_put_chr(output, '\n');
    sprint_element_destroy(&text);

    sprint_require(sprint_plugin_begin(argc, argv));

    const char* test_text_io = "ZONE,LAYER=1,SOLDERMASK=true,WIDTH=0,P0=332158/408480,P1=332158/409045,P2=332158/409610;\n"
                               "TRACK;# This is a comment that will be ignored\n"
                               "TRACK,LAYER=7,WIDTH=2000,P0=928537/606471,P1=78537/606471,P2=78537/56471,P3=928537/56471,P4=928537/606471;";
    sprint_stringbuilder* builder = sprint_stringbuilder_create(7);
    sprint_tokenizer* tokenizer = sprint_tokenizer_from_str(test_text_io, false);
    sprint_token token = {0};
    while (true) {
        sprint_error error = sprint_tokenizer_next(tokenizer, &token, builder);
        if (error == SPRINT_ERROR_EOF) {
            sprint_assert(true, token.type != SPRINT_TOKEN_TYPE_INVALID);
            break;
        }

        size_t value_size = builder->count + 1;
        char value[value_size];
        sprint_stringbuilder_output(builder, value, value_size);
        sprint_assert(true, value != NULL);
        printf("%s at %d:%d: %s\n", SPRINT_TOKEN_TYPE_NAMES[token.type], token.origin.line, token.origin.pos, value);
    }
    sprint_tokenizer_destroy(tokenizer);

    sprint_element circle;
    sprint_require(sprint_circle_create(&circle,
            SPRINT_LAYER_MECHANICAL,
            10,
            sprint_tuple_of(sprint_dist_um(1), sprint_dist_um(20)),
            10));

    sprint_prim_format format_dist = SPRINT_PRIM_FORMAT_DIST_UM;
    sprint_prim_format format_angle = format_dist == SPRINT_PRIM_FORMAT_RAW ? format_dist : SPRINT_PRIM_FORMAT_COOKED;
    sprint_prim_format format_layer = format_angle;

    sprint_plugin_output(output);
    sprint_output_put_chr(output, '\n');

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
        sprint_tuple_output(tuple, output, SPRINT_PRIM_FORMAT_COOKED);
        putchar('\n');
    }

    sprint_list_destroy(list);
    sprint_output_destroy(output, NULL);

    return 0;
}
