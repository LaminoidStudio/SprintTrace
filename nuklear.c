//
// Created by Benedikt on 30.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

// Nuklear GUI (https://github.com/Immediate-Mode-UI/Nuklear) used is released 'public domain'

#define NK_IMPLEMENTATION
#include "nuklear.h"

float nk_sin(float x)
{
    NK_STORAGE const float a0 = +1.91059300966915117e-31f;
    NK_STORAGE const float a1 = +1.00086760103908896f;
    NK_STORAGE const float a2 = -1.21276126894734565e-2f;
    NK_STORAGE const float a3 = -1.38078780785773762e-1f;
    NK_STORAGE const float a4 = -2.67353392911981221e-2f;
    NK_STORAGE const float a5 = +2.08026600266304389e-2f;
    NK_STORAGE const float a6 = -3.03996055049204407e-3f;
    NK_STORAGE const float a7 = +1.38235642404333740e-4f;
    return a0 + x*(a1 + x*(a2 + x*(a3 + x*(a4 + x*(a5 + x*(a6 + x*a7))))));
}

float nk_cos(float x)
{
    /* New implementation. Also generated using lolremez. */
    /* Old version significantly deviated from expected results. */
    NK_STORAGE const float a0 = 9.9995999154986614e-1f;
    NK_STORAGE const float a1 = 1.2548995793001028e-3f;
    NK_STORAGE const float a2 = -5.0648546280678015e-1f;
    NK_STORAGE const float a3 = 1.2942246466519995e-2f;
    NK_STORAGE const float a4 = 2.8668384702547972e-2f;
    NK_STORAGE const float a5 = 7.3726485210586547e-3f;
    NK_STORAGE const float a6 = -3.8510875386947414e-3f;
    NK_STORAGE const float a7 = 4.7196604604366623e-4f;
    NK_STORAGE const float a8 = -1.8776444013090451e-5f;
    return a0 + x*(a1 + x*(a2 + x*(a3 + x*(a4 + x*(a5 + x*(a6 + x*(a7 + x*a8)))))));
}
