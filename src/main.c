#include <dirent.h>
#include <libgen.h>
#include <linux/limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>

struct dir_listing_item {
	char *name;
	int is_dir;
};

struct dir_listing {
	char path[PATH_MAX];
	struct dir_listing_item *items;
	size_t count;
	size_t capacity;
	int selected;
};

struct app {
	struct dir_listing left;
	struct dir_listing center;
	struct dir_listing right;
};

static int dir_listing_cmp(const void *a, const void *b)
{
	const struct dir_listing_item *ia = a;
	const struct dir_listing_item *ib = b;
	if (ia->is_dir != ib->is_dir)
		return ib->is_dir - ia->is_dir;
	return strcmp(ia->name, ib->name);
}

static void dir_listing_sort(struct dir_listing *list)
{
	qsort(list->items, list->count, sizeof(struct dir_listing_item),
	      dir_listing_cmp);
}

static void dir_listing_free(struct dir_listing *dl)
{
	for (size_t i = 0; i < dl->count; i++)
		free(dl->items[i].name);
	free(dl->items);
	dl->items = NULL;
	dl->count = 0;
	dl->capacity = 0;
	dl->selected = 0;
}

static int dir_listing_init(struct dir_listing *dl, const char *path)
{
	dl->selected = -1;
	strlcpy(dl->path, path, sizeof(dl->path));

	DIR *d = opendir(path);
	if (!d)
		return -1;

	struct dir_listing_item *item;
	struct dirent *e;
	while ((e = readdir(d))) {
		if (e->d_name[0] == '.' &&
		    (e->d_name[1] == '\0' ||
		     (e->d_name[1] == '.' && e->d_name[2] == '\0')))
			continue;

		if (dl->count >= dl->capacity) {
			size_t new_cap = dl->capacity ? dl->capacity * 2 : 16;
			void *tmp = realloc(dl->items, new_cap * sizeof(*item));
			if (!tmp)
				goto fail;
			dl->items = tmp;
			dl->capacity = new_cap;
		}

		char *name = strdup(e->d_name);
		if (!name)
			goto fail;

		dl->items[dl->count].is_dir = e->d_type == DT_DIR;
		dl->items[dl->count].name = name;
		dl->count++;
	}

	dir_listing_sort(dl);

	closedir(d);

	return 0;
fail:
	dir_listing_free(dl);
	closedir(d);

	return -1;
}

static void app_free(struct app *app)
{
	free(app);
}

static int child_dir_listing_init(struct app *app)
{
	char path[PATH_MAX];
	int selected = app->center.selected;

	if (selected < 0)
		return 0;

	dir_listing_free(&app->right);
	if (app->center.items[selected].is_dir) {
		snprintf(path, sizeof(path), "%s/%s", app->center.path,
			 app->center.items[selected].name);
		if (dir_listing_init(&app->right, path))
			return -1;
	}

	return 0;
}

static int app_init(struct app *app)
{
	char cwd[PATH_MAX];
	if (!getcwd(cwd, sizeof(cwd)))
		return -1;

	if (dir_listing_init(&app->center, cwd))
		return -1;

	if (app->center.count > 0) {
		app->center.selected = 0;
		if (child_dir_listing_init(app))
			goto fail_center;
	}

	char tmp[PATH_MAX];
	strlcpy(tmp, cwd, sizeof(tmp));
	if (dir_listing_init(&app->left, dirname(tmp)))
		goto fail_right;

	return 0;

fail_right:
	dir_listing_free(&app->right);
fail_center:
	dir_listing_free(&app->center);
	return -1;
}

static int move_center_selected(struct app *app, int n)
{
	if (app->center.count == 0)
		return 0;

	int next = app->center.selected + n;
	if (next < 0)
		next = 0;
	else if (next >= (int)app->center.count)
		next = app->center.count - 1;

	app->center.selected = next;
	child_dir_listing_init(app);

	return 0;
}

