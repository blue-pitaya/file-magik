#ifndef FILE_MAGIK_UI
#define FILE_MAGIK_UI

#include "InteractiveList.h"
#include "Vec2d.h"
#include "core.h"
#include "glib.h"

typedef struct {
    InteractiveList *parent_section;
    InteractiveList *cwd_section;
    InteractiveList *child_section;
    GString *cwd;
    Vec2d *terminal_size;
} UI;

UI *UI_new();
void UI_free(UI *);

Err UI_init(UI *);
Err UI_handle_key(UI *, int key);
void UI_render(UI *);

#endif
