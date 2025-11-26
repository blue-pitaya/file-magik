#ifndef FILE_MAGIK_INTERACTIVE_LIST
#define FILE_MAGIK_INTERACTIVE_LIST

#include "Vec2d.h"
#include "glib.h"

typedef struct {
    GString *name;
    unsigned int style;
} InteractiveListItem;

typedef struct {
    GList *items;
    Vec2d position;
    Vec2d size;
    int selected_idx;
} InteractiveList;

InteractiveList *InteractiveList_new();
void InteractiveList_free(InteractiveList *);

void InteractiveList_move_idx(InteractiveList *, int);
void InteractiveList_append(InteractiveList *, char *, unsigned int);
void InteractiveList_set_bounds(InteractiveList *, int, int, int, int);
void InteractiveList_draw(InteractiveList *);

#endif
