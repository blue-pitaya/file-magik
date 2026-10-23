package dev.bluepitaya.filemagik;

import java.io.IOException;
import java.io.UncheckedIOException;

import dev.bluepitaya.filemagik.terminal.Terminal;
import dev.bluepitaya.filemagik.terminal.Terminal.TerminalSize;
import dev.bluepitaya.filemagik.ui.EventLoop;
import sun.misc.Signal;

final class AppScreen {
    private static final String MESSAGE = "hello world";

    private final Terminal terminal;
    private final EventLoop eventLoop;

    private TerminalSize size;

    public AppScreen(Terminal terminal, EventLoop eventLoop) {
        this.terminal = terminal;
        this.eventLoop = eventLoop;
    }

    public void start() throws IOException {
        terminal.start();
        try {
            size = terminal.size();
            eventLoop.afterEach(this::draw);
            Signal.handle(new Signal("WINCH"), signal -> eventLoop.invokeLater(this::updateSize));
            Thread inputReader = new Thread(this::readInput, "input-reader");
            inputReader.setDaemon(true);
            inputReader.start();
            draw();
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
        if (key == 'q') {
            eventLoop.stop();
        }
    }

    private void updateSize() {
        try {
            size = terminal.size();
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private void draw() {
        try {
            int row = (size.height() + 1) / 2;
            int column = Math.max(1, (size.width() - MESSAGE.length()) / 2 + 1);
            terminal.write("\u001b[2J\u001b[" + row + ";" + column + "H" + MESSAGE);
            terminal.flush();
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }
}
