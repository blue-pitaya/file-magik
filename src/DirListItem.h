#ifndef filemagik_DirListItem
#define filemagik_DirListItem

#include "IDirContentItem.h"
#include <string>

using namespace std;

class DirListItem : public IDirContentItem {
  public:
    string name = "";
};

#endif