static void draw_border(int cols, int top_y, int w, int h)
{
	int bot_y = top_y + h - 1;

	for (int i = 0; i < w; i++) {
		mvprintw(top_y, i, "\u2500");
		mvprintw(bot_y, i, "\u2500");
	}
	for (int i = top_y; i <= bot_y; i++) {
		mvprintw(i, 0, "\u2502");
		mvprintw(i, w - 1, "\u2502");
	}
	for (int j = 1; j < cols; j++) {
		int x = j * w / cols;
		for (int i = top_y; i <= bot_y; i++)
			mvprintw(i, x, "\u2502");
		mvprintw(top_y, x, "\u252C");
		mvprintw(bot_y, x, "\u2534");
	}
	mvprintw(top_y, 0, "\u250C");
	mvprintw(top_y, w - 1, "\u2510");
	mvprintw(bot_y, 0, "\u2514");
	mvprintw(bot_y, w - 1, "\u2518");
}

static void list_draw(struct dir_listing *list, int x, int y, int w, int h)
{
	int len;

	for (int i = 0; i < list->count && i < h; i++) {
		struct dir_listing_item *it = &list->items[i];
		len = strlen(it->name);

		if (list->selected >= 0 && i == list->selected) {
			attron(COLOR_PAIR(3));
		} else if (it->is_dir) {
			attron(COLOR_PAIR(2) | A_BOLD);
		}

		if (len >= w)
			mvprintw(y + i, x, "%.*s~", w - 2, it->name);
		else
			mvprintw(y + i, x, "%s", it->name);

		if (list->selected >= 0 && i == list->selected)
			attroff(COLOR_PAIR(3));
		else if (it->is_dir)
			attroff(COLOR_PAIR(2) | A_BOLD);
	}
}

static void render(struct app *app)
{
	int col_w = COLS / 3;

	draw_border(3, 1, COLS, LINES - 1);
	list_draw(&app->left, 1, 2, col_w - 1, LINES - 3);
	list_draw(&app->center, col_w + 1, 2, col_w - 1, LINES - 3);
	list_draw(&app->right, 2 * COLS / 3 + 1, 2,
		  COLS - 1 - (2 * COLS / 3 + 1), LINES - 3);

	attron(COLOR_PAIR(2) | A_BOLD);
	mvprintw(0, 0, " %s ", app->center.path);
	attroff(COLOR_PAIR(2) | A_BOLD);
}

static void term_init(void)
{
	/* let C library interpret multibyte/wide characters (e.g. box-drawing) */
	setlocale(LC_ALL, "");

	/* enter curses mode, allocate stdscr */
	initscr();
	/* make the terminal cursor invisible */
	curs_set(0);

	/* disable line buffering and erase/kill character processing */
	cbreak();
	/* let getch() return KEY_LEFT etc. instead of raw escape sequences */
	keypad(stdscr, TRUE);
	/* disable automatic echoing of characters typed by the user */
	noecho();
	/* make getch() non-blocking */
	nodelay(stdscr, TRUE);

	/* allocate color pair table and initialize COLORS/COLOR_PAIRS */
	start_color();
	/* assign the terminal's native fg/bg to color number -1 */
	use_default_colors();
	init_pair(1, COLOR_WHITE, 16);
	init_pair(2, COLOR_BLUE, 16);
	init_pair(3, 16, COLOR_WHITE);
}

int main(void)
{
	struct app *app;

	app = calloc(1, sizeof(*app));
	if (!app) {
		perror("malloc");
		return 1;
	}

	if (app_init(app)) {
		perror("app_init");
		return 1;
	}

	term_init();

	int key = -1;
	do {
		erase();

		/* handle keys */
		switch (key) {
		case 'j':
			move_center_selected(app, 1);
			break;
		case 'k':
			move_center_selected(app, -1);
			break;
		}

		render(app);

		refresh();
		napms(16);
		key = getch();
	} while (key != 'q');

	endwin();
	app_free(app);

	return 0;
}
