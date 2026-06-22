#ifndef filemagik_FileListItem
#define filemagik_FileListItem

#include "IDirContentItem.h"
#include <string>

using namespace std;

class FileListItem : public IDirContentItem {
  public:
    string name = "";
};

#endif
