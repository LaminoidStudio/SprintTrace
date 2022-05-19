//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "elements.h"
#include "primitives.h"
#include "token.h"
#include "output.h"
#include "errors.h"

#include <stdlib.h>
#include <string.h>

const int SPRINT_ELEMENT_DEPTH = 1000;
const int SPRINT_ELEMENT_INDENT = 2;

const char* SPRINT_ELEMENT_TYPE_NAMES[] = {
        [SPRINT_ELEMENT_TRACK] = "track",
        [SPRINT_ELEMENT_PAD_THT] = "THT pad",
        [SPRINT_ELEMENT_PAD_SMT] = "SMT pad",
        [SPRINT_ELEMENT_ZONE] = "zone",
        [SPRINT_ELEMENT_TEXT] = "text",
        [SPRINT_ELEMENT_CIRCLE] = "circle",
        [SPRINT_ELEMENT_COMPONENT] = "component",
        [SPRINT_ELEMENT_GROUP] = "group"
};

const char* SPRINT_ELEMENT_TYPE_KEYWORDS_OPENING[] = {
        [SPRINT_ELEMENT_TRACK] = "TRACK",
        [SPRINT_ELEMENT_PAD_THT] = "PAD",
        [SPRINT_ELEMENT_PAD_SMT] = "SMDPAD",
        [SPRINT_ELEMENT_ZONE] = "ZONE",
        [SPRINT_ELEMENT_TEXT] = "TEXT",
        [SPRINT_ELEMENT_CIRCLE] = "CIRCLE",
        [SPRINT_ELEMENT_COMPONENT] = "BEGIN_COMPONENT",
        [SPRINT_ELEMENT_GROUP] = "GROUP"
};

const char* SPRINT_ELEMENT_TYPE_KEYWORDS_CLOSING[] = {
        [SPRINT_ELEMENT_TRACK] = NULL,
        [SPRINT_ELEMENT_PAD_THT] = NULL,
        [SPRINT_ELEMENT_PAD_SMT] = NULL,
        [SPRINT_ELEMENT_ZONE] = NULL,
        [SPRINT_ELEMENT_TEXT] = NULL,
        [SPRINT_ELEMENT_CIRCLE] = NULL,
        [SPRINT_ELEMENT_COMPONENT] = "END_COMPONENT",
        [SPRINT_ELEMENT_GROUP] = "END_GROUP"
};

const char* sprint_element_type_to_keyword(sprint_element_type type, bool closing)
{
    if (type < 0) return NULL;

    if (!closing && type < sizeof(SPRINT_ELEMENT_TYPE_KEYWORDS_OPENING) / sizeof(const char *))
        return SPRINT_ELEMENT_TYPE_KEYWORDS_OPENING[type];
    else if (closing && type < sizeof(SPRINT_ELEMENT_TYPE_KEYWORDS_CLOSING) / sizeof(const char *))
        return SPRINT_ELEMENT_TYPE_KEYWORDS_CLOSING[type];

    sprint_throw_format(false, "element type unknown: %d", type);
    return NULL;
}

bool sprint_element_type_from_keyword(sprint_element_type* type, bool* closing, const char* keyword)
{
    if (type == NULL || closing == NULL || keyword == NULL) return false;

    // Preset closing to false
    *closing = false;

    // Determine the type
    if (strcasecmp(keyword, sprint_element_type_to_keyword(SPRINT_ELEMENT_TRACK, false)) == 0)
        *type = SPRINT_ELEMENT_TRACK;
    else if (strcasecmp(keyword, sprint_element_type_to_keyword(SPRINT_ELEMENT_PAD_THT, false)) == 0)
        *type = SPRINT_ELEMENT_PAD_THT;
    else if (strcasecmp(keyword, sprint_element_type_to_keyword(SPRINT_ELEMENT_PAD_SMT, false)) == 0)
        *type = SPRINT_ELEMENT_PAD_SMT;
    else if (strcasecmp(keyword, sprint_element_type_to_keyword(SPRINT_ELEMENT_ZONE, false)) == 0)
        *type = SPRINT_ELEMENT_ZONE;
    else if (strcasecmp(keyword, sprint_element_type_to_keyword(SPRINT_ELEMENT_TEXT, false)) == 0)
        *type = SPRINT_ELEMENT_TEXT;
    else if (strcasecmp(keyword, sprint_element_type_to_keyword(SPRINT_ELEMENT_CIRCLE, false)) == 0)
        *type = SPRINT_ELEMENT_CIRCLE;
    else if (strcasecmp(keyword, sprint_element_type_to_keyword(SPRINT_ELEMENT_COMPONENT, false)) == 0)
        *type = SPRINT_ELEMENT_COMPONENT;
    else if (strcasecmp(keyword, sprint_element_type_to_keyword(SPRINT_ELEMENT_GROUP, false)) == 0)
        *type = SPRINT_ELEMENT_GROUP;
    else if (strcasecmp(keyword, sprint_element_type_to_keyword(SPRINT_ELEMENT_COMPONENT, true)) == 0) {
        *closing = true;
        *type = SPRINT_ELEMENT_COMPONENT;
    }
    else if (strcasecmp(keyword, sprint_element_type_to_keyword(SPRINT_ELEMENT_GROUP, true)) == 0) {
        *closing = true;
        *type = SPRINT_ELEMENT_GROUP;
    } else
        return false;

    return true;
}

sprint_error sprint_element_type_output(sprint_element_type type, sprint_output* output, bool closing,
                                        sprint_prim_format format)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_element_type_valid(type) || !sprint_prim_format_valid(format))
        return SPRINT_ERROR_ARGUMENT_RANGE;

    // Determine, if the output format is cooked
    bool cooked = sprint_prim_format_cooked(format);

    // Don't output anything for cooked and closing
    if (cooked && closing)
        return SPRINT_ERROR_NONE;

    // Determine the correct name
    const char* type_name = NULL;
    if (!cooked)
        type_name = sprint_element_type_to_keyword(type, closing);
    else if (type < sizeof(SPRINT_ELEMENT_TYPE_NAMES) / sizeof(const char*))
        type_name = SPRINT_ELEMENT_TYPE_NAMES[type];
    else {
        sprint_throw_format(false, "type without name: %d", type);
        return SPRINT_ERROR_INTERNAL;
    }

    // If there is no name, return a format error
    if (!sprint_assert(false, type_name != NULL))
        return SPRINT_ERROR_ARGUMENT_FORMAT;

    // Otherwise, just print the name
    return sprint_rethrow(sprint_output_put_str(output, type_name));
}

bool sprint_track_valid(sprint_track* track)
{
    return track != NULL && sprint_layer_valid(track->layer) && sprint_size_valid(track->width) &&
        track->num_points >= 2 && track->points != NULL && sprint_size_valid(track->clear);
}

