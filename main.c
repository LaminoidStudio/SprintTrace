//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include <stdio.h>

#include "libsprintpcb/list.h"
#include "libsprintpcb/primitives.h"
#include "libsprintpcb/elements.h"
#include "libsprintpcb/stringbuilder.h"

int gui_main(void);

int main() {
    sprint_element circle = sprint_circle_create(
            SPRINT_LAYER_MECHANICAL,
            10,
            sprint_tuple_of(sprint_dist_um(1), sprint_dist_um(20)),
            10);

    sprint_prim_format format_dist = SPRINT_PRIM_FORMAT_DIST_UM;
    sprint_prim_format format_angle = format_dist == SPRINT_PRIM_FORMAT_RAW ? format_dist : SPRINT_PRIM_FORMAT_COOKED;
    sprint_prim_format format_layer = format_angle;


    sprint_stringbuilder* builder = sprint_stringbuilder_of("Circle and builder test:\n");
    sprint_stringbuilder_put_str(builder, "layer: ");
    sprint_layer_string(circle.circle.layer, builder, format_layer);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "width: ");
    sprint_dist_string(circle.circle.width, builder, format_dist);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "center: ");
    sprint_tuple_string(&circle.circle.center, builder, format_dist);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "radius: ");
    sprint_dist_string(circle.circle.radius, builder, format_dist);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "clear: ");
    sprint_dist_string(circle.circle.clear, builder, format_dist);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "(cutout): ");
    sprint_bool_string(circle.circle.cutout, builder);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "(soldermask): ");
    sprint_bool_string(circle.circle.soldermask, builder);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "(start): ");
    sprint_angle_string(circle.circle.start, builder, format_angle);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "(stop): ");
    sprint_angle_string(circle.circle.stop, builder, format_angle);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_put_str(builder, "(fill): ");
    sprint_bool_string(circle.circle.fill, builder);
    sprint_stringbuilder_put_chr(builder, '\n');
    sprint_element_destroy(&circle);

    sprint_stringbuilder_put_str(builder, "String test: ");
    sprint_str_string("my string raw", builder, SPRINT_PRIM_FORMAT_RAW);
    sprint_stringbuilder_put_chr(builder, ' ');
    sprint_str_string("my string cooked", builder, SPRINT_PRIM_FORMAT_COOKED);
    sprint_stringbuilder_put_chr(builder, '\n');

    sprint_stringbuilder_flush(builder, stdout);

    sprint_tuple tuple1 = sprint_tuple_of(1, 2), tuple2 = sprint_tuple_of(3, 4);

    printf("track %d\n", (int) sizeof(sprint_track));
    printf("pad_tht %d\n", (int) sizeof(sprint_pad_tht));
    printf("pad_smt %d\n", (int) sizeof(sprint_pad_smt));
    printf("zone %d\n", (int) sizeof(sprint_zone));
    printf("text %d\n", (int) sizeof(sprint_text));
    printf("circle %d\n", (int) sizeof(sprint_circle));
    printf("component %d\n", (int) sizeof(sprint_component));
    printf("group %d\n\n", (int) sizeof(sprint_group));
    printf("element %d\n", (int) sizeof(sprint_element));

    sprint_list* list = sprint_list_create(sizeof(sprint_tuple), 0);
    sprint_list_add(list, &tuple1);
    sprint_list_add(list, &tuple2);
    sprint_list_add(list, &tuple2);

    for (int i = 0; i < list->count; i++) {
        sprint_tuple* tuple = (sprint_tuple*)sprint_list_get(list, i);
        sprint_tuple_print(tuple, stdout, SPRINT_PRIM_FORMAT_COOKED);
        putchar('\n');
    }

    sprint_list_destroy(list);

    return gui_main();
}







#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 320

#include "nuklear.h"
#include "nuklear_gdi.h"

static LRESULT CALLBACK
WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    if (nk_gdi_handle_event(wnd, msg, wparam, lparam))
        return 0;

    return DefWindowProcW(wnd, msg, wparam, lparam);
}

char buf[256] = {0};
nk_size mprog = 60;
int mslider = 10;
int mcheck = nk_true;

