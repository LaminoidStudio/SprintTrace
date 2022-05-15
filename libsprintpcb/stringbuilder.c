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
#include <stdbool.h>

#ifndef WIN32
#include <limits.h>
#endif

sprint_stringbuilder* sprint_stringbuilder_create(int capacity)
{
    if (capacity < 0) return NULL;

    // Allocate the string builder
    sprint_stringbuilder* builder = calloc(1, sizeof(*builder));
    builder->capacity = capacity;
    builder->content = malloc(capacity * sizeof(char) + 1);

    return builder;
}

sprint_stringbuilder* sprint_stringbuilder_of(const char* content)
{
    if (content == NULL) return NULL;

    // Allocate the string builder and copy the content
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

    // If required, free the content
    if (builder->content != NULL) {
        free(builder->content);
        builder->content = NULL;
    }

    // And finally, free the builder
    free(builder);
    return SPRINT_ERROR_NONE;
}

char* sprint_stringbuilder_complete(sprint_stringbuilder* builder)
{
    if (sprint_stringbuilder_trim(builder) != SPRINT_ERROR_NONE) {
        if (builder != NULL)
            free(builder);
        return NULL;
    }

    int count = builder->count;
    builder->count = 0;
    builder->capacity = 0;

    // If required, free the content
    char* result = builder->content;
    if (builder->content != NULL)
        builder->content[count] = 0;

    // And finally, free the builder
    free(builder);
    return result;
}

