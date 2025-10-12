#ifndef FILE_MAGIK_FILE_LIST
#define FILE_MAGIK_FILE_LIST

#include "core.h"
#include <dirent.h>

// TODO: support unlimited files via list or some shit
enum { FileList_MAX_FILES = 1024 };

typedef struct {
    char *file_name;
    unsigned int file_type;
} FileListItem;

typedef struct {
    FileListItem items[FileList_MAX_FILES];
    int size;
    int selected_idx;
} FileList;

void FileList_append_file(FileList *, struct dirent *);
void FileList_change_selected_idx(FileList *, int);
Err FileList_load(FileList *);

#endif
