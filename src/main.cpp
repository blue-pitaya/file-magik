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
    cbreak();
    keypad(stdscr, true);
    noecho();
    nodelay(stdscr, true);

    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, 16);
    init_pair(2, COLOR_BLUE, 16);
    init_pair(3, 16, COLOR_WHITE);

    UI ui;
    ui.init();

    int key = -1;
    do {
        erase();
        ui.handle_key(key);
        ui.render();
        refresh();

        napms(16);
        key = getch();
    } while (key != 'q');

    endwin();

    return 0;
}