static const sprint_track SPRINT_TRACK_DEFAULT = {
        .clear = 4000,
        .cutout = false,
        .soldermask = false,
        .flat_start = false,
        .flat_end = false
};

sprint_error sprint_track_create(sprint_element* element, sprint_layer layer, sprint_dist width,
                                   int num_points, sprint_tuple* points)
{
    if (element == NULL || num_points > 0 && points == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    memset(element, 0, sizeof(*element));
    element->type = SPRINT_ELEMENT_TRACK;

    // Required fields
    element->track.layer = layer;
    element->track.width = width;
    element->track.num_points = num_points;
    element->track.points = points;

    // Optional fields
    element->track.clear = SPRINT_TRACK_DEFAULT.clear;
    element->track.cutout = SPRINT_TRACK_DEFAULT.cutout;
    element->track.soldermask = SPRINT_TRACK_DEFAULT.soldermask;
    element->track.flat_start = SPRINT_TRACK_DEFAULT.flat_start;
    element->track.flat_end = SPRINT_TRACK_DEFAULT.flat_end;

    return sprint_track_valid(&element->track) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ARGUMENT_RANGE;
}

const char* SPRINT_PAD_THT_FORM_NAMES[] = {
        [SPRINT_PAD_THT_FORM_ROUND] = "round",
        [SPRINT_PAD_THT_FORM_OCTAGON] = "octagon",
        [SPRINT_PAD_THT_FORM_SQUARE] = "square",
        [SPRINT_PAD_THT_FORM_TRANSVERSE_ROUNDED] = "transverse rounded",
        [SPRINT_PAD_THT_FORM_TRANSVERSE_OCTAGON] = "transverse octagon",
        [SPRINT_PAD_THT_FORM_TRANSVERSE_RECTANGULAR] = "transverse rectangular",
        [SPRINT_PAD_THT_FORM_HIGH_ROUNDED] = "high rounded",
        [SPRINT_PAD_THT_FORM_HIGH_OCTAGON] = "high octagon",
        [SPRINT_PAD_THT_FORM_HIGH_RECTANGULAR] = "high rectangular"
};

bool sprint_pad_tht_form_valid(sprint_pad_tht_form form)
{
    return form >= SPRINT_PAD_THT_FORM_ROUND && form <= SPRINT_PAD_THT_FORM_HIGH_RECTANGULAR;
}

sprint_error sprint_pad_tht_form_output(sprint_pad_tht_form form, sprint_output* output, sprint_prim_format format)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_pad_tht_form_valid(form) || !sprint_prim_format_valid(format)) return SPRINT_ERROR_ARGUMENT_RANGE;

    // Write the string based on the format
    const char* form_name;
    if (sprint_prim_format_cooked(format)) {
        form_name = SPRINT_PAD_THT_FORM_NAMES[form];
        if (!sprint_assert(false, form_name != NULL))
            return SPRINT_ERROR_ASSERTION;
        return sprint_rethrow(sprint_output_put_str(output, form_name));
    } else
        return sprint_rethrow(sprint_output_put_int(output, form));
}

static const sprint_pad_tht SPRINT_PAD_THT_DEFAULT = {
        .link.has_id = false,
        .link.num_connections = 0,
        .clear = 4000,
        .soldermask = true,
        .rotation = 0,
        .via = false,
        .thermal = false,
        .thermal_tracks = 0x55555555,
        .thermal_tracks_width = 100,
        .thermal_tracks_individual = false
};

bool sprint_pad_tht_valid(sprint_pad_tht* pad)
{
    return pad != NULL && sprint_layer_valid(pad->layer) && sprint_tuple_valid(pad->position) &&
        sprint_size_valid(pad->size) && sprint_size_valid(pad->drill) &&
        sprint_pad_tht_form_valid(pad->form) && pad->link.num_connections >= 0 &&
        sprint_size_valid(pad->clear) && sprint_angle_valid(pad->rotation) &&
        pad->thermal_tracks_width >= 50 && pad->thermal_tracks_width <= 300;
}

