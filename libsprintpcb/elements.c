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

sprint_element sprint_text_create()
{

}

sprint_element sprint_circle_create()
{

}

sprint_element sprint_component_create()
{

}

sprint_element sprint_group_create()
{

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
            break;
        case SPRINT_ELEMENT_PAD_SMT:
            break;
        case SPRINT_ELEMENT_ZONE:
            break;
        case SPRINT_ELEMENT_TEXT:
            break;
        case SPRINT_ELEMENT_CIRCLE:
            break;
        case SPRINT_ELEMENT_COMPONENT:
            break;
        case SPRINT_ELEMENT_GROUP:
            break;
        default:
            break;
    }

    // Free the entire element
    free(element);

    return element == NULL ? SPRINT_ERROR_ARGUMENT_NULL : SPRINT_ERROR_NONE;
}
