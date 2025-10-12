#ifndef CORE
#define CORE

typedef enum {
    ERR_OK = 0,
    ERR_FAIL = 1,
    ERR_EXIT = 2,
} Err;

enum { FileList_MAX_FILES = 1024 };
typedef struct {
    char *files[FileList_MAX_FILES];
    int size;
    int selected_idx;
} FileList;

typedef struct {
    FileList list;
    char *cwd;
    int terminal_w;
    int terminal_h;
} UI;
Err UI_init(UI *);
Err UI_handle_key(UI *, int key);
void UI_render(UI *);

#endif
