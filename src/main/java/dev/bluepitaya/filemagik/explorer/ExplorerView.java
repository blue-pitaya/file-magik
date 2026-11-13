package dev.bluepitaya.filemagik.explorer;

import dev.bluepitaya.filemagik.preview.PreviewView;
import dev.bluepitaya.filemagik.service.DirectoryListing;
import dev.bluepitaya.filemagik.terminal.Canvas;
import dev.bluepitaya.filemagik.ui.View;

public final class ExplorerView implements View {

    private static final int[] COLUMN_RATIOS = {1, 3, 4};

    private final PreviewView previewView;

    private DirectoryListing parent;
    private int parentCursor = -1;
    private DirectoryListing current;
    private int cursor;

    public ExplorerView(ExplorerHandler handler, PreviewView previewView) {
        this.previewView = previewView;

        handler.listener(new ExplorerListener() {
            @Override
            public void listingChanged(DirectoryListing parentListing, int parentListingCursor, DirectoryListing currentListing) {
                parent = parentListing;
                parentCursor = parentListingCursor;
                current = currentListing;
            }

            @Override
            public void cursorChanged(int newCursor) {
                cursor = newCursor;
            }
        });
    }

    @Override
    public void render(Canvas canvas) {
        int height = canvas.height();
        int total = canvas.width() - 2;
        if (height <= 0 || total <= 0) {
            return;
        }

        int[] widths = columnWidths(total);
        int x = 0;
        if (widths[0] > 0) {
            if (parent != null) {
                FileListRenderer.render(canvas.sub(x, 0, widths[0], height), parent.entries(), parentCursor);
            }
            x += widths[0] + 1;
        }
        if (widths[1] > 0) {
            FileListRenderer.render(canvas.sub(x, 0, widths[1], height), current.entries(), cursor);
            x += widths[1] + 1;
        }
        if (widths[2] > 0) {
            previewView.render(canvas.sub(x, 0, widths[2], height));
        }
    }

    private static int[] columnWidths(int total) {
        int sum = COLUMN_RATIOS[0] + COLUMN_RATIOS[1] + COLUMN_RATIOS[2];
        int[] widths = new int[3];
        widths[0] = total * COLUMN_RATIOS[0] / sum;
        widths[1] = total * COLUMN_RATIOS[1] / sum;
        widths[2] = total - widths[0] - widths[1];
        return widths;
    }
}