sprint_error sprint_pad_tht_create(sprint_element* element, sprint_layer layer, sprint_tuple position,
                                   sprint_dist size, sprint_dist drill, sprint_pad_tht_form form)
{
    if (element == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    memset(element, 0, sizeof(*element));
    element->type = SPRINT_ELEMENT_PAD_THT;

    // Required fields
    element->pad_tht.layer = layer;
    element->pad_tht.position = position;
    element->pad_tht.size = size;
    element->pad_tht.drill = drill;
    element->pad_tht.form = form;

    // Optional fields
    element->pad_tht.link = SPRINT_PAD_THT_DEFAULT.link;
    element->pad_tht.clear = SPRINT_PAD_THT_DEFAULT.clear;
    element->pad_tht.soldermask = SPRINT_PAD_THT_DEFAULT.soldermask;
    element->pad_tht.rotation = SPRINT_PAD_THT_DEFAULT.rotation;
    element->pad_tht.via = SPRINT_PAD_THT_DEFAULT.via;
    element->pad_tht.thermal = SPRINT_PAD_THT_DEFAULT.thermal;
    element->pad_tht.thermal_tracks = SPRINT_PAD_THT_DEFAULT.thermal_tracks;
    element->pad_tht.thermal_tracks_width = SPRINT_PAD_THT_DEFAULT.thermal_tracks_width;
    element->pad_tht.thermal_tracks_individual = SPRINT_PAD_THT_DEFAULT.thermal_tracks_individual;

    return sprint_pad_tht_valid(&element->pad_tht) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ARGUMENT_RANGE;
}

bool sprint_pad_smt_valid(sprint_pad_smt* pad)
{
    return pad != NULL && sprint_layer_valid(pad->layer) && sprint_tuple_valid(pad->position) &&
           sprint_size_valid(pad->width) && sprint_size_valid(pad->height) &&
           pad->link.num_connections >= 0 && sprint_size_valid(pad->clear) &&
           sprint_angle_valid(pad->rotation) && pad->thermal_tracks >= 0 && pad->thermal_tracks <= 0xff &&
           pad->thermal_tracks_width >= 50 && pad->thermal_tracks_width <= 300;
}

static const sprint_pad_smt SPRINT_PAD_SMT_DEFAULT = {
        .link.has_id = false,
        .link.num_connections = 0,
        .clear = 4000,
        .soldermask = true,
        .rotation = 0,
        .thermal = false,
        .thermal_tracks = 0x55,
        .thermal_tracks_width = 100
};

sprint_error sprint_pad_smt_create(sprint_element* element, sprint_layer layer, sprint_tuple position,
                                     sprint_dist width, sprint_dist height)
{
    if (element == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    memset(element, 0, sizeof(*element));
    element->type = SPRINT_ELEMENT_PAD_SMT;

    // Required fields
    element->pad_smt.layer = layer;
    element->pad_smt.position = position;
    element->pad_smt.width = width;
    element->pad_smt.height = height;

    // Optional fields
    element->pad_smt.link = SPRINT_PAD_SMT_DEFAULT.link;
    element->pad_smt.clear = SPRINT_PAD_SMT_DEFAULT.clear;
    element->pad_smt.soldermask = SPRINT_PAD_SMT_DEFAULT.soldermask;
    element->pad_smt.rotation = SPRINT_PAD_SMT_DEFAULT.rotation;
    element->pad_smt.thermal = SPRINT_PAD_SMT_DEFAULT.thermal;
    element->pad_smt.thermal_tracks = SPRINT_PAD_SMT_DEFAULT.thermal_tracks;
    element->pad_smt.thermal_tracks_width = SPRINT_PAD_SMT_DEFAULT.thermal_tracks_width;

    return sprint_pad_smt_valid(&element->pad_smt) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ARGUMENT_RANGE;
}

bool sprint_zone_valid(sprint_zone* zone)
{
    return zone != NULL && sprint_layer_valid(zone->layer) && sprint_size_valid(zone->width) &&
           zone->num_points >= 2 && zone->points != NULL && sprint_size_valid(zone->clear);
}

static const sprint_zone SPRINT_ZONE_DEFAULT = {
        .clear = 4000,
        .cutout = false,
        .soldermask = false,
        .hatch = false,
        .hatch_auto = true
};

sprint_error sprint_zone_create(sprint_element* element, sprint_layer layer, sprint_dist width,
                                int num_points, sprint_tuple* points)
{
    if (element == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    memset(element, 0, sizeof(*element));
    element->type = SPRINT_ELEMENT_ZONE;

    // Required fields
    element->zone.layer = layer;
    element->zone.width = width;
    element->zone.num_points = num_points;
    element->zone.points = points;

    // Optional fields
    element->zone.clear = SPRINT_ZONE_DEFAULT.clear;
    element->zone.cutout = SPRINT_ZONE_DEFAULT.cutout;
    element->zone.soldermask = SPRINT_ZONE_DEFAULT.soldermask;
    element->zone.hatch = SPRINT_ZONE_DEFAULT.hatch;
    element->zone.hatch_auto = SPRINT_ZONE_DEFAULT.hatch_auto;

    return sprint_zone_valid(&element->zone) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ARGUMENT_RANGE;
}

const char* SPRINT_TEXT_TYPE_NAMES[] = {
        [SPRINT_TEXT_REGULAR] = "text",
        [SPRINT_TEXT_ID] = "ID text",
        [SPRINT_TEXT_VALUE] = "value text"
};

const char* SPRINT_TEXT_TYPE_KEYWORDS[] = {
        [SPRINT_TEXT_REGULAR] = "TEXT",
        [SPRINT_TEXT_ID] = "ID_TEXT",
        [SPRINT_TEXT_VALUE] = "VALUE_TEXT"
};

bool sprint_text_type_valid(sprint_text_type type)
{
    return type >= SPRINT_TEXT_REGULAR && type <= SPRINT_TEXT_VALUE;
}

sprint_error sprint_text_type_output(sprint_text_type type, sprint_output* output, sprint_prim_format format)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_text_type_valid(type) || !sprint_prim_format_valid(format)) return SPRINT_ERROR_ARGUMENT_RANGE;

    // Determine the name based on the format
    const char* type_name;
    if (sprint_prim_format_cooked(format))
        type_name = SPRINT_TEXT_TYPE_NAMES[type];
    else
        type_name = SPRINT_TEXT_TYPE_KEYWORDS[type];

    // Validate and output the string
    if (!sprint_assert(false, type_name != NULL))
        return SPRINT_ERROR_ASSERTION;
    return sprint_rethrow(sprint_output_put_str(output, type_name));
}

const char* SPRINT_TEXT_STYLE_NAMES[] = {
        [SPRINT_TEXT_STYLE_NARROW] = "narrow",
        [SPRINT_TEXT_STYLE_REGULAR] = "regular",
        [SPRINT_TEXT_STYLE_WIDE] = "wide"
};

bool sprint_text_style_valid(sprint_text_style style)
{
    return style >= SPRINT_TEXT_STYLE_NARROW && style <= SPRINT_TEXT_STYLE_WIDE;
}

sprint_error sprint_text_style_output(sprint_text_style style, sprint_output* output, sprint_prim_format format)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_text_style_valid(style) || !sprint_prim_format_valid(format)) return SPRINT_ERROR_ARGUMENT_RANGE;

    // Write the string based on the format
    if (sprint_prim_format_cooked(format)) {
        const char* style_name = SPRINT_TEXT_STYLE_NAMES[style];
        if (!sprint_assert(false, style_name != NULL))
            return SPRINT_ERROR_ASSERTION;
        return sprint_rethrow(sprint_output_put_str(output, style_name));
    } else
        return sprint_rethrow(sprint_output_put_int(output, style));
}

const char* SPRINT_TEXT_THICKNESS_NAMES[] = {
        [SPRINT_TEXT_THICKNESS_THIN] = "thin",
        [SPRINT_TEXT_THICKNESS_REGULAR] = "regular",
        [SPRINT_TEXT_THICKNESS_THICK] = "thick"
};

bool sprint_text_thickness_valid(sprint_text_thickness thickness)
{
    return thickness >= SPRINT_TEXT_THICKNESS_THIN && thickness <= SPRINT_TEXT_THICKNESS_THICK;
}

sprint_error sprint_text_thickness_output(sprint_text_thickness thickness, sprint_output* output,
                                          sprint_prim_format format)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_text_thickness_valid(thickness) || !sprint_prim_format_valid(format))
        return SPRINT_ERROR_ARGUMENT_RANGE;

    // Write the string based on the format
    if (sprint_prim_format_cooked(format)) {
        const char* thickness_name = SPRINT_TEXT_THICKNESS_NAMES[thickness];
        if (!sprint_assert(false, thickness_name != NULL))
            return SPRINT_ERROR_ASSERTION;
        return sprint_rethrow(sprint_output_put_str(output, thickness_name));
    } else
        return sprint_rethrow(sprint_output_put_int(output, thickness));
}

bool sprint_text_valid(sprint_text* text)
{
    return text != NULL && sprint_layer_valid(text->layer) && sprint_tuple_valid(text->position) &&
           (text->text == NULL || strchr(text->text, SPRINT_STRING_DELIMITER) == NULL) &&
           sprint_size_valid(text->height) && sprint_size_valid(text->clear) &&
           sprint_text_style_valid(text->style) && sprint_text_thickness_valid(text->thickness) &&
           sprint_angle_valid(text->rotation);
}

static const sprint_text SPRINT_TEXT_DEFAULT = {
        .clear = 4000,
        .cutout = false,
        .soldermask = false,
        .style = SPRINT_TEXT_STYLE_REGULAR,
        .thickness = SPRINT_TEXT_THICKNESS_REGULAR,
        .rotation = 0,
        .mirror_horizontal = false,
        .mirror_vertical = false,
        .visible = true
};

