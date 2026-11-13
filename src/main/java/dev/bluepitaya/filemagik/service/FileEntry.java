package dev.bluepitaya.filemagik.service;

import java.nio.file.Path;

public record FileEntry(Path path, String name, Type type) {

    public enum Type {
        DIRECTORY,
        FILE,
        SYMLINK_DIRECTORY,
        SYMLINK_FILE,
        BROKEN_SYMLINK,
        OTHER
    }

    public boolean isDirectory() {
        return type == Type.DIRECTORY || type == Type.SYMLINK_DIRECTORY;
    }
}
