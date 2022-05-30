//
// Created by Benedikt on 17.05.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include "output.h"
#include "stringbuilder.h"
#include "errors.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>

static bool sprint_output_write_chr_str_internal(sprint_output* output, char chr);
static bool sprint_output_write_chr_file_internal(sprint_output* output, char chr);
static bool sprint_output_write_str_str_internal(sprint_output* output, const char* str);
static bool sprint_output_write_str_file_internal(sprint_output* output, const char* str);
static bool sprint_output_write_format_str_internal(sprint_output* output, const char* format, va_list args);
static bool sprint_output_write_format_file_internal(sprint_output* output, const char* format, va_list args);
static bool sprint_output_close_str_internal(sprint_output* output, char** contents);
static bool sprint_output_close_file_internal(sprint_output* output, __attribute__((unused)) char** contents);

sprint_output* sprint_output_create_str(int capacity)
{
    // Construct a new string builder
    sprint_stringbuilder* builder = sprint_stringbuilder_create(capacity);
    if (builder == NULL)
        return NULL;

    // Allocate the memory
    sprint_output* output = calloc(1, sizeof(*output));
    if (output == NULL)
        return NULL;

    // Store the builder and the functions
    output->builder = builder;
    output->write_chr = sprint_output_write_chr_str_internal;
    output->write_str = sprint_output_write_str_str_internal;
    output->write_format = sprint_output_write_format_str_internal;
    output->close = sprint_output_close_str_internal;

    return output;
}

sprint_output* sprint_output_create_file(FILE* stream, bool close)
{
    if (stream == NULL) return NULL;

    // Allocate the memory
    sprint_output* output = calloc(1, sizeof(*output));
    if (output == NULL)
        return NULL;

    // Store the stream and the functions
    output->file = stream;
    output->write_chr = sprint_output_write_chr_file_internal;
    output->write_str = sprint_output_write_str_file_internal;
    output->write_format = sprint_output_write_format_file_internal;
    output->close = close ? sprint_output_close_file_internal : NULL;

    return output;
}

sprint_error sprint_output_put_int(sprint_output* output, int val)
{
    return sprint_output_format(output, "%d", val);
}

sprint_error sprint_output_put_chr(sprint_output* output, char chr)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (output->write_chr == NULL) return SPRINT_ERROR_STATE_INVALID;

    return output->write_chr(output, chr) ? SPRINT_ERROR_NONE : SPRINT_ERROR_IO;
}

sprint_error sprint_output_put_str(sprint_output* output, const char* str)
{
    if (output == NULL || str == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (output->write_str == NULL) return SPRINT_ERROR_STATE_INVALID;

    return output->write_str(output, str) ? SPRINT_ERROR_NONE : SPRINT_ERROR_IO;
}

sprint_error sprint_output_format(sprint_output* output, const char* format, ...)
{
    if (output == NULL || format == NULL) return SPRINT_ERROR_ARGUMENT_NULL;
    if (output->write_format == NULL) return SPRINT_ERROR_STATE_INVALID;

    va_list args;
    va_start(args, format);
    bool success = output->write_format(output, format, args);
    va_end(args);
    return success ? SPRINT_ERROR_NONE : SPRINT_ERROR_IO;
}

sprint_error sprint_output_destroy(sprint_output* output, char** contents)
{
    if (output == NULL) return SPRINT_ERROR_ARGUMENT_NULL;

    bool success = output->close == NULL ? true : output->close(output, contents);
    memset(output, 0, sizeof(*output));
    free(output);
    return success ? SPRINT_ERROR_NONE : SPRINT_ERROR_IO;
}

bool sprint_output_write_chr_str_internal(sprint_output* output, char chr)
{
    return sprint_check(sprint_stringbuilder_put_chr(output->builder, chr));
}

bool sprint_output_write_chr_file_internal(sprint_output* output, char chr)
{
    errno = 0;
    if (fputc(chr, output->file) != EOF)
        return true;

    sprint_throw_format(false, "error writing file: %s", strerror(errno));
    return false;
}

bool sprint_output_write_str_str_internal(sprint_output* output, const char* str)
{
    return sprint_check(sprint_stringbuilder_put_str(output->builder, str));
}

bool sprint_output_write_str_file_internal(sprint_output* output, const char* str)
{
    errno = 0;
    if (fputs(str, output->file) != EOF)
        return true;

    sprint_throw_format(false, "error writing file: %s", strerror(errno));
    return false;
}

bool sprint_output_write_format_str_internal(sprint_output* output, const char* format, va_list args)
{
    return sprint_check(sprint_stringbuilder_format_args(output->builder, format, args));
}

bool sprint_output_write_format_file_internal(sprint_output* output, const char* format, va_list args)
{
    errno = 0;
    if (vfprintf(output->file, format, args) >= 0)
        return true;

    sprint_throw_format(false, "error writing file: %s", strerror(errno));
    return false;
}

bool sprint_output_close_str_internal(sprint_output* output, char** contents)
{
    if (output->builder == NULL) return false;

    bool success;
    if (contents != NULL) {
        *contents = sprint_stringbuilder_complete(output->builder);
        success = sprint_assert(false, *contents != NULL);
    } else
        success = sprint_check(sprint_stringbuilder_destroy(output->builder));

    output->builder = NULL;
    return success;
}

bool sprint_output_close_file_internal(sprint_output* output, __attribute__((unused)) char** contents)
{
    if (output->file == NULL) return false;

    bool success = fclose(output->file) != EOF;
    output->file = NULL;

    return success;
}