sprint_error sprint_text_create(sprint_element* element, sprint_text_type type, sprint_layer layer,
                                sprint_tuple position, sprint_dist height, char* text)
{
    if (element == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_text_type_valid(type)) return SPRINT_ERROR_ARGUMENT_RANGE;

    memset(element, 0, sizeof(*element));
    element->type = SPRINT_ELEMENT_TEXT;

    // Required fields
    element->text.layer = layer;
    element->text.position = position;
    element->text.height = height;
    element->text.text = text;

    // Optional fields
    element->text.clear = SPRINT_TEXT_DEFAULT.clear;
    element->text.cutout = SPRINT_TEXT_DEFAULT.cutout;
    element->text.soldermask = SPRINT_TEXT_DEFAULT.soldermask;
    element->text.style = SPRINT_TEXT_DEFAULT.style;
    element->text.thickness = SPRINT_TEXT_DEFAULT.thickness;
    element->text.rotation = SPRINT_TEXT_DEFAULT.rotation;
    element->text.mirror_horizontal = SPRINT_TEXT_DEFAULT.mirror_horizontal;
    element->text.mirror_vertical = SPRINT_TEXT_DEFAULT.mirror_vertical;
    element->text.visible = SPRINT_TEXT_DEFAULT.visible;

    return sprint_text_valid(&element->text) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ARGUMENT_RANGE;
}

bool sprint_circle_valid(sprint_circle* circle)
{
    return circle != NULL && sprint_layer_valid(circle->layer) && sprint_size_valid(circle->width) &&
           sprint_tuple_valid(circle->center) && sprint_size_valid(circle->radius) &&
           sprint_size_valid(circle->clear) && sprint_angle_valid(circle->start) &&
           sprint_angle_valid(circle->stop);
}

static const sprint_circle SPRINT_CIRCLE_DEFAULT = {
        .clear = 4000,
        .cutout = false,
        .soldermask = false,
        .start = 0,
        .stop = 0,
        .fill = false
};

sprint_error sprint_circle_create(sprint_element* element, sprint_layer layer, sprint_dist width,
                                  sprint_tuple center, sprint_dist radius)
{
    if (element == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    memset(element, 0, sizeof(*element));
    element->type = SPRINT_ELEMENT_CIRCLE;

    // Required fields
    element->circle.layer = layer;
    element->circle.width = width;
    element->circle.center = center;
    element->circle.radius = radius;

    // Optional fields
    element->circle.clear = SPRINT_CIRCLE_DEFAULT.clear;
    element->circle.cutout = SPRINT_CIRCLE_DEFAULT.cutout;
    element->circle.soldermask = SPRINT_CIRCLE_DEFAULT.soldermask;
    element->circle.start = SPRINT_CIRCLE_DEFAULT.start;
    element->circle.stop = SPRINT_CIRCLE_DEFAULT.stop;
    element->circle.fill = SPRINT_CIRCLE_DEFAULT.fill;

    return sprint_circle_valid(&element->circle) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ARGUMENT_RANGE;
}

bool sprint_component_valid(sprint_component* component)
{
    return component != NULL && component->text_id->type == SPRINT_ELEMENT_TEXT && component->text_value->type == SPRINT_ELEMENT_TEXT &&
           sprint_text_valid(&component->text_id->text) && sprint_text_valid(&component->text_value->text) &&
           component->num_elements >= 0 && (component->num_elements == 0) == (component->elements == NULL) &&
           sprint_angle_valid(component->rotation);
}

bool sprint_element_type_valid(sprint_element_type type)
{
    return type >= SPRINT_ELEMENT_TRACK && type <= SPRINT_ELEMENT_GROUP;
}

static const sprint_component SPRINT_COMPONENT_DEFAULT = {
        .comment = NULL,
        .use_pickplace = false,
        .package = NULL,
        .rotation = 0
};

sprint_error sprint_component_create(sprint_element* element, sprint_element* text_id, sprint_element* text_value,
                                       int num_elements, sprint_element* elements)
{
    if (element == NULL || text_id == NULL || text_value == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (text_id->type != SPRINT_ELEMENT_TEXT || text_value->type != SPRINT_ELEMENT_TEXT)
        return SPRINT_ERROR_ARGUMENT_FORMAT;

    memset(element, 0, sizeof(*element));
    element->type = SPRINT_ELEMENT_COMPONENT;

    // Required fields
    element->component.text_id = text_id;
    element->component.text_value = text_value;
    element->component.num_elements = num_elements;
    element->component.elements = elements;

    // Optional fields
    element->component.comment = SPRINT_COMPONENT_DEFAULT.comment;
    element->component.use_pickplace = SPRINT_COMPONENT_DEFAULT.use_pickplace;
    element->component.package = SPRINT_COMPONENT_DEFAULT.package;
    element->component.rotation = SPRINT_COMPONENT_DEFAULT.rotation;

    return sprint_component_valid(&element->component) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ARGUMENT_RANGE;
}

bool sprint_group_valid(sprint_group* group)
{
    return group != NULL && group->num_elements >= 0 && (group->num_elements == 0) == (group->elements == NULL);
}

sprint_error sprint_group_create(sprint_element* element, int num_elements, sprint_element* elements)
{
    if (element == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    memset(element, 0, sizeof(*element));
    element->type = SPRINT_ELEMENT_GROUP;

    // Required fields
    element->group.num_elements = num_elements;
    element->group.elements = elements;

    return sprint_group_valid(&element->group) ? SPRINT_ERROR_NONE : SPRINT_ERROR_ARGUMENT_RANGE;
}

static sprint_error sprint_indent_output_internal(sprint_output* output, int depth)
{
    sprint_error error = SPRINT_ERROR_NONE;
    for (depth--; depth >= 0; depth--)
        for (int indent = 0; indent < SPRINT_ELEMENT_INDENT; indent++)
            sprint_chain(error, sprint_output_put_chr(output, ' '));
    return sprint_rethrow(error);
}

static sprint_error sprint_element_prefix_output_internal(sprint_output* output, bool cooked, int depth)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (cooked)
        sprint_chain(error, sprint_output_put_str(output, "sprint_element{type="));
    else
        sprint_chain(error, sprint_indent_output_internal(output, depth));
    return sprint_rethrow(error);
}

static sprint_error sprint_element_suffix_output_internal(sprint_output* output, bool cooked)
{
    sprint_error error = SPRINT_ERROR_NONE;
    if (cooked)
        sprint_chain(error, sprint_output_put_chr(output, '}'));
    else {
        sprint_chain(error, sprint_output_put_chr(output, SPRINT_STATEMENT_TERMINATOR));
        sprint_chain(error, sprint_output_put_chr(output, '\n'));
    }
    return sprint_rethrow(error);
}

static sprint_error sprint_array_output_internal(sprint_output* output, bool cooked, int index,
                                                 const char* keyword_raw, const char* keyword_cooked)
{
    // Put the statement separator
    sprint_error error = SPRINT_ERROR_NONE;
    if (cooked)
        sprint_chain(error, sprint_output_put_str(output, ", "));
    else
        sprint_chain(error, sprint_output_put_chr(output, SPRINT_STATEMENT_SEPARATOR));

    // Put the keyword
    sprint_chain(error, sprint_output_put_str(output, cooked ? keyword_cooked : keyword_raw));

    // Put the optional index
    if (index >= 0)
        sprint_chain(error, sprint_output_put_int(output, index));

    // Put the value separator
    sprint_chain(error, sprint_output_put_chr(output, cooked ? '=' : SPRINT_VALUE_SEPARATOR));
    return sprint_rethrow(error);
}

static sprint_error sprint_param_output_internal(sprint_output* output, bool cooked,
                                                 const char* tag_raw, const char* tag_cooked)
{
    return sprint_array_output_internal(output, cooked, -1, tag_raw, tag_cooked);
}

static sprint_error sprint_element_output_internal(sprint_element* element, sprint_output* output,
                                                   sprint_prim_format format, int depth);

static sprint_error sprint_track_output_internal(sprint_track* track, sprint_output* output,
                                                 sprint_prim_format format, int depth)
{
    bool cooked = sprint_prim_format_cooked(format);
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_element_prefix_output_internal(output, cooked, depth));
    sprint_chain(error, sprint_element_type_output(SPRINT_ELEMENT_TRACK, output, false, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "LAYER", "layer"));
    sprint_chain(error, sprint_layer_output(track->layer, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "WIDTH", "width"));
    sprint_chain(error, sprint_dist_output(track->width, output, format));
    if (track->clear != SPRINT_TRACK_DEFAULT.clear) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "CLEAR", "clear"));
        sprint_chain(error, sprint_dist_output(track->clear, output, format));
    }
    if (track->cutout != SPRINT_TRACK_DEFAULT.cutout) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "CUTOUT", "cutout"));
        sprint_chain(error, sprint_bool_output(track->cutout, output));
    }
    if (track->soldermask != SPRINT_TRACK_DEFAULT.soldermask) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "SOLDERMASK", "soldermask"));
        sprint_chain(error, sprint_bool_output(track->soldermask, output));
    }
    if (track->flat_start != SPRINT_TRACK_DEFAULT.flat_start) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "FLATSTART", "flat start"));
        sprint_chain(error, sprint_bool_output(track->flat_start, output));
    }
    if (track->flat_end != SPRINT_TRACK_DEFAULT.flat_end) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "FLATEND", "flat end"));
        sprint_chain(error, sprint_bool_output(track->flat_end, output));
    }
    for (int index = 0; index < track->num_points; index++) {
        sprint_chain(error, sprint_array_output_internal(output, cooked, index, "P", "p"));
        sprint_chain(error, sprint_tuple_output(track->points[index], output, format));
    }
    sprint_chain(error, sprint_element_suffix_output_internal(output, cooked));
    return sprint_rethrow(error);
}

