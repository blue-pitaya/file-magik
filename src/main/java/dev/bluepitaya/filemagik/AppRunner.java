package dev.bluepitaya.filemagik;

import dev.bluepitaya.filemagik.explorer.ExplorerHandler;
import dev.bluepitaya.filemagik.explorer.ExplorerView;
import dev.bluepitaya.filemagik.terminal.Canvas;
import dev.bluepitaya.filemagik.terminal.FrameBuffer;
import dev.bluepitaya.filemagik.terminal.Terminal;
import dev.bluepitaya.filemagik.terminal.Terminal.TerminalSize;
import dev.bluepitaya.filemagik.ui.AppCommandException;
import dev.bluepitaya.filemagik.ui.EventLoop;
import dev.bluepitaya.filemagik.ui.StatusBar;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.Path;

final class AppRunner {

    private final Terminal terminal;
    private final EventLoop eventLoop;
    private final ExplorerHandler explorerHandler;
    private final FrameBuffer frameBuffer;
    private final ExplorerView explorerView;
    private final StatusBar statusBar;

    public AppRunner(Terminal terminal, EventLoop eventLoop, ExplorerHandler explorerHandler, FrameBuffer frameBuffer,
                     ExplorerView explorerView, StatusBar statusBar) {
        this.terminal = terminal;
        this.eventLoop = eventLoop;
        this.explorerHandler = explorerHandler;
        this.frameBuffer = frameBuffer;
        this.explorerView = explorerView;
        this.statusBar = statusBar;
    }

    public void run(Path directory) throws IOException {
        terminal.start();
        try {
            updateSize();
            eventLoop.afterEach(this::render);
            eventLoop.invokeLater(() -> explorerHandler.open(directory));
            terminal.onResize(() -> eventLoop.invokeLater(this::updateSize));
            Thread inputReader = new Thread(this::readInput, "input-reader");
            inputReader.setDaemon(true);
            inputReader.start();
            eventLoop.run();
        } finally {
            terminal.stop();
        }
    }

    private void readInput() {
        try {
            while (true) {
                int c = terminal.read(100);
                if (c == Terminal.EOF) {
                    eventLoop.stop();
                    return;
                }
                if (c == Terminal.TIMEOUT) {
                    continue;
                }
                char key = (char) c;
                eventLoop.invokeLater(() -> handleKey(key));
            }
        } catch (IOException e) {
            eventLoop.invokeLater(() -> {
                throw new UncheckedIOException(e);
            });
        }
    }

    private void handleKey(char key) {
        statusBar.clear(); // an error stays visible until the next keypress
        try {
            switch (key) {
                case 'j' -> explorerHandler.moveCursor(1);
                case 'k' -> explorerHandler.moveCursor(-1);
                case 'h' -> explorerHandler.goParent();
                case 'l' -> explorerHandler.enterSelected();
                case 'q' -> eventLoop.stop();
                default -> {
                }
            }
        } catch (AppCommandException e) {
            statusBar.showError(e.getMessage());
        }
    }

    private void updateSize() {
        TerminalSize size = terminal.size();
        frameBuffer.resize(size.width(), size.height());
    }

    private void render() {
        frameBuffer.clear();
        Canvas canvas = frameBuffer.canvas();
        int width = canvas.width();
        int height = canvas.height();
        explorerView.render(canvas.sub(0, 0, width, height - 1));
        statusBar.render(canvas.sub(0, height - 1, width, 1));
        try {
            frameBuffer.flush();
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }
}
