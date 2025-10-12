#include "src/UI.h"
#include <dirent.h>
#include <glib.h>
#include <locale.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>

UI ui;

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