static sprint_error sprint_pad_tht_output_internal(sprint_pad_tht* pad, sprint_output* output,
                                                   sprint_prim_format format, int depth)
{
    bool cooked = sprint_prim_format_cooked(format);
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_element_prefix_output_internal(output, cooked, depth));
    sprint_chain(error, sprint_element_type_output(SPRINT_ELEMENT_PAD_THT, output, false, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "LAYER", "layer"));
    sprint_chain(error, sprint_layer_output(pad->layer, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "POS", "position"));
    sprint_chain(error, sprint_tuple_output(pad->position, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "SIZE", "size"));
    sprint_chain(error, sprint_dist_output(pad->size, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "DRILL", "drill"));
    sprint_chain(error, sprint_dist_output(pad->drill, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "FORM", "form"));
    sprint_chain(error, sprint_pad_tht_form_output(pad->form, output, format));
    if (pad->clear != SPRINT_PAD_THT_DEFAULT.clear) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "CLEAR", "clear"));
        sprint_chain(error, sprint_dist_output(pad->clear, output, format));
    }
    if (pad->soldermask != SPRINT_PAD_THT_DEFAULT.soldermask) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "SOLDERMASK", "soldermask"));
        sprint_chain(error, sprint_bool_output(pad->soldermask, output));
    }
    if (pad->rotation != SPRINT_PAD_THT_DEFAULT.rotation) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "ROTATION", "rotation"));
        sprint_chain(error, sprint_angle_output(pad->rotation, output, sprint_prim_format_of(SPRINT_PRIM_FORMAT_ANGLE_COARSE, cooked)));
    }
    if (pad->via != SPRINT_PAD_THT_DEFAULT.via) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "VIA", "via"));
        sprint_chain(error, sprint_bool_output(pad->via, output));
    }
    if (pad->thermal != SPRINT_PAD_THT_DEFAULT.thermal) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "THERMAL", "thermal"));
        sprint_chain(error, sprint_bool_output(pad->thermal, output));
        sprint_chain(error, sprint_param_output_internal(output, cooked, "THERMAL_TRACKS", "tracks"));
        sprint_chain(error, sprint_int_output(pad->thermal_tracks, output));
        if (pad->thermal_tracks_width != SPRINT_PAD_THT_DEFAULT.thermal_tracks_width) {
            sprint_chain(error, sprint_param_output_internal(output, cooked, "THERMAL_TRACKS_WIDTH", "tracks width"));
            sprint_chain(error, sprint_int_output(pad->thermal_tracks_width, output));
        }
        if (pad->thermal_tracks_individual != SPRINT_PAD_THT_DEFAULT.thermal_tracks_individual) {
            sprint_chain(error, sprint_param_output_internal(output, cooked, "THERMAL_TRACKS_INDIVIDUAL", "tracks individual"));
            sprint_chain(error, sprint_bool_output(pad->thermal_tracks_individual, output));
        }
    }
    if (pad->link.has_id) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "PAD_ID", "pad ID"));
        sprint_chain(error, sprint_int_output(pad->link.id, output));
    }
    for (int index = 0; index < pad->link.num_connections; index++) {
        sprint_chain(error, sprint_array_output_internal(output, cooked, index, "CON", "c"));
        sprint_chain(error, sprint_int_output(pad->link.connections[index], output));
    }
    sprint_chain(error, sprint_element_suffix_output_internal(output, cooked));
    return sprint_rethrow(error);
}

