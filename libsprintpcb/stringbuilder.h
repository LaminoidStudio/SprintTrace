//
// Created by Benedikt on 27.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_STRINGBUILDER_H
#define SPRINTPCB_STRINGBUILDER_H

#include "errors.h"

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

// Represents a string builder similar to StringBuilder in Java
typedef struct sprint_stringbuilder {
    // The number of characters in this builder
    int count;

    // The total capacity of this builder in characters
    int capacity;

    // The pointer to the characters in this builder
    char* content;
} sprint_stringbuilder;

sprint_stringbuilder* sprint_stringbuilder_create(int capacity);
sprint_stringbuilder* sprint_stringbuilder_of(const char* content);
sprint_error sprint_stringbuilder_destroy(sprint_stringbuilder* builder);
char* sprint_stringbuilder_complete(sprint_stringbuilder* builder);
int sprint_stringbuilder_count(sprint_stringbuilder* builder);
int sprint_stringbuilder_capacity(sprint_stringbuilder* builder);
sprint_error sprint_stringbuilder_flush(sprint_stringbuilder* builder, FILE* stream);
sprint_error sprint_stringbuilder_output(sprint_stringbuilder* builder, char* destination, size_t capacity);
sprint_error sprint_stringbuilder_format(sprint_stringbuilder* builder, const char* format, ...);
sprint_error sprint_stringbuilder_put(sprint_stringbuilder* builder, sprint_stringbuilder* source);
sprint_error sprint_stringbuilder_put_range(sprint_stringbuilder* builder, sprint_stringbuilder* source,
                                            int start, int length);
sprint_error sprint_stringbuilder_put_chr(sprint_stringbuilder* builder, char chr);
sprint_error sprint_stringbuilder_put_str(sprint_stringbuilder* builder, const char* str);
sprint_error sprint_stringbuilder_put_str_range(sprint_stringbuilder* builder, const char* str, int limit);
sprint_error sprint_stringbuilder_put_int(sprint_stringbuilder* builder, int num);
sprint_error sprint_stringbuilder_put_hex(sprint_stringbuilder* builder, int num);
char* sprint_stringbuilder_substr(sprint_stringbuilder* builder, int start, int length);
sprint_error sprint_stringbuilder_at(sprint_stringbuilder* builder, char* result, int position);
sprint_error sprint_stringbuilder_remove_start(sprint_stringbuilder* builder, int length);
sprint_error sprint_stringbuilder_remove_end(sprint_stringbuilder* builder, int length);
sprint_error sprint_stringbuilder_clear(sprint_stringbuilder* builder);
sprint_error sprint_stringbuilder_grow(sprint_stringbuilder* builder, int capacity);
sprint_error sprint_stringbuilder_trim(sprint_stringbuilder* builder);

#endif //SPRINTPCB_STRINGBUILDER_H
