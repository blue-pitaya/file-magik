#include <dirent.h>
#include <locale.h>
#include <glib.h>
#include <ncurses.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

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

    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl");
        return 1;
    }
    int terminal_w = w.ws_col;
    int terminal_h = w.ws_row;

    initscr();
    curs_set(0);
    // raw();
    keypad(stdscr, TRUE);
    noecho();

    for (int x = 0; x < terminal_w; x++) {
        mvprintw(0, x, "%s", "\u2500");
        mvprintw(terminal_h - 1, x, "%s", "\u2500");
    }
    for (int y = 0; y < terminal_h; y++) {
        mvprintw(y, 0, "%s", "\u2502");
        mvprintw(y, terminal_w - 1, "\u2502");
    }
    mvprintw(0, 0, "\u250C");
    mvprintw(0, terminal_w - 1, "\u2510");
    mvprintw(terminal_h - 1, 0, "\u2514");
    mvprintw(terminal_h - 1, terminal_w - 1, "\u2518");
    refresh();

    int ch = getch();
    move(10, 10);
    if (ch == 'j') {
        printw("NICE");
    } else {
        printw("Errror!");
    }
    refresh();

    getch();

    endwin();

    return 0;
}
