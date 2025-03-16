

void QSau_ShowConsole(int yes_or_no);
void QSau_Update();
void QSau_SDLInit(void* sdl_window_ptr);
void QSau_SDLEvent(void* event_ptr);
void QSau_FinishRendering();
void QSau_SDLEvent_Starting();
void QSau_SDLEvent_Ending();
void QSau_Clear();

enum
{
        CBID_NULL,
        CBID_SMNU_START,
};
int qs_callback(int id);


enum
{
        QSAU_MENU_TEST,
        QSAU_MENU_SAUCEMENU,
        QSAU_MENU_CONSOLE,
        QSAU_MENU_COUNT_ALL,
};

int QSau_MenuVisibleThisFrame(int menuid);
int QSau_MenuVisible(int menuid,int yes_or_no);