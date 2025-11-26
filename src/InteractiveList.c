#include "InteractiveList.h"
#include "glib.h"
#include <ncurses.h>
#include <stdlib.h>

InteractiveList *InteractiveList_new() {
    InteractiveList *il = g_malloc(sizeof(InteractiveList));

    il->items = NULL;
    il->position.x = 0;
    il->position.y = 0;
    il->size.x = 0;
    il->size.y = 0;
    il->selected_idx = -1;

    return il;
}

void InteractiveList_free(InteractiveList *il) {
    for (GList *list = il->items; list != NULL; list = list->next) {
        // FIXME: this is bad
        g_free(list->data);
    }
    g_list_free(il->items);
    free(il);
}

void InteractiveList_move_idx(InteractiveList *il, int n) {
    int length = g_list_length(il->items);
    if (length <= 0) {
        return;
    }

    int res = il->selected_idx + n;
    if (res < 0) {
        il->selected_idx = 0;
    } else if (res >= il->size.y) {
        il->selected_idx = il->size.y - 1;
    } else if (res >= length) {
        il->selected_idx = length - 1;
    } else {
        il->selected_idx = res;
    }
}

void InteractiveList_append(InteractiveList *il, char *name,
                            unsigned int style) {
    InteractiveListItem *item = g_malloc(sizeof(InteractiveListItem));
    item->name = g_string_new(name);
    item->style = style;
    il->items = g_list_append(il->items, item);

    if (il->selected_idx < 0 && (g_list_length(il->items) > 0)) {
        il->selected_idx = 0;
    }
}

void InteractiveList_set_bounds(InteractiveList *il, int x, int y, int w,
                                int h) {
    il->position.x = x;
    il->position.y = y;
    il->size.x = w;
    il->size.y = h;
}

void InteractiveList_draw(InteractiveList *il) {
    int y = il->position.y;
    int x = il->position.x;
    int size = g_list_length(il->items);

    int idx = 0;
    for (GList *list = il->items; list != NULL; list = list->next) {
        if (idx >= il->size.y) {
            break;
        }
        InteractiveListItem *item = list->data;
        if (idx == il->selected_idx) {
            attron(COLOR_PAIR(3));
            mvprintw(y, x, "%s", item->name->str);
            attroff(COLOR_PAIR(3));
        } else {
            // TODO: magic number
            if (item->style == 1) {
                attron(COLOR_PAIR(2) | A_BOLD);
                mvprintw(y, x, "%s", item->name->str);
                attroff(COLOR_PAIR(2) | A_BOLD);
            } else {
                mvprintw(y, x, "%s", item->name->str);
            }
        }
        idx++;
        y++;
    }
}
