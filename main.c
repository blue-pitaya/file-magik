#include "src/core.h"
#include <dirent.h>
#include <glib.h>
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

UI ui;

int file_magik_read_dir() {
    DIR *d;
    struct dirent *dir;

    d = opendir("."); // Open current directory
    if (d == NULL) {
        perror("opendir");
        return 1;
    }

    printf("Files in current directory:\n");
    while ((dir = readdir(d)) != NULL) {
        // Skip . and ..
        if (dir->d_name[0] == '.' &&
            (dir->d_name[1] == '\0' ||
             (dir->d_name[1] == '.' && dir->d_name[2] == '\0')))
            continue;

        printf("%s\n", dir->d_name);
    }

    closedir(d);
    return 0;
}

int main(void) {
    setlocale(LC_ALL, "");

    initscr();

    curs_set(0);
    // raw();
    keypad(stdscr, TRUE);
    noecho();

    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, 16);
    init_pair(2, COLOR_BLUE, 16);
    init_pair(3, 16, COLOR_WHITE);

    UI_init(&ui);
    int key = -1;
    do {
        clear();
        UI_handle_key(&ui, key);
        UI_render(&ui);
        refresh();
        key = getch();
    } while (key != 'q');

    endwin();

    return 0;
}
