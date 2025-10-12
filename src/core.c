#include "core.h"
#include <dirent.h>
#include <glib.h>
#include <linux/limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

enum { MAIN_FRAME_Y = 1 };

void append_file(UI *ui, struct dirent *dir) {
    ui->list.file_name[ui->list.size] = dir->d_name;
    ui->list.file_type[ui->list.size] = dir->d_type;
    ui->list.size++;
}

int _strcmp(const void *p1, const void *p2) {
    return strcmp(*(const char **)p1, *(const char **)p2);
}

Err load_files(UI *ui) {
    DIR *d;
    if ((d = opendir(".")) == NULL) {
        return ERR_FAIL;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (g_str_equal(dir->d_name, ".")) {
            continue;
        }
        if (g_str_equal(dir->d_name, "..")) {
            continue;
        }
        append_file(ui, dir);
    }
    closedir(d);

    if (ui->list.size > 0) {
        ui->list.selected_idx = 0;
    }

    qsort(ui->list.file_name, ui->list.size, sizeof(char *), _strcmp);

    return ERR_OK;
}

Err load_cwd(UI *ui) {
    if (getcwd(ui->cwd, PATH_MAX) == NULL) {
        return ERR_FAIL;
    };

    return ERR_OK;
}

Err load_terminal_size(UI *ui) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        return ERR_FAIL;
    }
    ui->terminal_w = w.ws_col;
    ui->terminal_h = w.ws_row;

    return ERR_OK;
}

Err UI_init(UI *ui) {
    int err;
    if ((ui->cwd = malloc(PATH_MAX)) == NULL) {
        return ERR_FAIL;
    }
    if ((err = load_cwd(ui)) != ERR_OK) {
        return err;
    }
    if ((err = load_terminal_size(ui)) != ERR_OK) {
        return err;
    }
    ui->list.size = 0;
    ui->list.selected_idx = -1;
    if ((err = load_files(ui)) != ERR_OK) {
        return err;
    }

    return ERR_OK;
};

void UI_draw_border(UI *ui) {
    int w = ui->terminal_w;
    int h = ui->terminal_h;
    for (int x = 0; x < w; x++) {
        mvprintw(MAIN_FRAME_Y, x, "%s", "\u2500");
        mvprintw(h - 1, x, "%s", "\u2500");
    }
    for (int y = MAIN_FRAME_Y; y < h; y++) {
        mvprintw(y, 0, "%s", "\u2502");
        mvprintw(y, w - 1, "\u2502");
    }
    mvprintw(MAIN_FRAME_Y, 0, "\u250C");
    mvprintw(MAIN_FRAME_Y, w - 1, "\u2510");

    mvprintw(h - 1, 0, "\u2514");
    mvprintw(h - 1, w - 1, "\u2518");
}

void UI_draw_cwd(UI *ui) {
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(0, 0, " %s ", ui->cwd);
    attroff(COLOR_PAIR(2) | A_BOLD);
}

void draw_file_list(UI *ui) {
    int y = MAIN_FRAME_Y + 1;
    for (int i = 0; i < ui->list.size; i++) {
        if (i == ui->list.selected_idx) {
            attron(COLOR_PAIR(3));
            mvprintw(y, 1, "%s", ui->list.file_name[i]);
            attroff(COLOR_PAIR(3));
        } else {
            if (ui->list.file_type[i] == DT_DIR) {
                attron(COLOR_PAIR(2) | A_BOLD);
                mvprintw(y, 1, "%s", ui->list.file_name[i]);
                attroff(COLOR_PAIR(2) | A_BOLD);
            } else {
                mvprintw(y, 1, "%s", ui->list.file_name[i]);
            }
        }
        y++;
        if (y >= ui->terminal_h) {
            break;
        }
    }
}

void FileList_change_selected_idx(FileList *f, int n) {
    if (f->size <= 0) {
        return;
    }
    int res = f->selected_idx + n;
    if (res < 0) {
        f->selected_idx = 0;
    } else if (res >= f->size) {
        f->selected_idx = f->size - 1;
    } else {
        f->selected_idx = res;
    }
}

Err UI_handle_key(UI *ui, int key) {
    switch (key) {
    case 'q':
        return ERR_EXIT;
    case 'j':
        FileList_change_selected_idx(&ui->list, 1);
        break;
    case 'k':
        FileList_change_selected_idx(&ui->list, -1);
        break;
    }

    return ERR_OK;
}

void UI_render(UI *ui) {
    UI_draw_border(ui);
    UI_draw_cwd(ui);
    draw_file_list(ui);
}
