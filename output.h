//
// libsprintpcb: dynamic text output routing
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_OUTPUT_H
#define SPRINTPCB_OUTPUT_H

#include "stringbuilder.h"
#include "errors.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

typedef struct sprint_output sprint_output;

struct sprint_output {
    bool (*write_chr)(sprint_output* output, char chr);
    bool (*write_str)(sprint_output* output, const char* str);
    bool (*write_format)(sprint_output* output, const char* format, va_list args);
    bool (*close)(sprint_output* output, char** contents);
    union {
        sprint_stringbuilder* builder;
        FILE* file;
    };
};

sprint_output* sprint_output_create_str(int capacity);
sprint_output* sprint_output_create_file(FILE* stream, bool close);
sprint_error sprint_output_put_int(sprint_output* output, int val);
sprint_error sprint_output_put_chr(sprint_output* output, char chr);
sprint_error sprint_output_put_str(sprint_output* output, const char* str);
sprint_error sprint_output_format(sprint_output* output, const char* format, ...);
sprint_error sprint_output_destroy(sprint_output* output, char** contents);

#endif //SPRINTPCB_OUTPUT_H
