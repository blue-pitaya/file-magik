package dev.bluepitaya.filemagik.explorer;

import java.io.IOException;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;
import java.util.function.Consumer;

import dev.bluepitaya.filemagik.service.DirectoryListing;
import dev.bluepitaya.filemagik.service.FileEntry;
import dev.bluepitaya.filemagik.service.FileService;

public final class ExplorerHandler {

    private final FileService fileService;
    private final Consumer<FileEntry> onCurrentEntryChanged;
    private final Map<Path, Path> selectedPaths = new HashMap<>();

    private ExplorerListener listener;
    private DirectoryListing current;
    private DirectoryListing parent;
    private int parentCursor = -1;
    private int cursor;

    public ExplorerHandler(FileService fileService, Consumer<FileEntry> onCurrentEntryChanged) {
        this.fileService = fileService;
        this.onCurrentEntryChanged = onCurrentEntryChanged;
    }

    public void listener(ExplorerListener listener) {
        this.listener = listener;
    }

    public void open(Path directory) {
        Path dir = directory.toAbsolutePath().normalize();
        if (current == null) {
            loadNearest(dir);
            notifyListing();
            return;
        }

        if (load(dir)) {
            notifyListing();
        }
    }

    public void moveCursor(int delta) {
        int clamped = clamp(cursor + delta);
        if (clamped == cursor) {
            return;
        }

        cursor = clamped;
        rememberSelection();
        listener.cursorChanged(cursor);
        notifyEntry();
    }

    public void enterSelected() {
        FileEntry entry = currentEntry();
        if (entry != null && entry.isDirectory()) {
            open(entry.path());
        }
    }

    public void goParent() {
        Path parentDir = current.directory().getParent();
        if (parentDir != null) {
            open(parentDir);
        }
    }

    private FileEntry currentEntry() {
        return current.entries().isEmpty() ? null : current.entries().get(cursor);
    }

    private void loadNearest(Path dir) {
        Path directory = dir;
        while (directory != null && !load(directory)) {
            directory = directory.getParent();
        }
    }

    private boolean load(Path dir) {
        DirectoryListing listing;
        try {
            listing = fileService.listing(dir);
        } catch (IOException e) {
            return false;
        }

        Path remembered = selectedPaths.get(listing.directory());
        int index = remembered == null ? -1 : listing.indexOf(remembered);
        current = listing;
        cursor = clamp(Math.max(index, 0));
        rememberSelection();
        loadParent();
        return true;
    }

    private void loadParent() {
        Path parentDir = current.directory().getParent();
        if (parentDir == null) {
            parent = null;
            parentCursor = -1;
            return;
        }

        try {
            parent = fileService.listing(parentDir);
            parentCursor = parent.indexOf(current.directory());
            selectedPaths.put(parentDir, current.directory());
        } catch (IOException e) {
            parent = null;
            parentCursor = -1;
        }
    }

    private void rememberSelection() {
        FileEntry entry = currentEntry();
        if (entry != null) {
            selectedPaths.put(current.directory(), entry.path());
        }
    }

    private int clamp(int index) {
        int size = current.entries().size();
        if (size == 0) {
            return 0;
        }
        return Math.max(0, Math.min(index, size - 1));
    }

    private void notifyListing() {
        listener.listingChanged(parent, parentCursor, current);
        listener.cursorChanged(cursor);
        notifyEntry();
    }

    private void notifyEntry() {
        onCurrentEntryChanged.accept(currentEntry());
    }
}
