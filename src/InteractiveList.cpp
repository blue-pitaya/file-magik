#include "InteractiveList.h"
#include <algorithm>
#include <ncurses.h>

InteractiveListItem::InteractiveListItem(std::string name, int style)
    : name(name), style(style) {
}

InteractiveList::InteractiveList(bool show_selection)
    : show_selection(show_selection) {
}

void InteractiveList::move_idx(int n) {
    auto length = items.size();
    if (length <= 0) {
        return;
    }

    int res = selected_idx + n;
    if (res < 0) {
        selected_idx = 0;
    } else if (res >= size.y) {
        selected_idx = size.y - 1;
    } else if (res >= length) {
        selected_idx = length - 1;
    } else {
        selected_idx = res;
    }
}

void InteractiveList::append(const std::string &name, int style) {
    items.emplace_back(name, style);

    if (selected_idx < 0) {
        selected_idx = 0;
    }
}

void InteractiveList::draw() {
    int y = position.y;
    int x = position.x;
    int length = items.size();

    int idx = 0;
    for (auto &item : items) {
        if (show_selection && idx == selected_idx) {
            attron(COLOR_PAIR(3));
            mvprintw(y, x, "%s", item.name.c_str());
            attroff(COLOR_PAIR(3));
        } else if (item.style == 1) {
            // TODO: magic number
            attron(COLOR_PAIR(2) | A_BOLD);
            mvprintw(y, x, "%s", item.name.c_str());
            attroff(COLOR_PAIR(2) | A_BOLD);
        } else {
            mvprintw(y, x, "%s", item.name.c_str());
        }

        idx++;
        y++;
    }
}

bool compare_items(const InteractiveListItem &a, const InteractiveListItem &b) {
    return a.name < b.name;
}

void InteractiveList::sort() {
    std::sort(items.begin(), items.end(), compare_items);
}

InteractiveListItem *InteractiveList::get_selected_item() {
    if (selected_idx >= 0 && selected_idx < items.size()) {
        return &items[selected_idx];
    }

    return nullptr;
}

void InteractiveList::clear() {
    items.clear();
    selected_idx = -1;
}
