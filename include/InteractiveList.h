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
    int show_selection;
} InteractiveList;

InteractiveList *InteractiveList_new(int);
void InteractiveList_free(InteractiveList *);

void InteractiveList_move_idx(InteractiveList *, int);
void InteractiveList_append(InteractiveList *, char *, unsigned int);
void InteractiveList_set_bounds(InteractiveList *, Vec2d, Vec2d);
void InteractiveList_draw(InteractiveList *);
void InteractiveList_sort(InteractiveList *);

#endif