static sprint_error sprint_pad_smt_output_internal(sprint_pad_smt* pad, sprint_output* output,
                                                   sprint_prim_format format, int depth)
{
    bool cooked = sprint_prim_format_cooked(format);
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_element_prefix_output_internal(output, cooked, depth));
    sprint_chain(error, sprint_element_type_output(SPRINT_ELEMENT_PAD_SMT, output, false, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "LAYER", "layer"));
    sprint_chain(error, sprint_layer_output(pad->layer, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "POS", "position"));
    sprint_chain(error, sprint_tuple_output(pad->position, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "SIZE_X", "width"));
    sprint_chain(error, sprint_dist_output(pad->width, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "SIZE_Y", "height"));
    sprint_chain(error, sprint_dist_output(pad->height, output, format));
    if (pad->clear != SPRINT_PAD_SMT_DEFAULT.clear) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "CLEAR", "clear"));
        sprint_chain(error, sprint_dist_output(pad->clear, output, format));
    }
    if (pad->soldermask != SPRINT_PAD_SMT_DEFAULT.soldermask) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "SOLDERMASK", "soldermask"));
        sprint_chain(error, sprint_bool_output(pad->soldermask, output));
    }
    if (pad->rotation != SPRINT_PAD_SMT_DEFAULT.rotation) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "ROTATION", "rotation"));
        sprint_chain(error, sprint_angle_output(pad->rotation, output, sprint_prim_format_of(SPRINT_PRIM_FORMAT_ANGLE_COARSE, cooked)));
    }
    if (pad->thermal != SPRINT_PAD_SMT_DEFAULT.thermal) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "THERMAL", "thermal"));
        sprint_chain(error, sprint_bool_output(pad->thermal, output));
        sprint_chain(error, sprint_param_output_internal(output, cooked, "THERMAL_TRACKS", "tracks"));
        sprint_chain(error, sprint_int_output(pad->thermal_tracks, output));
        if (pad->thermal_tracks_width != SPRINT_PAD_SMT_DEFAULT.thermal_tracks_width) {
            sprint_chain(error, sprint_param_output_internal(output, cooked, "THERMAL_TRACKS_WIDTH", "tracks width"));
            sprint_chain(error, sprint_int_output(pad->thermal_tracks_width, output));
        }
    }
    if (pad->link.has_id) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "PAD_ID", "pad ID"));
        sprint_chain(error, sprint_int_output(pad->link.id, output));
    }
    for (int index = 0; index < pad->link.num_connections; index++) {
        sprint_chain(error, sprint_array_output_internal(output, cooked, index, "CON", "c"));
        sprint_chain(error, sprint_int_output(pad->link.connections[index], output));
    }
    sprint_chain(error, sprint_element_suffix_output_internal(output, cooked));
    return sprint_rethrow(error);
}

static sprint_error sprint_zone_output_internal(sprint_zone* zone, sprint_output* output,
                                                sprint_prim_format format, int depth)
{
    bool cooked = sprint_prim_format_cooked(format);
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_element_prefix_output_internal(output, cooked, depth));
    sprint_chain(error, sprint_element_type_output(SPRINT_ELEMENT_ZONE, output, false, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "LAYER", "layer"));
    sprint_chain(error, sprint_layer_output(zone->layer, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "WIDTH", "width"));
    sprint_chain(error, sprint_dist_output(zone->width, output, format));
    if (zone->clear != SPRINT_ZONE_DEFAULT.clear) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "CLEAR", "clear"));
        sprint_chain(error, sprint_dist_output(zone->clear, output, format));
    }
    if (zone->cutout != SPRINT_ZONE_DEFAULT.cutout) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "CUTOUT", "cutout"));
        sprint_chain(error, sprint_bool_output(zone->cutout, output));
    }
    if (zone->soldermask != SPRINT_ZONE_DEFAULT.soldermask) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "SOLDERMASK", "soldermask"));
        sprint_chain(error, sprint_bool_output(zone->soldermask, output));
    }
    if (zone->hatch != SPRINT_ZONE_DEFAULT.hatch) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "HATCH", "hatch"));
        sprint_chain(error, sprint_bool_output(zone->hatch, output));
        if (zone->hatch_auto != SPRINT_ZONE_DEFAULT.hatch_auto) {
            sprint_chain(error, sprint_param_output_internal(output, cooked, "HATCH_AUTO", "hatch auto"));
            sprint_chain(error, sprint_bool_output(zone->hatch_auto, output));
        }
        if (!zone->hatch_auto) {
            sprint_chain(error, sprint_param_output_internal(output, cooked, "HATCH_WIDTH", "hatch width"));
            sprint_chain(error, sprint_dist_output(zone->hatch_width, output, format));
        }
    }
    for (int index = 0; index < zone->num_points; index++) {
        sprint_chain(error, sprint_array_output_internal(output, cooked, index, "P", "p"));
        sprint_chain(error, sprint_tuple_output(zone->points[index], output, format));
    }
    sprint_chain(error, sprint_element_suffix_output_internal(output, cooked));
    return sprint_rethrow(error);
}

static sprint_error sprint_text_output_internal(sprint_text* text, sprint_output* output, sprint_prim_format format,
                                                sprint_text_type type, int depth)
{
    bool cooked = sprint_prim_format_cooked(format);
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_element_prefix_output_internal(output, cooked, depth));
    sprint_chain(error, sprint_text_type_output(type, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "LAYER", "layer"));
    sprint_chain(error, sprint_layer_output(text->layer, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "POS", "position"));
    sprint_chain(error, sprint_tuple_output(text->position, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "TEXT", "text"));
    sprint_chain(error, sprint_str_output(text->text, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "HEIGHT", "height"));
    sprint_chain(error, sprint_dist_output(text->height, output, format));
    if (text->clear != SPRINT_TEXT_DEFAULT.clear) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "CLEAR", "clear"));
        sprint_chain(error, sprint_dist_output(text->clear, output, format));
    }
    if (text->cutout != SPRINT_TEXT_DEFAULT.cutout) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "CUTOUT", "cutout"));
        sprint_chain(error, sprint_bool_output(text->cutout, output));
    }
    if (text->soldermask != SPRINT_TEXT_DEFAULT.soldermask) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "SOLDERMASK", "soldermask"));
        sprint_chain(error, sprint_bool_output(text->soldermask, output));
    }
    if (text->style != SPRINT_TEXT_DEFAULT.style) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "STYLE", "style"));
        sprint_chain(error, sprint_text_style_output(text->style, output, format));
    }
    if (text->thickness != SPRINT_TEXT_DEFAULT.thickness) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "THICKNESS", "thickness"));
        sprint_chain(error, sprint_text_thickness_output(text->thickness, output, format));
    }
    if (text->rotation != SPRINT_TEXT_DEFAULT.rotation) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "ROTATION", "rotation"));
        sprint_chain(error, sprint_angle_output(text->rotation, output, sprint_prim_format_of(SPRINT_PRIM_FORMAT_ANGLE_WHOLE, cooked)));
    }
    if (text->mirror_horizontal != SPRINT_TEXT_DEFAULT.mirror_horizontal) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "MIRROR_HORZ", "mirror hz"));
        sprint_chain(error, sprint_bool_output(text->mirror_horizontal, output));
    }
    if (text->mirror_vertical != SPRINT_TEXT_DEFAULT.mirror_vertical) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "MIRROR_VERT", "mirror vt"));
        sprint_chain(error, sprint_bool_output(text->mirror_vertical, output));
    }
    if ((type == SPRINT_TEXT_ID || type == SPRINT_TEXT_VALUE) && text->visible != SPRINT_TEXT_DEFAULT.visible) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "VISIBLE", "visible"));
        sprint_chain(error, sprint_bool_output(text->visible, output));
    }
    sprint_chain(error, sprint_element_suffix_output_internal(output, cooked));
    return sprint_rethrow(error);
}

