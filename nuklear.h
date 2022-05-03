//
// Created by Benedikt on 30.04.2022.
//

#ifndef SPRINTPCB_NUKLEAR_H
#define SPRINTPCB_NUKLEAR_H

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_ZERO_COMMAND_MEMORY

#ifndef WIN32 // TODO: change me to SDL
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#endif

#define NK_MEMSET memset
#define NK_MEMCPY memcpy
#define NK_SQRT sqrt
#define NK_SIN sin
#define NK_COS cos

#include "Nuklear/nuklear.h"

#endif //SPRINTPCB_NUKLEAR_H
