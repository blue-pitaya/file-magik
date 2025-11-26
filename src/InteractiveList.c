#include "InteractiveList.h"
#include "glib.h"
#include <ncurses.h>

InteractiveList *InteractiveList_new(int show_selection) {
    InteractiveList *il = g_malloc(sizeof(InteractiveList));

    il->items = NULL;
    il->position.x = 0;
    il->position.y = 0;
    il->size.x = 0;
    il->size.y = 0;
    il->selected_idx = -1;
    il->show_selection = show_selection;

    return il;
}

void InteractiveList_free(InteractiveList *il) {
    for (GList *list = il->items; list != NULL; list = list->next) {
        InteractiveListItem *item = (InteractiveListItem *)list->data;
        g_string_free(item->name, TRUE);
        g_free(item);
    }
    g_list_free(il->items);
    g_free(il);
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

void InteractiveList_set_bounds(InteractiveList *il, Vec2d position,
                                Vec2d size) {
    il->position.x = position.x;
    il->position.y = position.y;
    il->size.x = size.x;
    il->size.y = size.y;
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
        if (il->show_selection && idx == il->selected_idx) {
            attron(COLOR_PAIR(3));
            mvprintw(y, x, "%s", item->name->str);
            attroff(COLOR_PAIR(3));
        } else if (item->style == 1) {
            // TODO: magic number
            attron(COLOR_PAIR(2) | A_BOLD);
            mvprintw(y, x, "%s", item->name->str);
            attroff(COLOR_PAIR(2) | A_BOLD);
        } else {
            mvprintw(y, x, "%s", item->name->str);
        }

        idx++;
        y++;
    }
}

gint compare(gconstpointer _a, gconstpointer _b) {
    return g_strcmp0(((InteractiveListItem *)_a)->name->str,
                     ((InteractiveListItem *)_b)->name->str);
}

void InteractiveList_sort(InteractiveList *this) {
    this->items = g_list_sort(this->items, compare);
}
