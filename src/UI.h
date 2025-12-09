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

  private:
    void load_dir_content(InteractiveList &list, const std::string &path);
    void on_resize();
};

// #include "InteractiveList.h"
// #include "Vec2d.h"
// #include "core.h"
// #include "glib.h"
//
// typedef struct {
//     GString *cwd;
//     Vec2d *terminal_size;
// } UI;
//
// UI *UI_new();
// void UI_free(UI *);
//
// Err UI_init(UI *);
// Err UI_handle_key(UI *, int key);
// void UI_render(UI *);

#endif
