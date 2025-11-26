#include "UI.h"
#include "InteractiveList.h"
#include "Vec2d.h"
#include "core.h"
#include <dirent.h>
#include <glib.h>
#include <linux/limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

enum { MAIN_FRAME_Y = 1 };

UI *UI_new() {
    UI *ui = g_malloc(sizeof(UI));
    ui->parent_section = InteractiveList_new();
    ui->cwd_section = InteractiveList_new();
    ui->child_section = InteractiveList_new();
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
    free(ui);
}

int get_cwd_panel_x(UI *ui) {
    return ui->terminal_size->x / 3 + 1;
}

int get_parent_panel_x(UI *ui) {
    return MAIN_FRAME_Y + 1;
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
        mvprintw(MAIN_FRAME_Y, x, "%s", "\u2500");
        mvprintw(h - 1, x, "%s", "\u2500");
    }

    for (int y = MAIN_FRAME_Y; y < h; y++) {
        mvprintw(y, 0, "%s", "\u2502");
        mvprintw(y, w / 3, "%s", "\u2502");
        mvprintw(y, 2 * w / 3, "%s", "\u2502");
        mvprintw(y, w - 1, "\u2502");
    }

    mvprintw(MAIN_FRAME_Y, 0, "\u250C");
    mvprintw(MAIN_FRAME_Y, split_line_1_x, "\u252C");
    mvprintw(h - 1, split_line_1_x, "\u2534");
    mvprintw(MAIN_FRAME_Y, split_line_2_x, "\u252C");
    mvprintw(h - 1, split_line_2_x, "\u2534");
    mvprintw(MAIN_FRAME_Y, w - 1, "\u2510");

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

    // FIXME:
    //  qsort(f->items, f->size, sizeof(FileListItem), _strcmp);

    return ERR_OK;
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

    // FIXME: invalid size for both
    int x = get_cwd_panel_x(ui);
    int y = MAIN_FRAME_Y + 1;
    InteractiveList_set_bounds(ui->cwd_section, x, y, 20, 10);
    y = MAIN_FRAME_Y + 1;
    x = get_parent_panel_x(ui);
    InteractiveList_set_bounds(ui->parent_section, x, y, 20, 10);

    return ERR_OK;
};

Err UI_handle_key(UI *ui, int key) {
    switch (key) {
    case 'q':
        return ERR_EXIT;
    case 'j':
        InteractiveList_move_idx(ui->cwd_section, 1);
        break;
    case 'k':
        InteractiveList_move_idx(ui->cwd_section, -1);
        break;
    }

    return ERR_OK;
}

void UI_render(UI *ui) {
    draw_border(ui);
    draw_cwd(ui);
    InteractiveList_draw(ui->cwd_section);
    InteractiveList_draw(ui->parent_section);
}
