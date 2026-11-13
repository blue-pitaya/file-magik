package dev.bluepitaya.filemagik.explorer;

import java.util.List;

import dev.bluepitaya.filemagik.service.FileEntry;
import dev.bluepitaya.filemagik.terminal.Canvas;
import dev.bluepitaya.filemagik.terminal.Color;
import dev.bluepitaya.filemagik.terminal.Style;
import dev.bluepitaya.filemagik.terminal.Text;

public final class FileListRenderer {

    private static final Style EMPTY_STYLE = Style.DEFAULT.fg(Color.BRIGHT_BLACK);

    private FileListRenderer() {
    }

    public static void render(Canvas canvas, List<FileEntry> entries, int cursor) {
        int width = canvas.width();
        int height = canvas.height();
        if (width <= 0 || height <= 0) {
            return;
        }

        if (entries.isEmpty()) {
            canvas.put(1, 0, "empty", EMPTY_STYLE);
            return;
        }

        int offset = Math.max(0, Math.min(cursor - height / 2, entries.size() - height));
        for (int row = 0; row < height && offset + row < entries.size(); row++) {
            int index = offset + row;
            renderRow(canvas, row, entries.get(index), index == cursor);
        }
    }

    private static Style styleOf(FileEntry entry) {
        return switch (entry.type()) {
            case DIRECTORY -> Style.DEFAULT.fg(Color.BLUE).bold();
            case SYMLINK_DIRECTORY, SYMLINK_FILE -> Style.DEFAULT.fg(Color.CYAN);
            case BROKEN_SYMLINK -> Style.DEFAULT.fg(Color.RED);
            case OTHER -> Style.DEFAULT.fg(Color.YELLOW);
            case FILE -> Style.DEFAULT;
        };
    }

    private static void renderRow(Canvas canvas, int row, FileEntry entry, boolean selected) {
        Style style = styleOf(entry);
        if (selected) {
            style = style.reverse();
            canvas.fillRow(row, style);
        }

        int nameWidth = canvas.width() - 1;
        if (nameWidth > 0) {
            canvas.put(1, row, Text.truncate(Text.sanitize(entry.name()), nameWidth), style);
        }
    }
}
