package dev.bluepitaya.filemagik.ui;

import dev.bluepitaya.filemagik.terminal.Canvas;
import dev.bluepitaya.filemagik.terminal.Color;
import dev.bluepitaya.filemagik.terminal.Style;

public final class StatusBar implements View {

    private static final Style ERROR = Style.DEFAULT.fg(Color.RED);

    private String error;

    public void showError(String message) {
        error = message;
    }

    public void clear() {
        error = null;
    }

    @Override
    public void render(Canvas canvas) {
        if (error != null) {
            canvas.put(0, 0, error, ERROR);
        }
    }
}
