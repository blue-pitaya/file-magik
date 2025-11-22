#ifndef FILE_MAGIK_UI
#define FILE_MAGIK_UI

#include "FileList.h"

typedef struct {
    FileList list;
    char *cwd;
    int terminal_w;
    int terminal_h;
} UI;

Err UI_init(UI *);
Err UI_handle_key(UI *, int key);
void UI_render(UI *);

#endif
