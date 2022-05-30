//
// libsprintpcb: element creation and output
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_ELEMENTS_H
#define SPRINTPCB_ELEMENTS_H

#include "primitives.h"
#include "output.h"
#include "errors.h"

#include <stdbool.h>

typedef struct sprint_element sprint_element;

/**
 * The maximum recursive element depth.
 */
extern const int SPRINT_ELEMENT_DEPTH;

/**
 * The length of the indentation to add for every depth layer to raw output.
 */
extern const int SPRINT_ELEMENT_INDENT;

typedef struct sprint_link {
    bool has_id;
    int id;
    int num_connections;
    int* connections;
} sprint_link;

typedef struct sprint_track {
    sprint_layer layer;
    sprint_dist width;
    int num_points;
    sprint_tuple* points;

    sprint_dist clear;
    bool cutout;
    bool soldermask;
    bool flat_start;
    bool flat_end;
} sprint_track;
bool sprint_track_valid(sprint_track* track);

typedef enum sprint_pad_tht_form {
    // Round
    SPRINT_PAD_THT_FORM_ROUND = 1,

    // Octagon
    SPRINT_PAD_THT_FORM_OCTAGON,

    // Square
    SPRINT_PAD_THT_FORM_SQUARE,

    // Rounded, landscape
    SPRINT_PAD_THT_FORM_TRANSVERSE_ROUNDED,

    // Octagon, landscape
    SPRINT_PAD_THT_FORM_TRANSVERSE_OCTAGON,

    // Rectangular, landscape
    SPRINT_PAD_THT_FORM_TRANSVERSE_RECTANGULAR,

    // Rounded, portrait
    SPRINT_PAD_THT_FORM_HIGH_ROUNDED,

    // Octagon, portrait
    SPRINT_PAD_THT_FORM_HIGH_OCTAGON,

    // Rectangular, portrait
    SPRINT_PAD_THT_FORM_HIGH_RECTANGULAR
} sprint_pad_tht_form;
extern const char* SPRINT_PAD_THT_FORM_NAMES[];
bool sprint_pad_tht_form_valid(sprint_pad_tht_form form);
sprint_error sprint_pad_tht_form_output(sprint_pad_tht_form form, sprint_output* output, sprint_prim_format format);

typedef struct sprint_pad_tht {
    // Required

    sprint_layer layer;
    sprint_tuple position;
    sprint_dist size;
    sprint_dist drill;
    sprint_pad_tht_form form;

    // Optional

    sprint_link link;
    sprint_dist clear;
    bool soldermask;
    sprint_angle rotation;
    bool via;
    bool thermal;
    int thermal_tracks;
    int thermal_tracks_width;
    bool thermal_tracks_individual;
} sprint_pad_tht;
bool sprint_pad_tht_valid(sprint_pad_tht* pad);

typedef struct sprint_pad_smt {
    sprint_layer layer;
    sprint_tuple position;
    sprint_dist width;
    sprint_dist height;

    sprint_link link;
    sprint_dist clear;
    bool soldermask;
    sprint_angle rotation;
    bool thermal;
    int thermal_tracks;
    int thermal_tracks_width;
} sprint_pad_smt;
bool sprint_pad_smt_valid(sprint_pad_smt* pad);

typedef struct sprint_zone {
    sprint_layer layer;
    sprint_dist width;
    int num_points;
    sprint_tuple* points;

    sprint_dist clear;
    bool cutout;
    bool soldermask;
    bool hatch;
    bool hatch_auto;
    sprint_dist hatch_width;
} sprint_zone;
bool sprint_zone_valid(sprint_zone* zone);

typedef enum sprint_text_type {
    SPRINT_TEXT_REGULAR,
    SPRINT_TEXT_ID,
    SPRINT_TEXT_VALUE
} sprint_text_type;
extern const char* SPRINT_TEXT_TYPE_NAMES[];
extern const char* SPRINT_TEXT_TYPE_KEYWORDS[];
bool sprint_text_type_valid(sprint_text_type type);
const char* sprint_text_type_to_keyword(sprint_text_type type);
bool sprint_text_type_from_keyword(sprint_text_type* type, const char* keyword);
sprint_error sprint_text_type_output(sprint_text_type type, sprint_output* output, sprint_prim_format format);

typedef enum sprint_text_style {
    // Narrow width text
    SPRINT_TEXT_STYLE_NARROW,

    // Regular width text
    SPRINT_TEXT_STYLE_REGULAR,

    // Wide width text
    SPRINT_TEXT_STYLE_WIDE
} sprint_text_style;
extern const char* SPRINT_TEXT_STYLE_NAMES[];
bool sprint_text_style_valid(sprint_text_style style);
sprint_error sprint_text_style_output(sprint_text_style style, sprint_output* output, sprint_prim_format format);

typedef enum sprint_text_thickness {
    // Thin stroke text
    SPRINT_TEXT_THICKNESS_THIN,

    // Regular stroke text
    SPRINT_TEXT_THICKNESS_REGULAR,

    // Thick stroke text
    SPRINT_TEXT_THICKNESS_THICK
} sprint_text_thickness;
extern const char* SPRINT_TEXT_THICKNESS_NAMES[];
bool sprint_text_thickness_valid(sprint_text_thickness thickness);
sprint_error sprint_text_thickness_output(sprint_text_thickness thickness, sprint_output* output,
                                          sprint_prim_format format);

