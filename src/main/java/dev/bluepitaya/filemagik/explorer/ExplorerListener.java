package dev.bluepitaya.filemagik.explorer;

import dev.bluepitaya.filemagik.service.DirectoryListing;

public interface ExplorerListener {

    void listingChanged(DirectoryListing parent, int parentCursor, DirectoryListing current);

    void cursorChanged(int cursor);
}
