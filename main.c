//
// Created by Benedikt on 26.04.2022.
// Copyright 2022, Laminoid.com (Muessig & Muessig GbR).
// Licensed under the terms and conditions of the GPLv3.
//

#include <stdio.h>
#include <string.h>

#include "libsprintpcb/list.h"
#include "libsprintpcb/primitives.h"
#include "libsprintpcb/elements.h"
#include "libsprintpcb/stringbuilder.h"
#include "libsprintpcb/output.h"
#include "libsprintpcb/errors.h"
#include "libsprintpcb/plugin.h"

int gui_main(void);

int main(int argc, const char* argv[]) {
    sprint_require(sprint_plugin_begin(argc, argv));
    return gui_main();
}


#include "nuklear.h"

struct demo_gui {
    int width;
    int height;
    char* title;
    struct nk_colorf bg;
    char buf[256];
    char buf2[256];
    nk_size mprog;
    int mslider;
    int mcheck;
    char* plugin;
};

void init_gui(struct demo_gui* gui)
{
    memset(gui, 0, sizeof(*gui));
    gui->width = 640;
    gui->height = 640;
    gui->title = "Demo";
    gui->bg = nk_color_cf(nk_rgb_f(0.10f, 0.18f, 0.24f));
    gui->mprog = 60;
    gui->mslider = 10;
    gui->mcheck = nk_true;
    strcpy(gui->buf2, "Hello, world!");

    sprint_output* buffer = sprint_output_create_str(63);
    sprint_assert(true, buffer != NULL);
    sprint_require(sprint_plugin_output(buffer));
    sprint_require(sprint_output_destroy(buffer, &gui->plugin));
}

void init_ctx(struct nk_context* ctx)
{
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
}

struct nk_canvas {
    struct nk_command_buffer *painter;
    struct nk_vec2 item_spacing;
    struct nk_vec2 panel_padding;
    struct nk_style_item window_background;
};

static nk_bool
canvas_begin(struct nk_context *ctx, struct nk_canvas *canvas, nk_flags flags,
             int x, int y, int width, int height, struct nk_color background_color)
{
    /* save style properties which will be overwritten */
    canvas->panel_padding = ctx->style.window.padding;
    canvas->item_spacing = ctx->style.window.spacing;
    canvas->window_background = ctx->style.window.fixed_background;

    /* use the complete window space and set background */
    ctx->style.window.spacing = nk_vec2(0,0);
    ctx->style.window.padding = nk_vec2(0,0);
    ctx->style.window.fixed_background = nk_style_item_color(background_color);

    /* create/update window and set position + size */
    if (!nk_begin(ctx, "Canvas", nk_rect(x, y, width, height), NK_WINDOW_NO_SCROLLBAR|flags))
        return nk_false;

    /* allocate the complete window space for drawing */
    {
        struct nk_rect total_space;
        total_space = nk_window_get_content_region(ctx);
        nk_layout_row_dynamic(ctx, total_space.h, 1);
        nk_widget(&total_space, ctx);
        canvas->painter = nk_window_get_canvas(ctx);
    }

    return nk_true;
}

static void
canvas_end(struct nk_context *ctx, struct nk_canvas *canvas)
{
    nk_end(ctx);
    ctx->style.window.spacing = canvas->panel_padding;
    ctx->style.window.padding = canvas->item_spacing;
    ctx->style.window.fixed_background = canvas->window_background;
}

