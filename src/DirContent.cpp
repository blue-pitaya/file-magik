#include "DirContent.h"
#include "DirListItem.h"
#include "FileListItem.h"
#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <utility>

DirContent::DirContent(const filesystem::path &path) : path(path) {
}

void DirContent::load() {
    this->items.clear();

    DIR *d;
    if ((d = opendir(path.c_str())) == NULL) {
        throw std::runtime_error("Cannot open current directory.");
    }

    struct dirent *dir;
    int style = 0;
    while ((dir = readdir(d)) != NULL) {
        if (std::strcmp(dir->d_name, ".") == 0) {
            continue;
        }
        if (std::strcmp(dir->d_name, "..") == 0) {
            continue;
        }

        switch (dir->d_type) {
        case DT_DIR: {
            auto item = DirListItem();
            item.name = dir->d_name;
            this->items.push_back(std::move(item));
            break;
        }
        default:
            auto item = FileListItem();
            item.name = dir->d_name;
            this->items.push_back(std::move(item));
            break;
        }
    }
    closedir(d);

    // FIXME:
    //  list.sort();
}
