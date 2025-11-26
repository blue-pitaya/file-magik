#include "UI.h"
#include "InteractiveList.h"
#include "Vec2d.h"
#include "core.h"
#include <dirent.h>
#include <glib.h>
#include <linux/limits.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>

static const Vec2d content_position = {.x = 1, .y = 2};

UI *UI_new() {
    UI *ui = g_malloc(sizeof(UI));
    ui->parent_section = InteractiveList_new(0);
    ui->cwd_section = InteractiveList_new(1);
    ui->child_section = InteractiveList_new(0);
    ui->cwd = g_string_new("");
    ui->terminal_size = Vec2d_new(0, 0);

    return ui;
}

void UI_free(UI *ui) {
    InteractiveList_free(ui->parent_section);
    InteractiveList_free(ui->cwd_section);
    InteractiveList_free(ui->child_section);
    g_string_free(ui->cwd, TRUE);
    Vec2d_free(ui->terminal_size);
    g_free(ui);
}

void load_cwd(UI *ui) {
    gchar *cwd = g_get_current_dir();
    g_string_assign(ui->cwd, cwd);
    g_free(cwd);
}

Err load_terminal_size(UI *ui) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        return ERR_FAIL;
    }
    ui->terminal_size->x = w.ws_col;
    ui->terminal_size->y = w.ws_row;

    return ERR_OK;
}

void draw_border(UI *ui) {
    int w = ui->terminal_size->x;
    int h = ui->terminal_size->y;
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
}

void draw_cwd(UI *ui) {
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(0, 0, " %s ", ui->cwd->str);
    attroff(COLOR_PAIR(2) | A_BOLD);
}

Err load_dir_content(UI *ui, InteractiveList *list, const char *path) {
    DIR *d;
    if ((d = opendir(path)) == NULL) {
        return ERR_FAIL;
    }

    struct dirent *dir;
    int style = 0;
    while ((dir = readdir(d)) != NULL) {
        if (g_str_equal(dir->d_name, ".")) {
            continue;
        }
        if (g_str_equal(dir->d_name, "..")) {
            continue;
        }
        if (dir->d_type == DT_DIR) {
            style = 1;
        } else {
            style = 0;
        }

        InteractiveList_append(list, dir->d_name, style);
    }
    closedir(d);

    InteractiveList_sort(list);

    return ERR_OK;
}

void on_resize(UI *ui) {
    Vec2d content_size;
    content_size.x = ui->terminal_size->x - 2;
    content_size.y = ui->terminal_size->y - 3;

    Vec2d panel_size;
    int number_of_panels = 3;
    panel_size.x = (content_size.x - (number_of_panels - 1)) / number_of_panels;
    panel_size.y = content_size.y;

    InteractiveList_set_bounds(ui->parent_section, content_position,
                               panel_size);

    Vec2d cwd_panel_position;
    cwd_panel_position.x = content_position.x + panel_size.x + 1;
    cwd_panel_position.y = content_position.y;
    InteractiveList_set_bounds(ui->cwd_section, cwd_panel_position, panel_size);

    Vec2d child_panel_position;
    child_panel_position.x = cwd_panel_position.x + panel_size.x + 2;
    child_panel_position.y = content_position.y;
    InteractiveList_set_bounds(ui->child_section, child_panel_position,
                               panel_size);
}

Err UI_init(UI *ui) {
    int err;
    load_cwd(ui);
    err = load_terminal_size(ui);
    if (err != ERR_OK) {
        return err;
    }
    err = load_dir_content(ui, ui->cwd_section, ".");
    if (err != ERR_OK) {
        return err;
    }
    err = load_dir_content(ui, ui->parent_section, "..");
    if (err != ERR_OK) {
        return err;
    }

    on_resize(ui);

    return ERR_OK;
};

void handle_cwd_panel_selection_change() {
}

Err UI_handle_key(UI *ui, int key) {
    int is_list_move_key = 0;
    int last_cwd_section_selected_idx = ui->cwd_section->selected_idx;

    switch (key) {
    case 'q':
        return ERR_EXIT;
    case 'j':
        is_list_move_key = 1;
        InteractiveList_move_idx(ui->cwd_section, 1);
        break;
    case 'k':
        is_list_move_key = 1;
        InteractiveList_move_idx(ui->cwd_section, -1);
        break;
    }

    int selected_idx = ui->cwd_section->selected_idx;
    if (is_list_move_key && (last_cwd_section_selected_idx != selected_idx)) {
        InteractiveListItem *item =
            InteractiveList_get_selected_item(ui->cwd_section);
        if (item != NULL && item->style == 1) {
            GString *path = g_string_new(item->name->str);
            g_string_prepend(path, "./");
            // TODO: handle errpr
            load_dir_content(ui, ui->child_section, path->str);
            g_free(path);
        } else {
            InteractiveList_clear(ui->child_section);
        }
    }

    return ERR_OK;
}

void UI_render(UI *ui) {
    draw_border(ui);
    draw_cwd(ui);
    InteractiveList_draw(ui->cwd_section);
    InteractiveList_draw(ui->parent_section);
    InteractiveList_draw(ui->child_section);
}
