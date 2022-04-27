//
// Created by Benedikt on 27.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_STRINGBUILDER_H
#define SPRINTPCB_STRINGBUILDER_H

#include "errors.h"

#include <stdarg.h>

// Represents a string builder similar to StringBuilder in Java
typedef struct {
    // The number of characters in this builder
    int count;

    // The total capacity of this builder in characters
    int capacity;

    // The pointer to the characters in this builder
    char* content;
} sprint_stringbuilder;

sprint_stringbuilder* sprint_stringbuilder_create(int capacity);

sprint_stringbuilder* sprint_stringbuilder_of(char* content);

sprint_error sprint_stringbuilder_destroy(sprint_stringbuilder* builder);

char* sprint_stringbuilder_complete(sprint_stringbuilder* builder);

sprint_error sprint_stringbuilder_format(sprint_stringbuilder* builder, const char* format, ...);

sprint_error sprint_stringbuilder_putc(sprint_stringbuilder* builder, char chr);

sprint_error sprint_stringbuilder_puts(sprint_stringbuilder* builder, char* str);

sprint_error sprint_stringbuilder_putd(sprint_stringbuilder* builder, int num);

sprint_error sprint_stringbuilder_putb(sprint_stringbuilder* builder, sprint_stringbuilder* source);

char* sprint_stringbuilder_substr(sprint_stringbuilder* builder, int start, int length);

sprint_error sprint_stringbuilder_at(sprint_stringbuilder* builder, int position);

sprint_error sprint_stringbuilder_remove(sprint_stringbuilder* builder, int tail);

sprint_error sprint_stringbuilder_clear(sprint_stringbuilder* builder);

sprint_error sprint_stringbuilder_grow(sprint_stringbuilder* builder, int capacity);

sprint_error sprint_stringbuilder_trim(sprint_stringbuilder* builder);

#endif //SPRINTPCB_STRINGBUILDER_H
