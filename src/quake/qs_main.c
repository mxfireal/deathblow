
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "qs_nuklear.h"
#include "quakesaucedef.h"
#define GLQUAKE_H // dirty trick to avoid getting glquake defs
#include "quakedef.h"

SDL_Window *draw_context;
SDL_GLContext gl_context;
typedef int(*funcptr_t)();

funcptr_t operation;
int operation_idle();
int operation_game();

int running;
int in_game;

#if DEBUG
#define APPEND_STR " [debug]"
#else
#define APPEND_STR " "
#endif


#define use_fallback 0

void Host_Init (void);
int main(int argc, char *argv[])
{
        #if use_fallback
        return spasm_sdl_main(argc, argv);
        #endif
        int flags = 0;
        flags |= SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
        char caption[64];
        q_snprintf(caption, sizeof(caption), "quakesauce " QUAKESAUCE_VER_STRING APPEND_STR);
        float aspect = (3.0 / 4.0);
        int width = 800;
        int height = (aspect * width);
        operation = &operation_idle;
        running=1;

        SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
        SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS);

        //create window
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); //dont know.. dont care
#if 0
        /* fsaa */
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, fsaa > 0 ? 1 : 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, fsaa);
        #endif
        
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#if 0
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
        draw_context = SDL_CreateWindow(caption, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
        SDL_ShowWindow(draw_context);
        SDL_RaiseWindow(draw_context);

        gl_context = SDL_GL_CreateContext(draw_context);

        glewExperimental = 1;
        if (glewInit() != GLEW_OK) {
            Sys_Error("Failed to setup GLEW\n");
            exit(1);
        }

        QSau_SDLInit(draw_context);

        //operations
        running=1;

        //init game
        extern quakeparms_t	parms;
        host_parms = &parms;
	parms.basedir = ".";

	parms.argc = argc;
	parms.argv = argv;

	parms.errstate = 0;

	COM_InitArgv(parms.argc, parms.argv);
        Sys_Init();
        parms.memsize = (256 * 1024 * 1024);
	if (COM_CheckParm("-heapsize"))
	{
		int t = COM_CheckParm("-heapsize") + 1;
		if (t < com_argc)
			parms.memsize = Q_atoi(com_argv[t]) * 1024;
	}

	parms.membase = malloc (parms.memsize);

	if (!parms.membase)
		Sys_Error ("Not enough memory free; check disk space\n");

        Host_Init();
        static float last_time = 0;
        last_time = (float)SDL_GetTicks64() / 1000.0f;

        while (running)
        {
                float this_time =(float)SDL_GetTicks64() / 1000.0f;
                operation();
                 //render ui elements
                QSau_FinishRendering();
                static int test=0;
                
                SDL_GL_SwapWindow(draw_context);
        }

        return 0;
}

void change_operation(funcptr_t loperation)
{
        operation = loperation;
        int height,width;
        SDL_GetWindowSize(draw_context,&width,&height);
        glViewport(0,0,width,height);
        vid.width = width;
        vid.height = height;
}


int sdl_msgloop();
void IN_SendKeyEvents();
int operation_idle()
{
        in_game=0;

        sdl_msgloop();
        QSau_MenuVisibleThisFrame(QSAU_MENU_SAUCEMENU);

        //render
        glClearColor(0,0.5f,0.5f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
}

void Host_Frame (float time);
void GL_SetupState(void);
int operation_game()
{
        //these are static so they can be inintialized to zero
        static double		time, oldtime, newtime;
        in_game=1;
        //(run quake)
        //todo: this
        //QSau_MenuVisibleThisFrame(QSAU_MENU_SAUCEMENU);

        newtime = Sys_DoubleTime ();
        time = newtime - oldtime;

        Host_Frame (time);

        oldtime = newtime;
}

extern SDL_Event* sauce_event;
int sdl_msgloop() 
{
        SDL_Event event;

        QSau_SDLEvent_Starting();

	while (SDL_PollEvent(&event))
        {
                QSau_SDLEvent(&event);
                //... do whatever you need to do

                if (event.type == SDL_QUIT)
                {
                        running = 0;
                }

                
                if (in_game)
                {
                        sauce_event = &event;
                        IN_SendKeyEvents();
                }
                continue;
        }
        QSau_SDLEvent_Ending();
}



int qs_callback(int id)
{
        if (id == CBID_SMNU_START)
        {
                change_operation(&operation_game);
                Cbuf_AddText("map start"); //hacky
                return;
        }
}