//
// Created by Benedikt on 27.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "stringbuilder.h"
#include "errors.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

sprint_stringbuilder* sprint_stringbuilder_create(int capacity)
{
    if (capacity < 0) return NULL;

    sprint_stringbuilder* builder = calloc(1, sizeof(*builder));
    builder->capacity = capacity;
    builder->content = malloc(capacity * sizeof(char) + 1);

    return builder;
}

sprint_stringbuilder* sprint_stringbuilder_of(char* content)
{
    if (content == NULL) return NULL;

    sprint_stringbuilder* builder = calloc(1, sizeof(*builder));
    builder->count = (int) strlen(content);
    builder->capacity = builder->count * 2;
    builder->content = malloc(builder->capacity * sizeof(char) + 1);

    // Copy the string (strcpy is safe, because the string length is known)
    strcpy(builder->content, content);
    return builder;
}

sprint_error sprint_stringbuilder_destroy(sprint_stringbuilder* builder)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    builder->count = 0;
    builder->capacity = 0;

    if (builder->content != NULL) {
        free(builder->content);
        builder->content = NULL;
    }

    return SPRINT_ERROR_NONE;
}

char* sprint_stringbuilder_complete(sprint_stringbuilder* builder)
{
    if (sprint_stringbuilder_trim(builder) != SPRINT_ERROR_NONE) {
        free(builder);
        return NULL;
    }

    int count = builder->count;
    builder->count = 0;
    builder->capacity = 0;

    char* result = builder->content;
    if (builder->content != NULL)
        builder->content[count] = 0;

    free(builder);
    return result;
}

sprint_error sprint_stringbuilder_format(sprint_stringbuilder* builder, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int additional_length = vsnprintf(NULL, 0, format, args);
    if (additional_length < 0) {
        va_end(args);
        return SPRINT_ERROR_ARGUMENT_FORMAT;
    }

    int minimum_capacity = builder->count + additional_length;
    if (minimum_capacity >= builder->capacity)
    {
        sprint_error error = sprint_stringbuilder_grow(builder, builder->capacity * 2 + minimum_capacity);
        if (error != SPRINT_ERROR_NONE) {
            va_end(args);
            return error;
        }
    }

    int written_length = vsnprintf(builder->content + builder->count, builder->capacity + 1, format, args);
    va_end(args);
    if (written_length != additional_length)
        return SPRINT_ERROR_ASSERTION;

    builder->count += written_length;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_stringbuilder_putc(sprint_stringbuilder* builder, char chr)
{

}

sprint_error sprint_stringbuilder_puts(sprint_stringbuilder* builder, char* str)
{

}

sprint_error sprint_stringbuilder_putd(sprint_stringbuilder* builder, int num)
{

}

sprint_error sprint_stringbuilder_putb(sprint_stringbuilder* builder, sprint_stringbuilder* source)
{

}

char* sprint_stringbuilder_substr(sprint_stringbuilder* builder, int start, int length)
{

}

sprint_error sprint_stringbuilder_at(sprint_stringbuilder* builder, int position)
{

}

sprint_error sprint_stringbuilder_remove(sprint_stringbuilder* builder, int tail)
{

}

sprint_error sprint_stringbuilder_clear(sprint_stringbuilder* builder)
{

}

sprint_error sprint_stringbuilder_grow(sprint_stringbuilder* builder, int capacity)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (capacity < 1) return SPRINT_ERROR_ARGUMENT_RANGE;

    // If the content is uninitialized, alloc an array with the desired capacity
    if (builder->content == NULL) {
        builder->count = 0;
        builder->capacity = capacity;
        builder->content = malloc(capacity * sizeof(char) + 1);
        return builder->content == NULL ? SPRINT_ERROR_OVERFLOW : SPRINT_ERROR_NONE;
    }

    // If the builder is already big enough, do nothing
    if (builder->capacity >= capacity) return SPRINT_ERROR_NONE;

    // Grow the builder to the new capacity
    void* new_content = realloc(builder->content, capacity * sizeof(char) + 1);
    if (new_content == NULL) return SPRINT_ERROR_OVERFLOW;

    // Update the capacity and content
    builder->capacity = capacity;
    builder->content = new_content;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_stringbuilder_trim(sprint_stringbuilder* builder)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // If the content is uninitialized, alloc an array with a single character
    if (builder->content == NULL) {
        builder->count = 0;
        builder->capacity = 0;
        builder->content = malloc(sizeof(char));
        return builder->content == NULL ? SPRINT_ERROR_OVERFLOW : SPRINT_ERROR_NONE;
    }

    // Only do something, if the count doesn't already match the capacity
    if (builder->count == builder->capacity) return SPRINT_ERROR_NONE;

    // Shrink the builder to the count
    char* new_content = realloc(builder->content, builder->count * sizeof(char) + 1);
    if (new_content == NULL) return SPRINT_ERROR_OVERFLOW;

    // Update the capacity and content
    builder->capacity = builder->count;
    builder->content = new_content;
    return SPRINT_ERROR_NONE;
}
