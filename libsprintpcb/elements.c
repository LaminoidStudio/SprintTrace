//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "elements.h"
#include "errors.h"

#include <stdlib.h>
#include <string.h>

const int SPRINT_ELEMENT_DEPTH = 1000;

bool sprint_track_valid(sprint_track* track)
{
    return track != NULL && sprint_layer_valid(track->layer) && sprint_size_valid(track->width) &&
        track->num_points >= 0 && (track->num_points == 0) == (track->points == NULL) &&
        sprint_size_valid(track->clear);
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

bool sprint_pad_tht_form_valid(sprint_pad_tht_form form)
{
    return form >= SPRINT_PAD_THT_FORM_ROUND && form <= SPRINT_PAD_THT_FORM_HIGH_RECTANGULAR;
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
           zone->num_points >= 0 && (zone->num_points == 0) == (zone->points == NULL) &&
           sprint_size_valid(zone->clear);
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

bool sprint_text_type_valid(sprint_text_type type)
{
    return type >= SPRINT_TEXT_REGULAR && type <= SPRINT_TEXT_VALUE;
}

bool sprint_text_style_valid(sprint_text_style style)
{
    return style >= SPRINT_TEXT_STYLE_NARROW && style <= SPRINT_TEXT_STYLE_WIDE;
}

bool sprint_text_thickness_valid(sprint_text_thickness thickness)
{
    return thickness >= SPRINT_TEXT_THICKNESS_THIN && thickness <= SPRINT_TEXT_THICKNESS_THICK;
}

bool sprint_text_valid(sprint_text* text)
{
    return text != NULL && sprint_text_type_valid(text->type) && sprint_layer_valid(text->layer) &&
           sprint_tuple_valid(text->position) && sprint_size_valid(text->height) &&
           sprint_size_valid(text->clear) && sprint_text_style_valid(text->style) &&
           sprint_text_thickness_valid(text->thickness) && sprint_angle_valid(text->rotation);
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

    memset(element, 0, sizeof(*element));
    element->type = SPRINT_ELEMENT_TEXT;

    // Required fields
    element->text.type = type;
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
    return component != NULL && sprint_text_valid(component->text_id) && sprint_text_valid(component->text_value) &&
           component->num_elements >= 0 && (component->num_elements == 0) == (component->elements == NULL) &&
           sprint_angle_valid(component->rotation);
}

static const sprint_component SPRINT_COMPONENT_DEFAULT = {
        .comment = NULL,
        .use_pickplace = false,
        .package = NULL,
        .rotation = 0
};

sprint_error sprint_component_create(sprint_element* element, sprint_text* text_id, sprint_text* text_value,
                                       int num_elements, sprint_element* elements)
{
    if (element == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

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
    sprint_error error = SPRINT_ERROR_NONE;
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
                if (element->component.text_id->text != NULL) {
                    free(element->component.text_id->text);
                    element->component.text_id->text = NULL;
                }
                free(element->component.text_id);
                element->component.text_id = NULL;
            }

            // Free the value text
            if (element->component.text_value != NULL) {
                if (element->component.text_value->text != NULL) {
                    free(element->component.text_value->text);
                    element->component.text_value->text = NULL;
                }
                free(element->component.text_value);
                element->component.text_value = NULL;
            }

            // Free the elements recursively
            if (element->component.elements != NULL) {
                for (int index = 0; index < element->component.num_elements; index++)
                    sprint_check(sprint_element_destroy_internal(&element->component.elements[index], depth + 1));
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
                for (int index = 0; index < element->group.num_elements; index++)
                    sprint_check(sprint_element_destroy_internal(&element->group.elements[index], depth + 1));
                free(element->group.elements);
                element->group.elements = NULL;
            }
            element->group.num_elements = 0;
            break;

        default:
            // Unknown elements cannot be freed
            return SPRINT_ERROR_ARGUMENT_RANGE;
    }

    // Finally, free the parsed element
    free(element);
    return SPRINT_ERROR_NONE;
}
#pragma clang diagnostic pop

sprint_error sprint_element_destroy(sprint_element* element)
{
    return sprint_element_destroy_internal(element, 0);
}

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

sprint_error sprint_element_type_print(sprint_element_type type, FILE* stream, sprint_prim_format format)
{
    if (stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_stringbuilder* builder = sprint_stringbuilder_create(7);
    if (builder == NULL)
        return SPRINT_ERROR_MEMORY;

    sprint_error error = SPRINT_ERROR_NONE;
    sprint_chain(error, sprint_element_type_string(type, builder, format));
    if (!sprint_chain(error, sprint_stringbuilder_flush(builder, stream)))
        sprint_stringbuilder_destroy(builder);

    return sprint_rethrow(error);
}

sprint_error sprint_element_type_string(sprint_element_type type, sprint_stringbuilder* builder,
                                        sprint_prim_format format)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (type >= sizeof(SPRINT_ELEMENT_TYPE_NAMES) / sizeof(const char*)) return SPRINT_ERROR_ARGUMENT_RANGE;

    // Write the string based on the format
    const char* type_name;
    switch (format) {
        case SPRINT_PRIM_FORMAT_RAW:
            return sprint_rethrow(sprint_stringbuilder_put_int(builder, type));

        case SPRINT_PRIM_FORMAT_COOKED:
            type_name = SPRINT_ELEMENT_TYPE_NAMES[type];
            if (type_name == NULL)
                return SPRINT_ERROR_ARGUMENT_RANGE;
            return sprint_rethrow(sprint_stringbuilder_put_str(builder, type_name));

        default:
            return SPRINT_ERROR_ARGUMENT_RANGE;
    }
}
