package dev.bluepitaya.filemagik.preview;

import dev.bluepitaya.filemagik.explorer.FileListRenderer;
import dev.bluepitaya.filemagik.service.DirectoryListing;
import dev.bluepitaya.filemagik.service.FileEntry;
import dev.bluepitaya.filemagik.terminal.Canvas;
import dev.bluepitaya.filemagik.terminal.Color;
import dev.bluepitaya.filemagik.terminal.Style;
import dev.bluepitaya.filemagik.terminal.Text;
import dev.bluepitaya.filemagik.ui.View;

public final class PreviewView implements View {

    private static final Style ERROR_STYLE = Style.DEFAULT.fg(Color.RED);

    private DirectoryListing listing;
    private FileEntry file;
    private String error;

    public PreviewView(PreviewHandler handler) {
        handler.listener(new PreviewListener() {
            @Override
            public void previewCleared() {
                clear();
            }

            @Override
            public void directoryPreviewed(DirectoryListing directoryListing) {
                clear();
                listing = directoryListing;
            }

            @Override
            public void filePreviewed(FileEntry entry) {
                clear();
                file = entry;
            }

            @Override
            public void previewFailed(Exception e) {
                clear();
                error = errorText(e);
            }
        });
    }

    @Override
    public void render(Canvas canvas) {
        if (canvas.width() <= 0 || canvas.height() <= 0) {
            return;
        }

        if (listing != null) {
            FileListRenderer.render(canvas, listing.entries(), -1);
            return;
        }

        if (error != null) {
            putLine(canvas, error, ERROR_STYLE);
            return;
        }

        if (file != null) {
            putLine(canvas, file.name(), Style.DEFAULT);
        }
    }

    private void clear() {
        listing = null;
        file = null;
        error = null;
    }

    private static void putLine(Canvas canvas, String text, Style style) {
        int width = canvas.width() - 1;
        if (width <= 0) {
            return;
        }
        canvas.put(1, 0, Text.truncate(Text.sanitize(text), width), style);
    }

    private static String errorText(Exception e) {
        String name = e.getClass().getSimpleName();
        String text = e.getMessage();
        return text == null || text.isEmpty() ? name : name + ": " + text;
    }
}
