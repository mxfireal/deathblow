#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "qs_nuklear.h"

#include "qs_nui.h"
#include "quakedef.h"

extern int con_totallines; // total lines in console scrollback
extern int con_backscroll; // lines up from bottom to display
extern int con_current;    // where next message will be printed
extern int con_x;          // offset in current line for next print
extern char *con_text;

extern int con_buffersize;

char field_buffer[128];
int field_length;
int dont_care;


void ui_console()
{
        if (nk_begin(ctx, "QConsole", nk_rect(260, 130, 230, 250),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                         NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
        {

                nk_layout_row_static(ctx, 180, 278, 1);
                int len = strlen(con_text);
                nk_edit_string(ctx, NK_EDIT_BOX, con_text, &dont_care, len, nk_filter_default);
                
                nk_edit_string(ctx, NK_EDIT_FIELD, field_buffer, &field_length, 128, nk_filter_default);

                //experimental

        }
        nk_end(ctx);
}