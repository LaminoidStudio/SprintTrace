//
// SprintTrace: dynamic array list
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTTRACE_LIST_H
#define SPRINTTRACE_LIST_H

#include "errors.h"

// Represents a growing list similar to ArrayList in Java
typedef struct sprint_list {
    // The number of elements in this list
    int count;

    // The size of one element in bytes
    int size;

    // The total capacity of this list in elements
    int capacity;

    // The pointer to the elements of this list
    void* elements;
} sprint_list;

sprint_list* sprint_list_create(int width, int capacity);
sprint_error sprint_list_destroy(sprint_list* list);
sprint_error sprint_list_complete(sprint_list* list, int* count, void** elements);
int sprint_list_count(sprint_list* list);
int sprint_list_size(sprint_list* list);
int sprint_list_capacity(sprint_list* list);
sprint_error sprint_list_add(sprint_list* list, void* element);
void* sprint_list_get(sprint_list* list, int index);
sprint_error sprint_list_set(sprint_list* list, int index, void* element);
void* sprint_list_remove(sprint_list* list);
sprint_error sprint_list_clear(sprint_list* list);
sprint_error sprint_list_grow(sprint_list* list, int capacity);
sprint_error sprint_list_trim(sprint_list* list);

#endif //SPRINTTRACE_LIST_H
