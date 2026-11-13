package dev.bluepitaya.filemagik.terminal;

public final class Canvas {

    private final FrameBuffer frameBuffer;
    private final int left;
    private final int top;
    private final int width;
    private final int height;

    Canvas(FrameBuffer frameBuffer, int left, int top, int width, int height) {
        this.frameBuffer = frameBuffer;
        this.left = left;
        this.top = top;
        this.width = Math.max(0, width);
        this.height = Math.max(0, height);
    }

    public int width() {
        return width;
    }

    public int height() {
        return height;
    }

    public Canvas sub(int x, int y, int width, int height) {
        int fromX = clamp(x, 0, this.width);
        int fromY = clamp(y, 0, this.height);
        int toX = clamp(x + width, fromX, this.width);
        int toY = clamp(y + height, fromY, this.height);
        return new Canvas(frameBuffer, left + fromX, top + fromY, toX - fromX, toY - fromY);
    }

    public int put(int x, int y, String text, Style style) {
        if (x >= width || y < 0 || y >= height) {
            return 0;
        }

        String clean = Text.sanitize(text);
        int column = x;
        int index = 0;
        while (index < clean.length() && column < width) {
            int codePoint = clean.codePointAt(index);
            index += Character.charCount(codePoint);
            int charWidth = Text.charWidth(codePoint);
            if (charWidth == 0) {
                continue;
            }
            if (charWidth == 2) {
                putWide(column, y, codePoint, style);
            } else {
                frameBuffer.setCell(left + column, top + y, codePoint, style);
            }
            column += charWidth;
        }
        return Math.min(column, width) - x;
    }

    public void fillRow(int y, Style style) {
        if (y < 0 || y >= height) {
            return;
        }

        for (int x = 0; x < width; x++) {
            frameBuffer.setCell(left + x, top + y, ' ', style);
        }
    }

    private void putWide(int column, int y, int codePoint, Style style) {
        if (column + 1 < width) {
            frameBuffer.setWideCell(left + column, top + y, codePoint, style);
        } else {
            frameBuffer.setCell(left + column, top + y, ' ', style);
        }
    }

    private static int clamp(int value, int min, int max) {
        return Math.max(min, Math.min(value, max));
    }
}
