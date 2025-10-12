#include "FileList.h"
#include "core.h"
#include "glib.h"
#include <unistd.h>

int _strcmp(const void *p1, const void *p2) {
    FileListItem *f1 = p1;
    FileListItem *f2 = p2;

    return strcmp(f1->file_name, f2->file_name);
}
void FileList_append_file(FileList *f, struct dirent *dir) {
    f->items[f->size].file_name = dir->d_name;
    f->items[f->size].file_type = dir->d_type;
    f->size++;
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

Err FileList_load(FileList *f) {
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
        FileList_append_file(f, dir);
    }
    closedir(d);

    if (f->size > 0) {
        f->selected_idx = 0;
    }

    qsort(f->items, f->size, sizeof(FileListItem), _strcmp);

    return ERR_OK;
}
