#include "UI.h"
#include "InteractiveList.h"
#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <ncurses.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

UI::UI()
    : parent_section(InteractiveList(false)),
      cwd_section(InteractiveList(true)),
      child_section(InteractiveList(false)) {
}

void UI::load_dir_content(InteractiveList &list, const std::string &path) {
    DIR *d;
    if ((d = opendir(path.c_str())) == NULL) {
        throw std::runtime_error("Error.");
    }

    struct dirent *dir;
    int style = 0;
    while ((dir = readdir(d)) != NULL) {
        if (std::strcmp(dir->d_name, ".") == 0) {
            continue;
        }
        if (std::strcmp(dir->d_name, "..") == 0) {
            continue;
        }

        if (dir->d_type == DT_DIR) {
            style = 1;
        } else {
            style = 0;
        }

        std::string name = dir->d_name;
        list.append(name, style);
    }
    closedir(d);

    list.sort();
}

void UI::on_resize() {
    Vec2d content_position(1, 2);
    Vec2d content_size(terminal_size.x - 2, terminal_size.y - 3);
    int number_of_panels = 3;
    int x = (content_size.x - (number_of_panels - 1)) / number_of_panels;
    Vec2d panel_size(x, content_size.y);

    parent_section.position = content_position;
    parent_section.size = panel_size;

    Vec2d cwd_panel_position(content_position.x + panel_size.x + 1,
                             content_position.y);
    cwd_section.position = cwd_panel_position;
    cwd_section.size = panel_size;

    Vec2d child_panel_position(cwd_panel_position.x + panel_size.x + 2,
                               content_position.y);

    child_section.position = child_panel_position;
    child_section.size = panel_size;
}

void UI::init() {
    cwd = std::filesystem::current_path().u8string();

    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        throw std::runtime_error("Error.");
    }
    terminal_size.x = w.ws_col;
    terminal_size.y = w.ws_row;

    load_dir_content(cwd_section, ".");
    load_dir_content(parent_section, "..");

    this->on_resize();
}

void UI::handle_key(int key) {
    bool is_list_move_key = false;
    int last_cwd_section_selected_idx = cwd_section.selected_idx;

    switch (key) {
    case 'j':
        is_list_move_key = true;
        cwd_section.move_idx(1);
        break;
    case 'k':
        is_list_move_key = true;
        cwd_section.move_idx(-1);
        break;
    }

    int selected_idx = cwd_section.selected_idx;
    if (is_list_move_key && (last_cwd_section_selected_idx != selected_idx)) {
        auto item = cwd_section.get_selected_item();
        // TODO: magic "1"
        if (item != nullptr && item->style == 1) {
            std::string path = "./" + item->name;
            load_dir_content(child_section, path);
        } else {
            child_section.clear();
        }
    }
}

void UI::render() {
    // TODO: duplicated content_position
    Vec2d content_position(1, 2);

    // border
    int w = terminal_size.x;
    int h = terminal_size.y;
    int split_line_1_x = w / 3;
    int split_line_2_x = 2 * w / 3;

    for (int x = 0; x < w; x++) {
        mvprintw(content_position.y - 1, x, "%s", "\u2500");
        mvprintw(h - 1, x, "%s", "\u2500");
    }

    for (int y = content_position.y - 1; y < h; y++) {
        mvprintw(y, 0, "%s", "\u2502");
        mvprintw(y, w / 3, "%s", "\u2502");
        mvprintw(y, 2 * w / 3, "%s", "\u2502");
        mvprintw(y, w - 1, "\u2502");
    }

    mvprintw(content_position.y - 1, 0, "\u250C");
    mvprintw(content_position.y - 1, split_line_1_x, "\u252C");
    mvprintw(h - 1, split_line_1_x, "\u2534");
    mvprintw(content_position.y - 1, split_line_2_x, "\u252C");
    mvprintw(h - 1, split_line_2_x, "\u2534");
    mvprintw(content_position.y - 1, w - 1, "\u2510");

    mvprintw(h - 1, 0, "\u2514");
    mvprintw(h - 1, w - 1, "\u2518");

    // cwd
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(0, 0, " %s ", cwd.c_str());
    attroff(COLOR_PAIR(2) | A_BOLD);

    // lists
    cwd_section.draw();
    parent_section.draw();
    child_section.draw();
}