static sprint_error sprint_circle_output_internal(sprint_circle* circle, sprint_output* output,
                                                  sprint_prim_format format, int depth)
{
    bool cooked = sprint_prim_format_cooked(format);
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_element_prefix_output_internal(output, cooked, depth));
    sprint_chain(error, sprint_element_type_output(SPRINT_ELEMENT_CIRCLE, output, false, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "LAYER", "layer"));
    sprint_chain(error, sprint_layer_output(circle->layer, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "WIDTH", "width"));
    sprint_chain(error, sprint_dist_output(circle->width, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "CENTER", "center"));
    sprint_chain(error, sprint_tuple_output(circle->center, output, format));
    sprint_chain(error, sprint_param_output_internal(output, cooked, "RADIUS", "radius"));
    sprint_chain(error, sprint_dist_output(circle->radius, output, format));
    if (circle->clear != SPRINT_CIRCLE_DEFAULT.clear) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "CLEAR", "clear"));
        sprint_chain(error, sprint_dist_output(circle->clear, output, format));
    }
    if (circle->cutout != SPRINT_CIRCLE_DEFAULT.cutout) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "CUTOUT", "cutout"));
        sprint_chain(error, sprint_bool_output(circle->cutout, output));
    }
    if (circle->soldermask != SPRINT_CIRCLE_DEFAULT.soldermask) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "SOLDERMASK", "soldermask"));
        sprint_chain(error, sprint_bool_output(circle->soldermask, output));
    }
    if (circle->start != SPRINT_CIRCLE_DEFAULT.start) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "START", "start"));
        sprint_chain(error, sprint_angle_output(circle->start, output, format));
    }
    if (circle->stop != SPRINT_CIRCLE_DEFAULT.stop) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "STOP", "stop"));
        sprint_chain(error, sprint_angle_output(circle->stop, output, format));
    }
    if (circle->fill != SPRINT_CIRCLE_DEFAULT.fill) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "FILL", "fill"));
        sprint_chain(error, sprint_bool_output(circle->fill, output));
    }
    sprint_chain(error, sprint_element_suffix_output_internal(output, cooked));
    return sprint_rethrow(error);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
static sprint_error sprint_component_output_internal(sprint_component* component, sprint_output* output,
                                                     sprint_prim_format format, int depth)
{
    // Make sure that there is a valid ID and value text
    if (component->text_id->type != SPRINT_ELEMENT_TEXT || component->text_value->type != SPRINT_ELEMENT_TEXT)
        return SPRINT_ERROR_STATE_INVALID;

    // Make sure that there is at least one child element
    if (component->num_elements < 1)
        return SPRINT_ERROR_NONE;

    // Check, if the format is cooked
    bool cooked = sprint_prim_format_cooked(format);

    // Write the component header
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_element_prefix_output_internal(output, cooked, depth));
    sprint_chain(error, sprint_element_type_output(SPRINT_ELEMENT_COMPONENT, output, false, format));
    if (component->comment != NULL) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "COMMENT", "comment"));
        sprint_chain(error, sprint_str_output(component->comment, output, format));
    }
    if (component->use_pickplace != SPRINT_COMPONENT_DEFAULT.use_pickplace) {
        sprint_chain(error, sprint_param_output_internal(output, cooked, "USE_PICKPLACE", "pickplace"));
        sprint_chain(error, sprint_bool_output(component->use_pickplace, output));
    }
    if (component->use_pickplace) {
        if (component->package != NULL) {
            sprint_chain(error, sprint_param_output_internal(output, cooked, "PACKAGE", "package"));
            sprint_chain(error, sprint_str_output(component->comment, output, format));
        }
        if (component->rotation != SPRINT_COMPONENT_DEFAULT.rotation) {
            sprint_chain(error, sprint_param_output_internal(output, cooked, "ROTATION", "rotation"));
            sprint_chain(error, sprint_angle_output(component->rotation, output, sprint_prim_format_of(SPRINT_PRIM_FORMAT_ANGLE_WHOLE, cooked)));
        }
    }
    if (!sprint_chain(error, sprint_element_suffix_output_internal(output, cooked)))
        return sprint_rethrow(error);

    // Write the contained elements
    for (int index = 0; index < component->num_elements; index++) {
        sprint_element* element = &component->elements[index];
        if (element == NULL)
            return SPRINT_ERROR_STATE_INVALID;
        if (!sprint_chain(error, sprint_element_output_internal(element, output, format, depth + 1)))
            return sprint_rethrow(error);
    }

    // Write the ID and value texts
    sprint_chain(error, sprint_text_output_internal(&component->text_id->text,
                                                    output, format, SPRINT_TEXT_ID, depth + 1));
    sprint_chain(error, sprint_text_output_internal(&component->text_value->text,
                                                    output, format, SPRINT_TEXT_VALUE, depth + 1));

    // Write the component footer
    sprint_chain(error, sprint_element_prefix_output_internal(output, cooked, depth));
    sprint_chain(error, sprint_element_type_output(SPRINT_ELEMENT_COMPONENT, output, true, format));
    sprint_chain(error, sprint_element_suffix_output_internal(output, cooked));
    return sprint_rethrow(error);
}

static sprint_error sprint_group_output_internal(sprint_group* group, sprint_output* output,
                                                 sprint_prim_format format, int depth)
{
    // Make sure that there is at least one child element
    if (group->num_elements < 1) return SPRINT_ERROR_NONE;

    // Check, if the format is cooked
    bool cooked = sprint_prim_format_cooked(format);

    // Write the component header
    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_element_prefix_output_internal(output, cooked, depth));
    sprint_chain(error, sprint_element_type_output(SPRINT_ELEMENT_GROUP, output, false, format));
    if (!sprint_chain(error, sprint_element_suffix_output_internal(output, cooked)))
        return sprint_rethrow(error);

    // Write the contained elements
    for (int index = 0; index < group->num_elements; index++) {
        sprint_element* element = &group->elements[index];
        if (element == NULL)
            return SPRINT_ERROR_STATE_INVALID;
        if (!sprint_chain(error, sprint_element_output_internal(element, output, format, depth + 1)))
            return sprint_rethrow(error);
    }

    // Write the component footer
    sprint_chain(error, sprint_element_prefix_output_internal(output, cooked, depth));
    sprint_chain(error, sprint_element_type_output(SPRINT_ELEMENT_GROUP, output, true, format));
    sprint_chain(error, sprint_element_suffix_output_internal(output, cooked));
    return sprint_rethrow(error);
}

