package dev.bluepitaya.filemagik;

import dev.bluepitaya.filemagik.explorer.ExplorerHandler;
import dev.bluepitaya.filemagik.explorer.ExplorerView;
import dev.bluepitaya.filemagik.terminal.FrameBuffer;
import dev.bluepitaya.filemagik.terminal.Terminal;
import dev.bluepitaya.filemagik.terminal.Terminal.TerminalSize;
import dev.bluepitaya.filemagik.ui.EventLoop;
import sun.misc.Signal;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.Path;

final class AppRunner {

    private final Terminal terminal;
    private final EventLoop eventLoop;
    private final ExplorerHandler explorerHandler;
    private final FrameBuffer frameBuffer;
    private final ExplorerView explorerView;

    public AppRunner(Terminal terminal, EventLoop eventLoop, ExplorerHandler explorerHandler, FrameBuffer frameBuffer, ExplorerView explorerView) {
        this.terminal = terminal;
        this.eventLoop = eventLoop;
        this.explorerHandler = explorerHandler;
        this.frameBuffer = frameBuffer;
        this.explorerView = explorerView;
    }

    public void run(Path directory) throws IOException {
        terminal.start();
        try {
            updateSize();
            eventLoop.afterEach(this::render);
            eventLoop.invokeLater(() -> explorerHandler.open(directory));
            Signal.handle(new Signal("WINCH"), signal -> eventLoop.invokeLater(this::updateSize));
            Thread inputReader = new Thread(this::readInput, "input-reader");
            inputReader.setDaemon(true);
            inputReader.start();
            eventLoop.run();
        } finally {
            terminal.stop();
        }
    }

    private void readInput() {
        byte[] buffer = new byte[64];
        try {
            while (true) {
                int count = terminal.read(buffer, 0);
                if (count < 0) {
                    eventLoop.stop();
                    return;
                }
                for (int i = 0; i < count; i++) {
                    byte key = buffer[i];
                    eventLoop.invokeLater(() -> handleKey(key));
                }
            }
        } catch (IOException e) {
            eventLoop.invokeLater(() -> {
                throw new UncheckedIOException(e);
            });
        }
    }

    private void handleKey(byte key) {
        switch ((char) key) {
            case 'j' -> explorerHandler.moveCursor(1);
            case 'k' -> explorerHandler.moveCursor(-1);
            case 'h' -> explorerHandler.goParent();
            case 'l' -> explorerHandler.enterSelected();
            case 'q' -> eventLoop.stop();
            default -> {
            }
        }
    }

    private void updateSize() {
        try {
            TerminalSize size = terminal.size();
            frameBuffer.resize(size.width(), size.height());
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private void render() {
        frameBuffer.clear();
        explorerView.render(frameBuffer.canvas());
        try {
            frameBuffer.flush();
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }
}
