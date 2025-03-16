
#include "qs_nuklear.h"

#include "qs_nui.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024
int initalized = 0;
struct nk_context *ctx;
extern SDL_Window *draw_context;
int order_ok = 0;

#define GLQUAKE_H
#include "quakedef.h"

#define is_enabled 1

void qsau_init()
{
        initalized = 1;
}

void ui_test()
{
        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                         NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
        {
                enum
                {
                        EASY,
                        HARD
                };
                static int op = EASY;
                static int property = 20;

                nk_layout_row_static(ctx, 30, 80, 1);
                if (nk_button_label(ctx, "button"))
                        printf("button pressed!\n");
                nk_layout_row_dynamic(ctx, 30, 2);
                if (nk_option_label(ctx, "easy", op == EASY))
                        op = EASY;
                if (nk_option_label(ctx, "hard", op == HARD))
                        op = HARD;
                nk_layout_row_dynamic(ctx, 22, 1);
                nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);
        }
        nk_end(ctx);
}

int qs_callback(int id);

const char* m_skills[] = {"Easy","Normal","Hard","Nightmare!"};

void ui_saucemenu()
{
        if (nk_begin(ctx, "Welcome to quakesauce", nk_rect(50, 50, 230, 250),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                         NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
        {

                static int skillv = 1;
                nk_layout_row_static(ctx, 30, 170, 1);
                if (nk_button_label(ctx, "Start"))
                {
                        extern cvar_t skill;
                        Cvar_SetValueQuick(&skill,skillv);
                        qs_callback(CBID_SMNU_START);
                }
                
                skillv = nk_combo(ctx, m_skills, NK_LEN(m_skills), skillv, 25, nk_vec2(200,200));
        }
        nk_end(ctx);
}

void ui_console();

int menu_data[QSAU_MENU_COUNT_ALL];
int menu_data_frame[QSAU_MENU_COUNT_ALL];
typedef void (*vfunc_t)();

vfunc_t menu_drawfuncs[] = {
    &ui_test,
    &ui_saucemenu,
    &ui_console,
};
int num_things_drawn;

void QSau_ShowConsole(int yes_or_no)
{
        QSau_MenuVisible(QSAU_MENU_CONSOLE,yes_or_no);
}

void QSau_Update()
{
        if (!ctx) return;
        nk_sdl_handle_grab();

        num_things_drawn = 0;

        for (int i = 0; i < QSAU_MENU_COUNT_ALL; i++)
        {
                int render = menu_data[i] | menu_data_frame[i];
                if (!render)
                        continue;
                vfunc_t func = menu_drawfuncs[i];
                func();
                num_things_drawn++;
                menu_data_frame[i] = 0;
        }
}

int QSau_MenuVisibleThisFrame(int menuid)
{
        menu_data_frame[menuid] = 1;
}
int QSau_MenuVisible(int menuid, int yes_or_no)
{
        menu_data[menuid] = yes_or_no;
}

#if is_enabled
void QSau_SDLInit(void *sdl_window_ptr)
{
        SDL_Window *sw = sdl_window_ptr;
        ctx = nk_sdl_init(sw);

        {
                struct nk_font_atlas *atlas;
                nk_sdl_font_stash_begin(&atlas);
                nk_sdl_font_stash_end();
                /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    /*nk_style_set_font(ctx, &roboto->handle);*/}

    qsau_init();
}
#else
void QSau_SDLInit(void *sdl_window_ptr) {}
#endif

void QSau_SDLEvent(void *event_ptr)
{
        if (!ctx) return;
        nk_sdl_handle_event(event_ptr);
}

void QSau_FinishRendering()
{
        if (!ctx) return;
        QSau_Update(); // cheesy but lol
        if (num_things_drawn)
        {
                nk_sdl_render(NK_ANTI_ALIASING_OFF, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
        }
}

void QSau_SDLEvent_Starting()
{
        if (!ctx) return;
        nk_input_begin(ctx);
}

void QSau_SDLEvent_Ending()
{
        if (!ctx) return;
        nk_input_end(ctx);
}

void QSau_Clear()
{
        if (!ctx) return;
        nk_clear(ctx);
        memset(menu_data, 0, sizeof(menu_data));
}
