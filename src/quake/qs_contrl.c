#include <SDL2/SDL.h>

void QSC_SetMouseLocked(int val)
{
        SDL_SetRelativeMouseMode(val);
}