#include <dirent.h>
#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MAX_NAME 256
#define MAX_ITEMS 2048
#define MAX_CWD 4096

#define STYLE_FILE 0
#define STYLE_DIR 1

struct vec2d {
	int x;
	int y;
};

struct list_item {
	char name[MAX_NAME];
	int style;
};

struct list {
	struct list_item items[MAX_ITEMS];
	int count;
	struct vec2d position;
	struct vec2d size;
	int selected_idx;
	int show_selection;
};

struct ui {
	struct list parent;
	struct list cwd;
	struct list child;
	char cwd_path[MAX_CWD];
	struct vec2d term;
};

/* List operations */

static void list_init(struct list *l, int show_selection)
{
	memset(l, 0, sizeof(*l));
	l->selected_idx = -1;
	l->show_selection = show_selection;
}

static void list_clear(struct list *l)
{
	l->count = 0;
	l->selected_idx = -1;
}

static int list_append(struct list *l, const char *name, int style)
{
	struct list_item *it;

	if (l->count >= MAX_ITEMS)
		return -1;

	it = &l->items[l->count++];
	strncpy(it->name, name, MAX_NAME - 1);
	it->name[MAX_NAME - 1] = '\0';
	it->style = style;

	if (l->selected_idx < 0)
		l->selected_idx = 0;

	return 0;
}

static void list_move(struct list *l, int n)
{
	int r;

	if (l->count <= 0)
		return;

	r = l->selected_idx + n;

	if (r < 0)
		l->selected_idx = 0;
	else if (r >= l->size.y)
		l->selected_idx = l->size.y - 1;
	else if (r >= l->count)
		l->selected_idx = l->count - 1;
	else
		l->selected_idx = r;
}

static int item_cmp(const void *a, const void *b)
{
	return strcmp(((const struct list_item *)a)->name,
		      ((const struct list_item *)b)->name);
}

static void list_sort(struct list *l)
{
	qsort(l->items, l->count, sizeof(struct list_item), item_cmp);
}

static struct list_item *list_selected(struct list *l)
{
	if (l->selected_idx >= 0 && l->selected_idx < l->count)
		return &l->items[l->selected_idx];

	return NULL;
}

static void list_draw(struct list *l)
{
	struct list_item *it;
	int y;
	int x;
	int i;

	y = l->position.y;
	x = l->position.x;

	for (i = 0; i < l->count; i++, y++) {
		it = &l->items[i];

		if (l->show_selection && i == l->selected_idx) {
			attron(COLOR_PAIR(3));
			mvprintw(y, x, "%s", it->name);
			attroff(COLOR_PAIR(3));
		} else if (it->style == STYLE_DIR) {
			attron(COLOR_PAIR(2) | A_BOLD);
			mvprintw(y, x, "%s", it->name);
			attroff(COLOR_PAIR(2) | A_BOLD);
		} else {
			mvprintw(y, x, "%s", it->name);
		}
	}
}

/* Directory loading */

static void load_dir(struct list *l, const char *path)
{
	struct dirent *e;
	DIR *d;
	int style;

	list_clear(l);

	d = opendir(path);
	if (!d)
		return;

	while ((e = readdir(d))) {
		if (e->d_name[0] == '.' &&
		    (e->d_name[1] == '\0' ||
		     (e->d_name[1] == '.' && e->d_name[2] == '\0')))
			continue;

		style = (e->d_type == DT_DIR) ? STYLE_DIR : STYLE_FILE;
		list_append(l, e->d_name, style);
	}

	closedir(d);
	list_sort(l);
}

/* UI */