int gui_main(void)
{
    GdiFont* font;
    struct nk_context *ctx;

    WNDCLASSW wc;
    ATOM atom;
    RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    DWORD style = WS_SYSMENU | WS_BORDER | WS_CAPTION;//WS_OVERLAPPEDWINDOW;
    DWORD exstyle = WS_EX_APPWINDOW;
    HWND wnd;
    HDC dc;
    int running = 1;
    int needs_refresh = 1;

    /* Win32 */
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(0);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"NuklearWindowClass";
    atom = RegisterClassW(&wc);

    AdjustWindowRectEx(&rect, style, FALSE, exstyle);
    wnd = CreateWindowExW(exstyle, wc.lpszClassName, L"Nuklear Demo",
                          style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                          rect.right - rect.left, rect.bottom - rect.top,
                          NULL, NULL, wc.hInstance, NULL);
    dc = GetDC(wnd);

    /* GUI */
    font = nk_gdifont_create("Segoe UI", 14);
    ctx = nk_gdi_init(font, dc, WINDOW_WIDTH, WINDOW_HEIGHT);

    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT]                    = nk_rgb(0x00, 0x00, 0x00);
    table[NK_COLOR_WINDOW]                  = nk_rgb(0xF2, 0xF2, 0xF2);
    table[NK_COLOR_HEADER]                  = nk_rgb(0xCF, 0xD3, 0xD7);
    table[NK_COLOR_BORDER]                  = nk_rgb(0xC4, 0xC4, 0xC4);
    table[NK_COLOR_BUTTON]                  = nk_rgb(0xFF, 0xFF, 0xFF);
    table[NK_COLOR_BUTTON_HOVER]            = nk_rgb(0xF2, 0xF2, 0xF2);
    table[NK_COLOR_BUTTON_ACTIVE]           = nk_rgb(0xE5, 0xE5, 0xE5);
    table[NK_COLOR_TOGGLE]                  = nk_rgb(0xFF, 0xFF, 0xFF);
    table[NK_COLOR_TOGGLE_HOVER]            = nk_rgb(0xC4, 0xC4, 0xC4);
    table[NK_COLOR_TOGGLE_CURSOR]           = nk_rgb(0x52, 0x52, 0x52);
    table[NK_COLOR_SELECT]                  = nk_rgb(0x4F, 0x9E, 0xE3);
    table[NK_COLOR_SELECT_ACTIVE]           = nk_rgb(0x35, 0x75, 0xB0);
    table[NK_COLOR_SLIDER]                  = nk_rgb(0xFF, 0xFF, 0xFF);
    table[NK_COLOR_SLIDER_CURSOR]           = nk_rgb(0x4D, 0x4D, 0x4D);
    table[NK_COLOR_SLIDER_CURSOR_HOVER]     = nk_rgb(0x40, 0x40, 0x40);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE]    = nk_rgb(0x33, 0x33, 0x33);
    table[NK_COLOR_PROPERTY]                = nk_rgb(0xFF, 0xFF, 0xFF);
    table[NK_COLOR_EDIT]                    = nk_rgb(0xFF, 0xFF, 0xFF);
    table[NK_COLOR_EDIT_CURSOR]             = nk_rgb(0x4D, 0x4D, 0x4D);
    table[NK_COLOR_COMBO]                   = nk_rgb(0xFF, 0xFF, 0xFF);
    table[NK_COLOR_CHART]                   = nk_rgb(0xFF, 0xFF, 0xFF);
    table[NK_COLOR_CHART_COLOR]             = nk_rgb(0x4D, 0x4D, 0x4D);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT]   = nk_rgb(0x40, 0x40, 0x40);
    table[NK_COLOR_SCROLLBAR]               = nk_rgb(0xF2, 0xF2, 0xF2);
    table[NK_COLOR_SCROLLBAR_CURSOR]        = nk_rgb(0xC4, 0xC4, 0xC4);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER]  = nk_rgb(0xB8, 0xB8, 0xB8);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgb(0xAB, 0xAB, 0xAB);
    table[NK_COLOR_TAB_HEADER]              = nk_rgb(0xCF, 0xD3, 0xD7);
    nk_style_from_table(ctx, table);

    while (running)
    {
        /* Input */
        MSG msg;
        nk_input_begin(ctx);
        if (needs_refresh == 0) {
            if (GetMessageW(&msg, NULL, 0, 0) <= 0)
                running = 0;
            else {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            needs_refresh = 1;
        } else needs_refresh = 0;

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                running = 0;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            needs_refresh = 1;
        }
        nk_input_end(ctx);

        /* GUI */
        if (nk_begin(ctx, "Demo", nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT),
                     //0))
                     //NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                     NK_WINDOW_TITLE))
                     //NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property1 = 20;
            static int property2 = 20;

            nk_layout_row_static(ctx, 30, 80, 2);
            if (nk_button_label(ctx, "button1"))
                fprintf(stdout, "button1 pressed\n");
            if (nk_button_label(ctx, "button2"))
                fprintf(stdout, "button2 pressed\n");
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(ctx, 22, 2);
            nk_property_int(ctx, "Compression:", 0, &property1, 100, 10, 1);
            nk_property_int(ctx, "Other:", 0, &property2, 100, 10, 1);
            nk_layout_row_dynamic(ctx, 22, 2);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, buf, sizeof(buf) - 1, nk_filter_default);
            if (nk_button_label(ctx, "Done"))
                printf("%s\n", buf);

            nk_layout_row_dynamic(ctx, 22, 3);
            nk_progress(ctx, &mprog, 100, NK_MODIFIABLE);
            nk_slider_int(ctx, 0, &mslider, 16, 1);
            nk_checkbox_label(ctx, "check", &mcheck);

            const float values[]={26.0f,13.0f,30.0f,15.0f,25.0f,10.0f,20.0f,40.0f,12.0f,8.0f,22.0f,28.0f};
            nk_layout_row_dynamic(ctx, 150, 1);
            nk_chart_begin(ctx, NK_CHART_COLUMN, NK_LEN(values), 0, 50);
            for (size_t i = 0; i < NK_LEN(values); ++i)
                nk_chart_push(ctx, values[i]);
            nk_chart_end(ctx);
        }
        nk_end(ctx);

        /* Draw */
        nk_gdi_render(nk_rgb(240,240,240));
    }

    nk_gdifont_del(font);
    ReleaseDC(wnd, dc);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}
