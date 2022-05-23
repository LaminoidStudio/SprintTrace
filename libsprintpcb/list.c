//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "list.h"
#include "errors.h"

#include <stdlib.h>
#include <string.h>

sprint_list* sprint_list_create(int size, int capacity)
{
    if (size < 1 || capacity < 0) return NULL;

    sprint_list* list = calloc(1, sizeof(*list));
    list->size = size;
    list->capacity = capacity;
    list->elements = capacity > 0 ? malloc(capacity * size) : NULL;

    return list;
}

sprint_error sprint_list_destroy(sprint_list* list)
{
    if (list == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    list->count = 0;
    list->capacity = 0;

    if (list->elements != NULL)
        free(list->elements);

    free(list);
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_list_complete(sprint_list* list, int* count, void** elements)
{
    if (list == NULL || count == NULL || elements == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    sprint_error error = SPRINT_ERROR_NONE;
    if (!sprint_chain(error, sprint_list_trim(list))) {
        free(list);
        return sprint_rethrow(error);
    }

    // Store the elements and count
    *elements = list->elements;
    *count = list->count;

    // Zero the state
    list->count = 0;
    list->size = 0;
    list->capacity = 0;

    // And finally, free the list
    free(list);

    return SPRINT_ERROR_NONE;
}

int sprint_list_count(sprint_list* list)
{
    return list == NULL ? 0 : list->count;
}

int sprint_list_size(sprint_list* list)
{
    return list == NULL ? 0 : list->size;
}

int sprint_list_capacity(sprint_list* list)
{
    return list == NULL ? 0 : list->capacity;
}

sprint_error sprint_list_add(sprint_list* list, void* element)
{
    if (list == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Start with an initial capacity of one
    if (list->capacity < 1) list->capacity = 1;

    // If the list is not initialized or the capacity is insufficient, grow the list
    if (list->elements == NULL || list->count + 1 >= list->capacity) {
        // When growing the list, double its count every time it has to be expanded
        int new_capacity = list->capacity * 2;
        if (new_capacity < list->capacity) return SPRINT_ERROR_OVERFLOW;
        sprint_error error = sprint_list_grow(list, new_capacity);
        if (error != SPRINT_ERROR_NONE) return sprint_rethrow(error);
    }

    // Copy the new element to the end of the list and increment the count
    memcpy(list->elements + list->size * list->count, element, list->size);
    list->count++;
    return SPRINT_ERROR_NONE;
}

void* sprint_list_get(sprint_list* list, int index)
{
    if (list == NULL || index < 0 || index >= list->count) return NULL;
    return list->elements + list->size * index;
}

sprint_error sprint_list_set(sprint_list* list, int index, void* element)
{
    if (list == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (index < 0 || index >= list->count) return SPRINT_ERROR_ARGUMENT_RANGE;

    memcpy(list->elements + list->size * index, element, list->size);
    return SPRINT_ERROR_NONE;
}

void* sprint_list_remove(sprint_list* list)
{
    if (list == NULL || list->count < 1) return NULL;

    list->count--;
    return list->elements + list->size * list->count;
}

sprint_error sprint_list_clear(sprint_list* list)
{
    if (list == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    list->size = 0;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_list_grow(sprint_list* list, int capacity)
{
    if (list == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (capacity < 1) return SPRINT_ERROR_ARGUMENT_RANGE;

    // If the list is uninitialized, initialize it
    if (list->elements == NULL) {
        list->elements = malloc(list->size * capacity);
        if (list->elements == NULL) return SPRINT_ERROR_MEMORY;

        list->capacity = capacity;
        return SPRINT_ERROR_NONE;
    }

    // If the list is already big enough, do nothing
    if (list->capacity >= capacity) return SPRINT_ERROR_NONE;

    // Grow the list to the new count
    void* new_elements = realloc(list->elements, list->size * capacity);
    if (new_elements == NULL) return SPRINT_ERROR_MEMORY;

    // Update the capacity and elements
    list->capacity = capacity;
    list->elements = new_elements;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_list_trim(sprint_list* list)
{
    if (list == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // If there are no elements, trim the list to zero
    if (list->elements == NULL) {
        list->count = 0;
        list->capacity = 0;
        return SPRINT_ERROR_NONE;
    }

    // Only do something, if the count doesn't already match the capacity
    if (list->count == list->capacity) return SPRINT_ERROR_NONE;

    // Shrink the list to the count
    void* new_elements = realloc(list->elements, list->size * list->count);
    if (new_elements == NULL) return SPRINT_ERROR_MEMORY;

    // Update the capacity and elements
    list->capacity = list->count;
    list->elements = new_elements;
    return SPRINT_ERROR_NONE;
}
