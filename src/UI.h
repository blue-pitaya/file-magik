#ifndef FILE_MAGIK_UI
#define FILE_MAGIK_UI

#include "InteractiveList.h"
#include <string>

class UI {
  public:
    InteractiveList parent_section;
    InteractiveList cwd_section;
    InteractiveList child_section;
    std::string cwd;
    Vec2d terminal_size;

    UI();

    void init();
    void handle_key(int key);
    void render();
    void go_parent();

  private:
    void load_dir_content(InteractiveList &list, const std::string &path);
    void on_resize();
};

#endif
