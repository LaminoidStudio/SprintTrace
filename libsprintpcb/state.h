//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#ifndef SPRINTPCB_STATE_H
#define SPRINTPCB_STATE_H

typedef enum {
    l
} sprint_callback___;

typedef struct {
    int (*on_track)();
    int (*on_thtpad)();
    int (*on_smtpad)();
    int (*on_zone)();
    int (*on_text)();
    int (*on_circle)();
} sprint_state;

#endif //SPRINTPCB_STATE_H