sprint_error sprint_stringbuilder_flush(sprint_stringbuilder* builder, FILE* stream)
{
    if (builder == NULL || stream == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    char* contents = sprint_stringbuilder_complete(builder);
    if (contents == NULL) return SPRINT_ERROR_STATE_INVALID;

    // Try to write the contents of the builder to the stream and discard the contents
    bool success = fputs(contents, stream) >= 0;
    free(contents);
    return success ? SPRINT_ERROR_NONE : SPRINT_ERROR_IO;
}

sprint_error sprint_stringbuilder_output(sprint_stringbuilder* builder, char* destination, size_t capacity)
{
    if (builder == NULL || destination == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (capacity < builder->count + 1) return SPRINT_ERROR_OVERFLOW;

    memcpy(destination, builder->content, builder->count * sizeof(char));
    destination[builder->count] = 0;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_stringbuilder_format(sprint_stringbuilder* builder, const char* format, ...)
{
    if (builder == NULL || format == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Determine the required buffer space
    va_list args;
    va_start(args, format);
    int additional_length = vsnprintf(NULL, 0, format, args);
    va_end(args);
    if (additional_length < 0)
        return SPRINT_ERROR_ARGUMENT_FORMAT;

    // Grow the buffer to fit the new content
    int minimum_capacity = builder->count + additional_length;
    if (builder->content == NULL || minimum_capacity >= builder->capacity)
    {
        sprint_error error = sprint_stringbuilder_grow(builder, builder->capacity * 2 + minimum_capacity);
        if (error != SPRINT_ERROR_NONE)
            return sprint_rethrow(error);
    }

    // Actually write the formatted content
    va_start(args, format);
    int written_length = vsnprintf(builder->content + builder->count, builder->capacity + 1, format, args);
    va_end(args);
    if (written_length != additional_length)
        return SPRINT_ERROR_ASSERTION;

    // Increase the buffer usage
    builder->count += written_length;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_stringbuilder_put(sprint_stringbuilder* builder, sprint_stringbuilder* source)
{
    if (source == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    return sprint_stringbuilder_put_range(builder, source, 0, source->count);
}

sprint_error sprint_stringbuilder_put_range(sprint_stringbuilder* builder, sprint_stringbuilder* source,
                                            int start, int length)
{
    if (builder == NULL || source == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (start < 0 || length < 0 || start + length > source->count) return SPRINT_ERROR_ARGUMENT_RANGE;
    if (source->content == NULL) return SPRINT_ERROR_STATE_INVALID;

    // Ensure that there is room to put the string
    int minimum_capacity = builder->count + source->count;
    if (builder->content == NULL || minimum_capacity >= builder->capacity)
    {
        sprint_error error = sprint_stringbuilder_grow(builder, builder->capacity * 2 + minimum_capacity);
        if (error != SPRINT_ERROR_NONE)
            return sprint_rethrow(error);
    }

    // Copy the contents of the source to the builder and increase the size
    memmove(builder->content + builder->count, source->content + start, length * sizeof(char));
    builder->count += source->count;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_stringbuilder_put_chr(sprint_stringbuilder* builder, char chr)
{
    if (builder == NULL || chr == 0) return SPRINT_ERROR_ARGUMENT_NULL;

    // Ensure that there is room to put the character
    if (builder->content == NULL || builder->count + 1 >= builder->capacity)
    {
        sprint_error error = sprint_stringbuilder_grow(builder, builder->capacity * 2);
        if (error != SPRINT_ERROR_NONE)
            return sprint_rethrow(error);
    }

    // Store the character
    builder->content[builder->count] = chr;
    builder->count++;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_stringbuilder_put_str(sprint_stringbuilder* builder, const char* str)
{
    return sprint_stringbuilder_put_str_range(builder, str, INT_MAX);
}

sprint_error sprint_stringbuilder_put_str_range(sprint_stringbuilder* builder, const char* str, int limit)
{
    if (builder == NULL || str == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (limit < 0) return SPRINT_ERROR_ARGUMENT_RANGE;
    if (builder->content == NULL) return SPRINT_ERROR_STATE_INVALID;

    // Ensure that there is room to put the string
    int additional_length = (int) strlen(str);
    if (additional_length > limit)
        additional_length = limit;
    int minimum_capacity = builder->count + additional_length;
    if (builder->content == NULL || minimum_capacity >= builder->capacity)
    {
        sprint_error error = sprint_stringbuilder_grow(builder, builder->capacity * 2 + minimum_capacity);
        if (error != SPRINT_ERROR_NONE)
            return sprint_rethrow(error);
    }

    // Copy the string
    strncpy(builder->content + builder->count, str, additional_length);
    builder->count += additional_length;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_stringbuilder_put_int(sprint_stringbuilder* builder, int num)
{
    // Using the formatter is the safest option to print a decimal integer
    return sprint_stringbuilder_format(builder, "%d", num);
}

sprint_error sprint_stringbuilder_put_hex(sprint_stringbuilder* builder, int num)
{
    // Using the formatter is the safest option to print an hexadecimal integer
    return sprint_stringbuilder_format(builder, "%x", num);
}

sprint_error sprint_stringbuilder_at(sprint_stringbuilder* builder, char* result, int position)
{
    if (builder == NULL || result == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (position < 0 || position >= builder->count) return SPRINT_ERROR_ARGUMENT_RANGE;
    if (builder->content == NULL) return SPRINT_ERROR_STATE_INVALID;

    // Simply return the character at the position
    *result = builder->content[position];
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_stringbuilder_remove_start(sprint_stringbuilder* builder, int length)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (length < 0 || length > builder->count) return SPRINT_ERROR_ARGUMENT_RANGE;
    if (builder->content == NULL) return SPRINT_ERROR_STATE_INVALID;
    if (length == 0) return SPRINT_ERROR_NONE;

    // Copy the characters back and subtract from the character count
    memmove(builder->content, builder->content + length, (builder->count - length) * sizeof(char));
    builder->count -= length;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_stringbuilder_remove_end(sprint_stringbuilder* builder, int length)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (length < 0 || length > builder->count) return SPRINT_ERROR_ARGUMENT_RANGE;
    if (length == 0) return SPRINT_ERROR_NONE;

    // Just subtract the length to trim from the character count
    builder->count -= length;
    return SPRINT_ERROR_NONE;
}

sprint_error sprint_stringbuilder_clear(sprint_stringbuilder* builder)
{
    if (builder == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    // Just reset the character count
    builder->count = 0;
    return SPRINT_ERROR_NONE;
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
        return builder->content == NULL ? SPRINT_ERROR_MEMORY : SPRINT_ERROR_NONE;
    }

    // If the builder is already big enough, do nothing
    if (builder->capacity >= capacity) return SPRINT_ERROR_NONE;

    // Grow the builder to the new capacity
    void* new_content = realloc(builder->content, capacity * sizeof(char) + 1);
    if (new_content == NULL) return SPRINT_ERROR_MEMORY;

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
        return builder->content == NULL ? SPRINT_ERROR_MEMORY : SPRINT_ERROR_NONE;
    }

    // Only do something, if the count doesn't already match the capacity
    if (builder->count == builder->capacity) return SPRINT_ERROR_NONE;

    // Shrink the builder to the count
    char* new_content = realloc(builder->content, builder->count * sizeof(char) + 1);
    if (new_content == NULL) return SPRINT_ERROR_MEMORY;

    // Update the capacity and content
    builder->capacity = builder->count;
    builder->content = new_content;
    return SPRINT_ERROR_NONE;
}
