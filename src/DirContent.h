#ifndef filemagik_DirContent
#define filemagik_DirContent

#include "IDirContentItem.h"
#include <filesystem>
#include <vector>

using namespace std;

class DirContent {
  public:
    filesystem::path path;
    vector<IDirContentItem> items;

    DirContent(const filesystem::path &path);
    void load();
};

#endif
