package dev.bluepitaya.filemagik.terminal;

import java.io.IOException;
import java.util.Arrays;

public final class FrameBuffer {

    static final int CONTINUATION = -1;

    private static final String CSI = "\u001b[";

    private final Terminal terminal;

    private int width;
    private int height;
    private int[] codePoints = new int[0];
    private Style[] styles = new Style[0];
    private int[] frontCodePoints = new int[0];
    private Style[] frontStyles = new Style[0];
    private boolean fullRedraw = true;

    public FrameBuffer(Terminal terminal) {
        this.terminal = terminal;
    }

    public void resize(int width, int height) {
        int newWidth = Math.max(0, width);
        int newHeight = Math.max(0, height);
        if (newWidth == this.width && newHeight == this.height) {
            return;
        }

        this.width = newWidth;
        this.height = newHeight;
        int size = newWidth * newHeight;
        codePoints = new int[size];
        styles = new Style[size];
        frontCodePoints = new int[size];
        frontStyles = new Style[size];
        Arrays.fill(codePoints, ' ');
        Arrays.fill(styles, Style.DEFAULT);
        Arrays.fill(frontCodePoints, ' ');
        Arrays.fill(frontStyles, Style.DEFAULT);
        fullRedraw = true;
    }

    public int width() {
        return width;
    }

    public int height() {
        return height;
    }

    public Canvas canvas() {
        return new Canvas(this, 0, 0, width, height);
    }

    public void clear() {
        Arrays.fill(codePoints, ' ');
        Arrays.fill(styles, Style.DEFAULT);
    }

    public void flush() throws IOException {
        boolean full = terminal.takeRedrawRequest() || fullRedraw;
        fullRedraw = false;

        StringBuilder out = new StringBuilder();
        if (full) {
            out.append(CSI).append("0m").append(CSI).append("2J");
        }

        Style current = null;
        int columnAfterLast = -1;
        int rowOfLast = -1;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = y * width + x;
                int codePoint = codePoints[index];
                if (codePoint == CONTINUATION) {
                    continue;
                }

                boolean wide = x + 1 < width && codePoints[index + 1] == CONTINUATION;
                if (!full && !isChanged(index, wide)) {
                    continue;
                }

                if (x != columnAfterLast || y != rowOfLast) {
                    appendMove(out, x, y);
                }
                Style style = styles[index];
                if (!style.equals(current)) {
                    out.append(style.toSgr());
                    current = style;
                }
                out.appendCodePoint(codePoint);

                frontCodePoints[index] = codePoint;
                frontStyles[index] = style;
                if (wide) {
                    frontCodePoints[index + 1] = CONTINUATION;
                    frontStyles[index + 1] = styles[index + 1];
                }
                columnAfterLast = x + (wide ? 2 : 1);
                rowOfLast = y;
            }
        }

        out.append(CSI).append("0m");
        terminal.write(out.toString());
        terminal.flush();
    }

    void setCell(int x, int y, int codePoint, Style style) {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            return;
        }

        breakWideAt(x, y);
        int index = y * width + x;
        codePoints[index] = codePoint;
        styles[index] = style;
    }

    void setWideCell(int x, int y, int codePoint, Style style) {
        if (x < 0 || y < 0 || x + 1 >= width || y >= height) {
            return;
        }

        breakWideAt(x, y);
        breakWideAt(x + 1, y);
        int index = y * width + x;
        codePoints[index] = codePoint;
        styles[index] = style;
        codePoints[index + 1] = CONTINUATION;
        styles[index + 1] = style;
    }

    private void breakWideAt(int x, int y) {
        int index = y * width + x;
        if (codePoints[index] == CONTINUATION) {
            if (x > 0) {
                codePoints[index - 1] = ' ';
            }
            return;
        }
        if (x + 1 < width && codePoints[index + 1] == CONTINUATION) {
            codePoints[index + 1] = ' ';
        }
    }

    private boolean isChanged(int index, boolean wide) {
        if (codePoints[index] != frontCodePoints[index] || !styles[index].equals(frontStyles[index])) {
            return true;
        }
        return wide && frontCodePoints[index + 1] != CONTINUATION;
    }

    private static void appendMove(StringBuilder out, int x, int y) {
        out.append(CSI).append(y + 1).append(';').append(x + 1).append('H');
    }
}