static void
canvas(struct nk_context *ctx)
{
    struct nk_canvas canvas;
    if (canvas_begin(ctx, &canvas, NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                                   NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE, 10, 10, 500, 550, nk_rgb(250,250,250)))
    {
        float x = canvas.painter->clip.x, y = canvas.painter->clip.y;

        nk_fill_rect(canvas.painter, nk_rect(x + 15, y + 15, 210, 210), 5, nk_rgb(247, 230, 154));
        nk_fill_rect(canvas.painter, nk_rect(x + 20, y + 20, 200, 200), 5, nk_rgb(188, 174, 118));
        /* nk_draw_text(canvas.painter, nk_rect(x + 30, y + 30, 150, 20), "Text to draw", 12, &font->handle, nk_rgb(188,174,118), nk_rgb(0,0,0)); */
        nk_fill_rect(canvas.painter, nk_rect(x + 250, y + 20, 100, 100), 0, nk_rgb(0,0,255));
        nk_fill_circle(canvas.painter, nk_rect(x + 20, y + 250, 100, 100), nk_rgb(255,0,0));
        nk_fill_triangle(canvas.painter, x + 250, y + 250, x + 350, y + 250, x + 300, y + 350, nk_rgb(0,255,0));
        nk_fill_arc(canvas.painter, x + 300, y + 420, 50, 0, 3.141592654f * 3.0f / 4.0f, nk_rgb(255,255,0));

        {
            float points[12];
            points[0]  = x + 200; points[1]  = y + 250;
            points[2]  = x + 250; points[3]  = y + 350;
            points[4]  = x + 225; points[5]  = y + 350;
            points[6]  = x + 200; points[7]  = y + 300;
            points[8]  = x + 175; points[9]  = y + 350;
            points[10] = x + 150; points[11] = y + 350;
            nk_fill_polygon(canvas.painter, points, 6, nk_rgb(0,0,0));
        }

        {
            float points[12];
            points[0]  = x + 200; points[1]  = y + 370;
            points[2]  = x + 250; points[3]  = y + 470;
            points[4]  = x + 225; points[5]  = y + 470;
            points[6]  = x + 200; points[7]  = y + 420;
            points[8]  = x + 175; points[9]  = y + 470;
            points[10] = x + 150; points[11] = y + 470;
            nk_stroke_polygon(canvas.painter, points, 6, 4, nk_rgb(0,0,0));
        }

        {
            float points[8];
            points[0]  = x + 250; points[1]  = y + 200;
            points[2]  = x + 275; points[3]  = y + 220;
            points[4]  = x + 325; points[5]  = y + 170;
            points[6]  = x + 350; points[7]  = y + 200;
            nk_stroke_polyline(canvas.painter, points, 4, 2, nk_rgb(255,128,0));
        }

        nk_stroke_line(canvas.painter, x + 15, y + 10, x + 200, y + 10, 2.0f, nk_rgb(189,45,75));
        nk_stroke_rect(canvas.painter, nk_rect(x + 370, y + 20, 100, 100), 10, 3, nk_rgb(0,0,255));
        nk_stroke_curve(canvas.painter, x + 380, y + 200, x + 405, y + 270, x + 455, y + 120, x + 480, y + 200, 2, nk_rgb(0,150,220));
        nk_stroke_circle(canvas.painter, nk_rect(x + 20, y + 370, 100, 100), 5, nk_rgb(0,255,120));
        nk_stroke_triangle(canvas.painter, x + 370, y + 250, x + 470, y + 250, x + 420, y + 350, 6, nk_rgb(255,0,143));
        nk_stroke_arc(canvas.painter, x + 420, y + 420, 50, 0, 3.141592654f * 3.0f / 4.0f, 5, nk_rgb(0,255,255));
    }
    canvas_end(ctx, &canvas);
}

sprint_error sprint_plugin_gui(struct nk_context* ctx);
void handle_gui(struct nk_context* ctx, struct demo_gui* gui)
{
    /* GUI */
    if (nk_begin(ctx, gui->title, nk_rect(0, 0, gui->width, gui->height),
            0))
        // NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
        // NK_WINDOW_TITLE))
        // NK_WINDOW_TITLE|NK_WINDOW_MOVABLE|NK_WINDOW_BORDER))
        // NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        enum {EASY, HARD};
        static int op = EASY;
        static int property1 = 20;
        static int property2 = 20;

        nk_layout_row_dynamic(ctx, 200, 1);
        if (nk_group_begin(ctx, "PCB", NK_WINDOW_TITLE | NK_WINDOW_BORDER)) {
            sprint_require(sprint_plugin_gui(ctx));
            nk_group_end(ctx);
        }

        nk_layout_row_static(ctx, 30, 100, 5);
        if (nk_button_label(ctx, "Do nothing"))
            sprint_plugin_end(SPRINT_OPERATION_NONE);
        if (nk_button_label(ctx, "Replace abs."))
            sprint_plugin_end(SPRINT_OPERATION_REPLACE_ABSOLUTE);
        if (nk_button_label(ctx, "Add abs."))
            sprint_plugin_end(SPRINT_OPERATION_ADD_ABSOLUTE);
        if (nk_button_label(ctx, "Replace rel."))
            sprint_plugin_end(SPRINT_OPERATION_REPLACE_RELATIVE);
        if (nk_button_label(ctx, "Add rel."))
            sprint_plugin_end(SPRINT_OPERATION_ADD_RELATIVE);

        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
        if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
        nk_layout_row_dynamic(ctx, 22, 2);
        nk_property_int(ctx, "Compression:", 0, &property1, 100, 10, 1);
        nk_property_int(ctx, "Other:", 0, &property2, 100, 10, 1);
        nk_layout_row_dynamic(ctx, 22, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, gui->buf, sizeof(gui->buf) - 1, nk_filter_default);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_DEACTIVATED, gui->buf2, sizeof(gui->buf2) - 1, nk_filter_default);
        if (nk_button_label(ctx, "Done"))
            printf("%s\n", gui->buf);

        nk_layout_row_dynamic(ctx, 22, 3);
        nk_progress(ctx, &gui->mprog, 100, NK_MODIFIABLE);
        nk_slider_int(ctx, 0, &gui->mslider, 16, 1);
        nk_checkbox_label(ctx, "check", &gui->mcheck);

        const float values[]={26.0f,13.0f,30.0f,15.0f,25.0f,10.0f,20.0f,40.0f,12.0f,8.0f,22.0f,28.0f};
        nk_layout_row_dynamic(ctx, 150, 1);
        nk_chart_begin(ctx, NK_CHART_COLUMN, NK_LEN(values), 0, 50);
        for (size_t i = 0; i < NK_LEN(values); ++i)
            nk_chart_push(ctx, values[i]);
        nk_chart_end(ctx);
    }
    nk_end(ctx);
}


