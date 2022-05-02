//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "elements.h"
#include "errors.h"

#include <stdlib.h>
#include <string.h>

sprint_element sprint_track_create(sprint_layer layer, sprint_dist width, int num_points, sprint_tuple* points)
{
    // todo input checking

    sprint_element element;
    memset(&element, 0, sizeof(element));
    element.type = SPRINT_ELEMENT_TRACK;

    // Required fields
    element.track.layer = layer;
    element.track.width = width;
    element.track.num_points = num_points;
    element.track.points = points;

    // Optional fields
    element.track.clear = 4000;
    element.track.cutout = false;
    element.track.soldermask = false;
    element.track.flatstart = false;
    element.track.flatend = false;

    return element;
}

sprint_element sprint_pad_tht_create(sprint_layer layer, sprint_tuple position, sprint_dist size,
                                     sprint_dist drill, sprint_pad_tht_form form)
{
    // todo input checking

    sprint_element element;
    memset(&element, 0, sizeof(element));
    element.type = SPRINT_ELEMENT_PAD_THT;

    // Required fields
    element.pad_tht.layer = layer;
    element.pad_tht.position = position;
    element.pad_tht.size = size;
    element.pad_tht.drill = drill;
    element.pad_tht.form = form;

    // Optional fields
    element.pad_tht.link.has_id = false;
    element.pad_tht.link.num_connections = 0;
    element.pad_tht.clear = 4000;
    element.pad_tht.soldermask = true;
    element.pad_tht.rotation = 0;
    element.pad_tht.via = false;
    element.pad_tht.thermal = false;
    element.pad_tht.thermal_tracks = 0x55555555;
    element.pad_tht.thermal_tracks_width = 100;
    element.pad_tht.thermal_tracks_individual = false;

    return element;
}

sprint_element sprint_pad_smt_create(sprint_layer layer, sprint_tuple position, sprint_tuple size)
{
    // todo input checking

    sprint_element element;
    memset(&element, 0, sizeof(element));
    element.type = SPRINT_ELEMENT_PAD_SMT;

    // Required fields
    element.pad_smt.layer = layer;
    element.pad_smt.position = position;
    element.pad_smt.size = size;

    // Optional fields
    element.pad_smt.link.has_id = false;
    element.pad_smt.link.num_connections = 0;
    element.pad_smt.clear = 4000;
    element.pad_smt.soldermask = true;
    element.pad_smt.rotation = 0;
    element.pad_smt.thermal = false;
    element.pad_smt.thermal_tracks = 0x55;
    element.pad_smt.thermal_tracks_width = 100;

    return element;
}

sprint_element sprint_zone_create(sprint_layer layer, sprint_dist width, int num_points, sprint_tuple* points)
{
    // todo input checking

    sprint_element element;
    memset(&element, 0, sizeof(element));
    element.type = SPRINT_ELEMENT_ZONE;

    // Required fields
    element.zone.layer = layer;
    element.zone.width = width;
    element.zone.num_points = num_points;
    element.zone.points = points;

    // Optional fields
    element.zone.clear = 4000;
    element.zone.cutout = false;
    element.zone.soldermask = false;
    element.zone.hatch = false;
    element.zone.hatch_auto = true;

    return element;
}

sprint_element sprint_text_create(sprint_text_type type, sprint_layer layer, sprint_tuple position,
                                  sprint_dist height, char* text)
{
    // todo input checking

    sprint_element element;
    memset(&element, 0, sizeof(element));
    element.type = SPRINT_ELEMENT_TEXT;

    // Required fields
    element.text.type = type;
    element.text.layer = layer;
    element.text.position = position;
    element.text.height = height;
    element.text.text = text;

    // Optional fields
    element.text.clear = 4000;
    element.text.cutout = false;
    element.text.soldermask = false;
    element.text.style = SPRINT_TEXT_STYLE_REGULAR;
    element.text.thickness = SPRINT_TEXT_THICKNESS_REGULAR;
    element.text.rotation = 0;
    element.text.mirror_horizontal = false;
    element.text.mirror_vertical = false;
    element.text.visible = true;

    return element;
}

sprint_element sprint_circle_create(sprint_layer layer, sprint_dist width, sprint_tuple center, sprint_dist radius)
{
    // todo input checking

    sprint_element element;
    memset(&element, 0, sizeof(element));
    element.type = SPRINT_ELEMENT_CIRCLE;

    // Required fields
    element.circle.layer = layer;
    element.circle.width = width;
    element.circle.center = center;
    element.circle.radius = radius;

    // Optional fields
    element.circle.clear = 4000;
    element.circle.cutout = false;
    element.circle.soldermask = false;
    element.circle.start = 0;
    element.circle.stop = 0;
    element.circle.fill = false;

    return element;
}

sprint_element sprint_component_create(sprint_text* text_id, sprint_text* text_value,
                                       int num_elements, sprint_element* elements)
{
    // todo input checking

    sprint_element element;
    memset(&element, 0, sizeof(element));
    element.type = SPRINT_ELEMENT_COMPONENT;

    // Required fields
    element.component.text_id = text_id;
    element.component.text_value = text_value;
    element.component.num_elements = num_elements;
    element.component.elements = elements;

    // Optional fields
    element.component.comment = NULL;
    element.component.use_pickplace = false;
    element.component.package = NULL;
    element.component.rotation = 0;

    return element;
}

sprint_element sprint_group_create(int num_elements, sprint_element* elements)
{
    // todo input checking

    sprint_element element;
    memset(&element, 0, sizeof(element));
    element.type = SPRINT_ELEMENT_GROUP;

    // Required fields
    element.group.num_elements = num_elements;
    element.group.elements = elements;

    return element;
}

sprint_error sprint_element_destroy(sprint_element* element)
{
    // Make sure the element is not null
    if (element == NULL)
        return SPRINT_ERROR_ARGUMENT_NULL;

    // If the element is not parsed, free nothing - the application is responsible for cleaning up
    if (!element->parsed)
        return SPRINT_ERROR_NONE;

    // Free allocated memory based on the type
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
                free(element->component.text_id);
                element->component.text_id = NULL;
            }

            // Free the value text
            if (element->component.text_value != NULL) {
                free(element->component.text_value);
                element->component.text_value = NULL;
            }

            // Free the elements
            element->component.num_elements = 0;
            if (element->component.elements != NULL) {
                free(element->component.elements);
                element->component.elements = NULL;
            }

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
            // Free the elements
            element->group.num_elements = 0;
            if (element->group.elements != NULL) {
                free(element->group.elements);
                element->group.elements = NULL;
            }
            break;

        default:
            // Unknown elements cannot be freed
            return SPRINT_ERROR_ARGUMENT_RANGE;
    }

    // Finally, free the parsed element
    free(element);
    return SPRINT_ERROR_NONE;
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

    sprint_error error = sprint_element_type_string(type, builder, format);
    if (error == SPRINT_ERROR_NONE)
        return sprint_stringbuilder_flush(builder, stream);

    sprint_stringbuilder_destroy(builder);
    return error;
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
            return sprint_stringbuilder_put_int(builder, type);

        case SPRINT_PRIM_FORMAT_COOKED:
            type_name = SPRINT_ELEMENT_TYPE_NAMES[type];
            if (type_name == NULL)
                return SPRINT_ERROR_ARGUMENT_RANGE;
            return sprint_stringbuilder_put_str(builder, type_name);

        default:
            return SPRINT_ERROR_ARGUMENT_RANGE;
    }
}
