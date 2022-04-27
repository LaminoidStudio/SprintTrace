//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef LIBSPRINTPCB_ERRORS_H
#define LIBSPRINTPCB_ERRORS_H

typedef enum {
    SPRINT_ERROR_NONE,
    SPRINT_ERROR_ASSERTION,
    SPRINT_ERROR_UNDERFLOW,
    SPRINT_ERROR_OVERFLOW,
    SPRINT_ERROR_IO,
    SPRINT_ERROR_STATE_INVALID,
    SPRINT_ERROR_ARGUMENT_NULL,
    SPRINT_ERROR_ARGUMENT_RANGE,
    SPRINT_ERROR_ARGUMENT_FORMAT,
    SPRINT_ERROR_NOT_ASCII,
    SPRINT_ERROR_SYNTAX// fixme
} sprint_error;

#endif //LIBSPRINTPCB_ERRORS_H