static void ui_layout(struct ui *ui)
{
	int cx;
	int cy;
	int cw;
	int ch;
	int px;

	cx = 1;
	cy = 2;
	cw = ui->term.x - 2;
	ch = ui->term.y - 3;
	px = (cw - 2) / 3;

	ui->parent.position.x = cx;
	ui->parent.position.y = cy;
	ui->parent.size.x = px;
	ui->parent.size.y = ch;

	ui->cwd.position.x = cx + px + 1;
	ui->cwd.position.y = cy;
	ui->cwd.size.x = px;
	ui->cwd.size.y = ch;

	ui->child.position.x = cx + px * 2 + 2;
	ui->child.position.y = cy;
	ui->child.size.x = px;
	ui->child.size.y = ch;
}

static void ui_init(struct ui *ui)
{
	struct winsize w;

	list_init(&ui->parent, 0);
	list_init(&ui->cwd, 1);
	list_init(&ui->child, 0);

	if (!getcwd(ui->cwd_path, MAX_CWD))
		strcpy(ui->cwd_path, ".");

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
		ui->term.x = 80;
		ui->term.y = 24;
	} else {
		ui->term.x = w.ws_col;
		ui->term.y = w.ws_row;
	}

	load_dir(&ui->cwd, ".");
	load_dir(&ui->parent, "..");
	ui_layout(ui);
}

static void ui_update_child(struct ui *ui)
{
	struct list_item *it;
	char path[MAX_NAME + 3];

	it = list_selected(&ui->cwd);
	if (it && it->style == STYLE_DIR) {
		snprintf(path, sizeof(path), "./%s", it->name);
		load_dir(&ui->child, path);
	} else {
		list_clear(&ui->child);
	}
}

static void ui_handle_key(struct ui *ui, int key)
{
	int prev;

	prev = ui->cwd.selected_idx;

	switch (key) {
	case 'j':
		list_move(&ui->cwd, 1);
		break;
	case 'k':
		list_move(&ui->cwd, -1);
		break;
	case 'h':
		break;
	default:
		return;
	}

	if (prev != ui->cwd.selected_idx)
		ui_update_child(ui);
}

static void ui_render_border(struct ui *ui)
{
	int w;
	int h;
	int s1;
	int s2;
	int i;

	w = ui->term.x;
	h = ui->term.y;
	s1 = w / 3;
	s2 = 2 * w / 3;

	for (i = 0; i < w; i++) {
		mvprintw(1, i, "\u2500");
		mvprintw(h - 1, i, "\u2500");
	}

	for (i = 1; i < h; i++) {
		mvprintw(i, 0, "\u2502");
		mvprintw(i, s1, "\u2502");
		mvprintw(i, s2, "\u2502");
		mvprintw(i, w - 1, "\u2502");
	}

	mvprintw(1, 0, "\u250C");
	mvprintw(1, s1, "\u252C");
	mvprintw(h - 1, s1, "\u2534");
	mvprintw(1, s2, "\u252C");
	mvprintw(h - 1, s2, "\u2534");
	mvprintw(1, w - 1, "\u2510");
	mvprintw(h - 1, 0, "\u2514");
	mvprintw(h - 1, w - 1, "\u2518");
}

static void ui_render(struct ui *ui)
{
	ui_render_border(ui);

	attron(COLOR_PAIR(2) | A_BOLD);
	mvprintw(0, 0, " %s ", ui->cwd_path);
	attroff(COLOR_PAIR(2) | A_BOLD);

	list_draw(&ui->parent);
	list_draw(&ui->cwd);
	list_draw(&ui->child);
}

/* Entry point */

int main(void)
{
	struct ui ui;
	int key;

	setlocale(LC_ALL, "");
	initscr();
	curs_set(0);
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	nodelay(stdscr, TRUE);

	start_color();
	use_default_colors();
	init_pair(1, COLOR_WHITE, 16);
	init_pair(2, COLOR_BLUE, 16);
	init_pair(3, 16, COLOR_WHITE);

	ui_init(&ui);

	key = -1;
	do {
		erase();
		ui_handle_key(&ui, key);
		ui_render(&ui);
		refresh();
		napms(16);
		key = getch();
	} while (key != 'q');

	endwin();
	return 0;
}