typedef struct sprint_text {
    sprint_layer layer;
    sprint_tuple position;
    sprint_dist height;
    char* text;

    sprint_text_type subtype;
    sprint_dist clear;
    bool cutout;
    bool soldermask;
    sprint_text_style style;
    sprint_text_thickness thickness;
    sprint_angle rotation;
    bool mirror_horizontal;
    bool mirror_vertical;

    bool visible;
} sprint_text;
bool sprint_text_valid(sprint_text* text);

typedef struct sprint_circle {
    sprint_layer layer;
    sprint_dist width;
    sprint_tuple center;
    sprint_dist radius;

    sprint_dist clear;
    bool cutout;
    bool soldermask;
    sprint_angle start;
    sprint_angle stop;
    bool fill;
} sprint_circle;
bool sprint_circle_valid(sprint_circle* circle);

typedef struct sprint_component {
    sprint_element* text_id;
    sprint_element* text_value;

    int num_elements;
    sprint_element* elements;

    char* comment;
    bool use_pickplace;
    char* package;
    sprint_angle rotation;
} sprint_component;
bool sprint_component_valid(sprint_component* component);

typedef struct sprint_group {
    int num_elements;
    sprint_element* elements;
} sprint_group;
bool sprint_group_valid(sprint_group* group);

typedef enum sprint_element_type {
    SPRINT_ELEMENT_TRACK,
    SPRINT_ELEMENT_PAD_THT,
    SPRINT_ELEMENT_PAD_SMT,
    SPRINT_ELEMENT_ZONE,
    SPRINT_ELEMENT_TEXT,
    SPRINT_ELEMENT_CIRCLE,
    SPRINT_ELEMENT_COMPONENT,
    SPRINT_ELEMENT_GROUP
} sprint_element_type;
bool sprint_element_type_valid(sprint_element_type type);

struct sprint_element {
    // The type of this element
    sprint_element_type type;

    // Whether the element has been created by parsing, which uses malloc for all buffers
    bool parsed;

    union {
        sprint_track track;
        sprint_pad_tht pad_tht;
        sprint_pad_smt pad_smt;
        sprint_zone zone;
        sprint_text text;
        sprint_circle circle;
        sprint_component component;
        sprint_group group;
    };
};

extern const char* SPRINT_ELEMENT_TYPE_NAMES[];
extern const char* SPRINT_ELEMENT_TYPE_KEYWORDS_OPENING[];
extern const char* SPRINT_ELEMENT_TYPE_KEYWORDS_CLOSING[];
const char* sprint_element_type_to_keyword(sprint_element_type type, bool closing);
bool sprint_element_type_from_keyword(sprint_element_type* type, bool* closing, const char* keyword);
sprint_error sprint_element_type_output(sprint_element_type type, sprint_output* output, bool closing,
                                        sprint_prim_format format);

sprint_error sprint_track_default(sprint_element* element, bool clear);
sprint_error sprint_track_create(sprint_element* element, sprint_layer layer, sprint_dist width,
                                 int num_points, sprint_tuple* points);
sprint_error sprint_pad_tht_default(sprint_element* element, bool clear);
sprint_error sprint_pad_tht_create(sprint_element* element, sprint_layer layer, sprint_tuple position,
                                   sprint_dist size, sprint_dist drill, sprint_pad_tht_form form);
sprint_error sprint_pad_smt_default(sprint_element* element, bool clear);
sprint_error sprint_pad_smt_create(sprint_element* element, sprint_layer layer, sprint_tuple position,
                                     sprint_dist width, sprint_dist height);
sprint_error sprint_zone_default(sprint_element* element, bool clear);
sprint_error sprint_zone_create(sprint_element* element, sprint_layer layer, sprint_dist width,
                                int num_points, sprint_tuple* points);
sprint_error sprint_text_default(sprint_element* element, bool clear);
sprint_error sprint_text_create(sprint_element* element, sprint_text_type type, sprint_layer layer,
                                sprint_tuple position, sprint_dist height, char* text);
sprint_error sprint_circle_default(sprint_element* element, bool clear);
sprint_error sprint_circle_create(sprint_element* element, sprint_layer layer, sprint_dist width,
                                  sprint_tuple center, sprint_dist radius);
sprint_error sprint_component_default(sprint_element* element, bool clear);
sprint_error sprint_component_create(sprint_element* element, sprint_element* text_id, sprint_element* text_value,
                                     int num_elements, sprint_element* elements);
sprint_error sprint_group_default(sprint_element* element, bool clear);
sprint_error sprint_group_create(sprint_element* element, int num_elements, sprint_element* elements);

const char* sprint_element_tag(sprint_element* element);
sprint_error sprint_element_output(sprint_element* element, sprint_output* output, sprint_prim_format format);
bool sprint_element_valid(sprint_element* element);
sprint_error sprint_element_destroy(sprint_element* element);

#endif //SPRINTPCB_ELEMENTS_H
