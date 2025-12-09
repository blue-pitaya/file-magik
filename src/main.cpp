#include "UI.h"
#include <dirent.h>
#include <locale.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>

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

    mvprintw(10, 19, "HELLO");

    UI ui;
    ui.init();

    int key = -1;
    do {
        clear();
        ui.handle_key(key);
        ui.render();
        refresh();
        key = getch();
    } while (key != 'q');

    endwin();

    return 0;
}