#ifdef WIN32

#include <windows.h>

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

int gui_main(void)
{
    GdiFont* font;
    struct nk_context *ctx;
    struct demo_gui gui;
    init_gui(&gui);

    WNDCLASSW wc;
    ATOM atom;
    RECT rect = { 0, 0, gui.width, gui.height };
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

    size_t window_name_size = strlen(gui.title) + 1, window_name_out_size = 0;
    wchar_t window_name[window_name_size];
    mbstowcs_s(&window_name_out_size, window_name, window_name_size, gui.title, window_name_size - 1);
    AdjustWindowRectEx(&rect, style, FALSE, exstyle);
    wnd = CreateWindowExW(exstyle, wc.lpszClassName, window_name,
                          style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                          rect.right - rect.left, rect.bottom - rect.top,
                          NULL, NULL, wc.hInstance, NULL);
    dc = GetDC(wnd);

    /* GUI */
    font = nk_gdifont_create("Segoe UI", 14);
    ctx = nk_gdi_init(font, dc, gui.width, gui.height);
    init_ctx(ctx);

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
        //canvas(ctx);
        handle_gui(ctx, &gui);

        /* Draw */
        nk_gdi_render(nk_rgb_cf(gui.bg));
    }

    nk_gdifont_del(font);
    ReleaseDC(wnd, dc);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

#else

#include "nuklear_sdl.h"

int gui_main(void)
{
    /* Platform */
    SDL_Window *win;
    SDL_Renderer *renderer;
    int running = 1;
    int flags = 0;
    float font_scale = 1;

    /* GUI */
    struct nk_context *ctx;
    struct demo_gui gui;
    init_gui(&gui);

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow(gui.title,
                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           gui.width, gui.height, SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);

    if (win == NULL) {
        SDL_Log("Error SDL_CreateWindow %s", SDL_GetError());
        exit(-1);
    }

    flags |= SDL_RENDERER_ACCELERATED;
    flags |= SDL_RENDERER_PRESENTVSYNC;

#if 0
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
#endif

    renderer = SDL_CreateRenderer(win, -1, flags);

    if (renderer == NULL) {
        SDL_Log("Error SDL_CreateRenderer %s", SDL_GetError());
        exit(-1);
    }

    /* scale the renderer output for High-DPI displays */
    {
        int render_w, render_h;
        int window_w, window_h;
        float scale_x, scale_y;
        SDL_GetRendererOutputSize(renderer, &render_w, &render_h);
        SDL_GetWindowSize(win, &window_w, &window_h);
        scale_x = (float)(render_w) / (float)(window_w);
        scale_y = (float)(render_h) / (float)(window_h);
        SDL_RenderSetScale(renderer, scale_x, scale_y);
        font_scale = scale_y;
    }

    /* GUI */
    ctx = nk_sdl_init(win, renderer);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        /* set up the font atlas and add desired font; note that font sizes are
         * multiplied by font_scale to produce better results at higher DPIs */
        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14 * font_scale, &config);*/
        nk_sdl_font_stash_end();

        /* this hack makes the font appear to be scaled down to the desired
         * size and is only necessary when font_scale > 1 */
        font->handle.height /= font_scale;
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        nk_style_set_font(ctx, &font->handle);
    }

    // Load additional stuff
    init_ctx(ctx);

    while (running)
    {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) goto cleanup;
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        /* GUI */
        handle_gui(ctx, &gui);

        SDL_SetRenderDrawColor(renderer, (Uint8) (gui.bg.r * 255), (Uint8) (gui.bg.g * 255), (Uint8) (gui.bg.b * 255), 255);
        SDL_RenderClear(renderer);

        nk_sdl_render(NK_ANTI_ALIASING_ON);

        SDL_RenderPresent(renderer);
    }

    cleanup:
    nk_sdl_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

#endif