static sprint_error sprint_element_output_internal(sprint_element* element, sprint_output* output,
                                                   sprint_prim_format format, int depth) {
    if (element == NULL || output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (!sprint_element_type_valid(element->type) || !sprint_prim_format_valid(format))
        return SPRINT_ERROR_ARGUMENT_RANGE;
    if (depth < 0 || depth >= SPRINT_ELEMENT_DEPTH) return SPRINT_ERROR_RECURSION;

    // Append the elements based on type and format
    sprint_error error = SPRINT_ERROR_NONE;
    switch (element->type) {
        case SPRINT_ELEMENT_TRACK:
            sprint_chain(error, sprint_track_output_internal(&element->track, output, format, depth));
            break;
        case SPRINT_ELEMENT_PAD_THT:
            sprint_chain(error, sprint_pad_tht_output_internal(&element->pad_tht, output, format, depth));
            break;
        case SPRINT_ELEMENT_PAD_SMT:
            sprint_chain(error, sprint_pad_smt_output_internal(&element->pad_smt, output, format, depth));
            break;
        case SPRINT_ELEMENT_ZONE:
            sprint_chain(error, sprint_zone_output_internal(&element->zone, output, format, depth));
            break;
        case SPRINT_ELEMENT_TEXT:
            sprint_chain(error, sprint_text_output_internal(&element->text, output, format, SPRINT_TEXT_REGULAR, depth));
            break;
        case SPRINT_ELEMENT_CIRCLE:
            sprint_chain(error, sprint_circle_output_internal(&element->circle, output, format, depth));
            break;
        case SPRINT_ELEMENT_COMPONENT:
            sprint_chain(error, sprint_component_output_internal(&element->component, output, format, depth));
            break;
        case SPRINT_ELEMENT_GROUP:
            sprint_chain(error, sprint_group_output_internal(&element->group, output, format, depth));
            break;
        default:
            sprint_throw_format(false, "element type unknown: %d", element->type);
            return SPRINT_ERROR_ARGUMENT_RANGE;
    }

    return sprint_rethrow(error);
}
#pragma clang diagnostic pop

sprint_error sprint_element_output(sprint_element* element, sprint_output* output, sprint_prim_format format)
{
    return sprint_element_output_internal(element, output, format, 0);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
static sprint_error sprint_element_destroy_internal(sprint_element* element, int depth)
{
    // Make sure the element is not null
    if (element == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (depth < 0 || depth >= SPRINT_ELEMENT_DEPTH) return SPRINT_ERROR_RECURSION;

    // If the element is not parsed, free nothing - the application is responsible for cleaning up
    if (!element->parsed)
        return SPRINT_ERROR_NONE;

    // Free allocated memory based on the type
    sprint_error first_error = SPRINT_ERROR_NONE, last_error = SPRINT_ERROR_NONE;
    switch (element->type) {
        case SPRINT_ELEMENT_TRACK:
            // Free the points
            element->track.num_points = 0;
            if (element->track.points != NULL) {
                free(element->track.points);
                element->track.points = NULL;
            }
            break;

        case SPRINT_ELEMENT_PAD_THT:
            // Free the connections
            element->pad_tht.link.num_connections = 0;
            if (element->pad_tht.link.connections != NULL) {
                free(element->pad_tht.link.connections);
                element->pad_tht.link.connections = NULL;
            }
            break;

        case SPRINT_ELEMENT_PAD_SMT:
            // Free the connections
            element->pad_smt.link.num_connections = 0;
            if (element->pad_smt.link.connections != NULL) {
                free(element->pad_smt.link.connections);
                element->pad_smt.link.connections = NULL;
            }
            break;

        case SPRINT_ELEMENT_ZONE:
            // Free the points
            element->zone.num_points = 0;
            if (element->zone.points != NULL) {
                free(element->zone.points);
                element->zone.points = NULL;
            }
            break;

        case SPRINT_ELEMENT_TEXT:
            // Free the text
            if (element->text.text != NULL) {
                free(element->text.text);
                element->text.text = NULL;
            }
            break;

        case SPRINT_ELEMENT_CIRCLE:
            // Do nothing
            break;

        case SPRINT_ELEMENT_COMPONENT:
            // Free the ID text
            if (element->component.text_id != NULL) {
                sprint_check(last_error = sprint_element_destroy_internal(element->component.text_id, depth + 1));
                first_error = last_error;
                element->component.text_id = NULL;
            }

            // Free the value text
            if (element->component.text_value != NULL) {
                sprint_check(last_error = sprint_element_destroy_internal(element->component.text_value, depth + 1));
                first_error = first_error == SPRINT_ERROR_NONE ? last_error : first_error;
                element->component.text_value = NULL;
            }

            // Free the elements recursively
            if (element->component.elements != NULL) {
                for (int index = 0; index < element->component.num_elements; index++) {
                    sprint_check(last_error = sprint_element_destroy_internal(&element->component.elements[index], depth + 1));
                    first_error = first_error == SPRINT_ERROR_NONE ? last_error : first_error;
                }
                free(element->component.elements);
                element->component.elements = NULL;
            }
            element->component.num_elements = 0;

            // Free the comment
            if (element->component.comment != NULL) {
                free(element->component.comment);
                element->component.comment = NULL;
            }

            // Free the package
            if (element->component.package != NULL) {
                free(element->component.package);
                element->component.package = NULL;
            }
            break;

        case SPRINT_ELEMENT_GROUP:
            // Free the elements recursively
            if (element->group.elements != NULL) {
                for (int index = 0; index < element->group.num_elements; index++) {
                    sprint_check(last_error = sprint_element_destroy_internal(&element->group.elements[index], depth + 1));
                    first_error = first_error == SPRINT_ERROR_NONE ? last_error : first_error;
                }
                free(element->group.elements);
                element->group.elements = NULL;
            }
            element->group.num_elements = 0;
            break;

        default:
            // Unknown elements cannot be freed
            sprint_throw_format(false, "could not free unknown element: %d", element->type);
            return SPRINT_ERROR_ARGUMENT_RANGE;
    }

    // Finally, free the parsed element
    free(element);
    return sprint_rethrow(first_error);
}
#pragma clang diagnostic pop

sprint_error sprint_element_destroy(sprint_element* element)
{
    return sprint_element_destroy_internal(element, 0);
}
