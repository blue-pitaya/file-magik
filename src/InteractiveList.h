#ifndef FILE_MAGIK_INTERACTIVE_LIST
#define FILE_MAGIK_INTERACTIVE_LIST

#include "Vec2d.h"
#include <string>
#include <vector>

class InteractiveListItem {
  public:
    std::string name;
    int style;

    InteractiveListItem(std::string name, int style);
};

class InteractiveList {
  public:
    std::vector<InteractiveListItem> items;
    Vec2d position;
    Vec2d size;
    int selected_idx = -1;
    bool show_selection;

    InteractiveList(bool show_selection);

    void move_idx(int n);
    void append(const std::string &name, int style);
    void draw();
    void sort();
    InteractiveListItem *get_selected_item();
    void clear();
};

#endif
