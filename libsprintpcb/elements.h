//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_ELEMENTS_H
#define SPRINTPCB_ELEMENTS_H

#include "primitives.h"

#include <stdbool.h>

typedef struct sprint_element sprint_element;

typedef struct {
    bool has_id;
    int id;
    int num_connections;
    int* connections;
} sprint_link;

typedef struct {
    sprint_layer layer;
    sprint_dist width;
    int num_points;
    sprint_tuple* points;

    sprint_dist clear;
    bool cutout;
    bool soldermask;
    bool flatstart;
    bool flatend;
} sprint_track;

typedef enum {
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

typedef struct {
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
    unsigned int thermal_tracks_width;
    bool thermal_tracks_individual;
} sprint_pad_tht;

typedef struct {
    sprint_layer layer;
    sprint_tuple position;
    sprint_tuple size;

    sprint_link link;
    sprint_dist clear;
    bool soldermask;
    sprint_angle rotation;
    bool thermal;
    unsigned int thermal_tracks;
    int thermal_tracks_width;
} sprint_pad_smt;

typedef struct {
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

typedef enum {
    SPRINT_TEXT_REGULAR,
    SPRINT_TEXT_ID,
    SPRINT_TEXT_VALUE
} sprint_text_type;

typedef enum {
    // Narrow width text
    SPRINT_TEXT_STYLE_NARROW,

    // Regular width text
    SPRINT_TEXT_STYLE_REGULAR,

    // Wide width text
    SPRINT_TEXT_STYLE_WIDE
} sprint_text_style;

typedef enum {
    // Thin stroke text
    SPRINT_TEXT_THICKNESS_THIN,

    // Regular stroke text
    SPRINT_TEXT_THICKNESS_REGULAR,

    // Thick stroke text
    SPRINT_TEXT_THICKNESS_THICK
} sprint_text_thickness;

typedef struct {
    sprint_text_type type;
    sprint_layer layer;
    sprint_tuple position;
    sprint_dist height;
    char* text;

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

typedef struct {
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

typedef struct {
    sprint_text* text_id;
    sprint_text* text_value;

    int num_elements;
    sprint_element* elements;

    char* comment;
    bool use_pickplace;
    char* package;
    sprint_angle rotation;
} sprint_component;

typedef struct {
    int num_elements;
    sprint_element* elements;
} sprint_group;

typedef enum {
    SPRINT_ELEMENT_TRACK,
    SPRINT_ELEMENT_PAD_THT,
    SPRINT_ELEMENT_PAD_SMT,
    SPRINT_ELEMENT_ZONE,
    SPRINT_ELEMENT_TEXT,
    SPRINT_ELEMENT_CIRCLE,
    SPRINT_ELEMENT_COMPONENT,
    SPRINT_ELEMENT_GROUP
} sprint_element_type;

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

#endif //SPRINTPCB_ELEMENTS_H
