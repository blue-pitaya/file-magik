#include <dirent.h>
#include <linux/limits.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//FIXME: should allow more items
#define MAX_ITEMS 2048

#define STYLE_FILE 0
#define STYLE_DIR 1

struct list_item {
	/* +1 for terminator */
	char name[NAME_MAX + 1];
	int style;
};

struct list {
	struct list_item items[MAX_ITEMS];
	int count;
	int selected_idx;
};

struct ui {
	char cwd_path[PATH_MAX];
	struct list cwd_list;
	struct list child_list;
};

static void list_init(struct list *list)
{
	memset(list, 0, sizeof(*list));
	list->selected_idx = -1;
}

static void list_clear(struct list *l)
{
	l->count = 0;
	l->selected_idx = -1;
}

static int list_append(struct list *list, const char *name, int style)
{
	struct list_item *it;

	if (list->count >= MAX_ITEMS)
		return ERR;

	it = &list->items[list->count++];
	strncpy(it->name, name, NAME_MAX);
	it->name[NAME_MAX] = '\0';
	it->style = style;

	return 0;
}

static void list_move(struct list *list, int n)
{
	int next;

	if (list->count <= 0)
		return;

	next = list->selected_idx + n;

	if (next < 0)
		list->selected_idx = 0;
	else if (next >= list->count)
		list->selected_idx = list->count - 1;
	else
		list->selected_idx = next;
}

static int item_cmp(const void *a, const void *b)
{
	return strcmp(((const struct list_item *)a)->name,
		      ((const struct list_item *)b)->name);
}

static void list_sort(struct list *list)
{
	qsort(list->items, list->count, sizeof(struct list_item), item_cmp);
}

static int load_dir(struct list *list, const char *path)
{
	struct dirent *e;
	DIR *d;
	int style;

	list_clear(list);

	d = opendir(path);
	if (!d)
		return -1;

	while ((e = readdir(d))) {
		if (e->d_name[0] == '.' &&
		    (e->d_name[1] == '\0' ||
		     (e->d_name[1] == '.' && e->d_name[2] == '\0')))
			continue;

		style = (e->d_type == DT_DIR) ? STYLE_DIR : STYLE_FILE;
		list_append(list, e->d_name, style);
	}

	closedir(d);
	list_sort(list);
	return 0;
}

static void ui_update_child(struct ui *ui)
{
	struct list_item *sel;
	char path[PATH_MAX];

	list_clear(&ui->child_list);

	if (ui->cwd_list.selected_idx < 0)
		return;

	sel = &ui->cwd_list.items[ui->cwd_list.selected_idx];
	if (sel->style != STYLE_DIR)
		return;

	snprintf(path, sizeof(path), "%s/%s", ui->cwd_path, sel->name);
	load_dir(&ui->child_list, path);
}

static void ui_init(struct ui *ui)
{
	list_init(&ui->cwd_list);
	list_init(&ui->child_list);

	if (!getcwd(ui->cwd_path, sizeof(ui->cwd_path)))
		strcpy(ui->cwd_path, ".");

	load_dir(&ui->cwd_list, ui->cwd_path);

	if (ui->cwd_list.count > 0) {
		ui->cwd_list.selected_idx = 0;
		ui_update_child(ui);
	}
}

static void draw_border(int cols, int top_y, int w, int h)
{
	int bot_y;
	int i;
	int j;
	int x;

	bot_y = top_y + h - 1;

	/* horizontal lines */
	for (i = 0; i < w; i++) {
		mvprintw(top_y, i, "\u2500");
		mvprintw(bot_y, i, "\u2500");
	}

	/* left and right edges */
	for (i = top_y; i <= bot_y; i++) {
		mvprintw(i, 0, "\u2502");
		mvprintw(i, w - 1, "\u2502");
	}

	/* vertical dividers */
	for (j = 1; j < cols; j++) {
		x = j * w / cols;

		for (i = top_y; i <= bot_y; i++)
			mvprintw(i, x, "\u2502");

		mvprintw(top_y, x, "\u252C");
		mvprintw(bot_y, x, "\u2534");
	}

	/* corners */
	mvprintw(top_y, 0, "\u250C");
	mvprintw(top_y, w - 1, "\u2510");
	mvprintw(bot_y, 0, "\u2514");
	mvprintw(bot_y, w - 1, "\u2518");
}

static void list_draw(struct list *list, int x, int y, int w, int h)
{
	struct list_item *it;
	int len;
	int i;

	for (i = 0; i < list->count && i < h; i++) {
		it = &list->items[i];
		len = strlen(it->name);

		if (list->selected_idx >= 0 && i == list->selected_idx) {
			attron(COLOR_PAIR(3));
		} else if (it->style == STYLE_DIR) {
			attron(COLOR_PAIR(2) | A_BOLD);
		}

		if (len >= w)
			mvprintw(y + i, x, "%.*s~", w - 2, it->name);
		else
			mvprintw(y + i, x, "%s", it->name);

		if (list->selected_idx >= 0 && i == list->selected_idx)
			attroff(COLOR_PAIR(3));
		else if (it->style == STYLE_DIR)
			attroff(COLOR_PAIR(2) | A_BOLD);
	}
}

static void ui_render(struct ui *ui)
{
	int col_w;

	col_w = COLS / 3;

	draw_border(3, 1, COLS, LINES - 1);
	list_draw(&ui->cwd_list, col_w + 1, 2, col_w - 1, LINES - 3);
	list_draw(&ui->child_list, 2 * COLS / 3 + 1, 2,
		  COLS - 1 - (2 * COLS / 3 + 1), LINES - 3);

	attron(COLOR_PAIR(2) | A_BOLD);
	mvprintw(0, 0, " %s ", ui->cwd_path);
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

static void term_cleanup(void)
{
	endwin();
}

int main(void)
{
	struct ui *ui;
	int key;

	ui = malloc(sizeof(*ui));
	if (!ui)
		return 1;

	term_init();
	ui_init(ui);

	key = ERR;
	do {
		erase();

		/* handle keys */
		switch (key) {
		case 'j':
			list_move(&ui->cwd_list, 1);
			ui_update_child(ui);
			break;
		case 'k':
			list_move(&ui->cwd_list, -1);
			ui_update_child(ui);
			break;
		}

		ui_render(ui);

		refresh();
		napms(16);
		key = getch();
	} while (key != 'q');

	term_cleanup();
	free(ui);

	return 0;
}
