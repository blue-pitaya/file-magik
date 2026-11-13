package dev.bluepitaya.filemagik.preview;

import java.io.IOException;

import dev.bluepitaya.filemagik.service.FileEntry;
import dev.bluepitaya.filemagik.service.FileService;

public final class PreviewHandler {

    private final FileService fileService;

    private PreviewListener listener;

    public PreviewHandler(FileService fileService) {
        this.fileService = fileService;
    }

    public void listener(PreviewListener listener) {
        this.listener = listener;
    }

    public void show(FileEntry entry) {
        if (entry == null || entry.type() == FileEntry.Type.OTHER) {
            listener.previewCleared();
            return;
        }

        if (entry.isDirectory()) {
            showDirectory(entry);
            return;
        }

        listener.filePreviewed(entry);
    }

    private void showDirectory(FileEntry entry) {
        try {
            listener.directoryPreviewed(fileService.listing(entry.path()));
        } catch (IOException e) {
            listener.previewFailed(e);
        }
    }
}
