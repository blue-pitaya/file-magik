package dev.bluepitaya.filemagik.preview;

import dev.bluepitaya.filemagik.service.DirectoryListing;
import dev.bluepitaya.filemagik.service.FileEntry;

public interface PreviewListener {

    void previewCleared();

    void directoryPreviewed(DirectoryListing listing);

    void filePreviewed(FileEntry entry);

    void previewFailed(Exception e);
}
