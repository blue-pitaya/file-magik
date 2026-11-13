package dev.bluepitaya.filemagik.service;

import java.nio.file.Path;
import java.util.List;

public record DirectoryListing(Path directory, List<FileEntry> entries) {

    public DirectoryListing {
        entries = List.copyOf(entries);
    }

    public int indexOf(Path path) {
        for (int i = 0; i < entries.size(); i++) {
            if (entries.get(i).path().equals(path)) {
                return i;
            }
        }
        return -1;
    }
}
