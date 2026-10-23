package dev.bluepitaya.filemagik.terminal;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.atomic.AtomicBoolean;

public final class Terminal {

    public record TerminalSize(int width, int height) {
    }

    private static final String ENTER_SCREEN = "\u001b[?1049h\u001b[?25l";
    private static final String LEAVE_SCREEN = "\u001b[0m\u001b[?25h\u001b[?1049l";

    private final AtomicBoolean redrawRequest = new AtomicBoolean();

    private InputStream input;
    private Writer output;
    private String savedSettings;
    private boolean started;
    private boolean active;
    private volatile boolean suspended;

    public synchronized void start() throws IOException {
        input = Tty.openInput();
        output = new OutputStreamWriter(Tty.openOutput(), StandardCharsets.UTF_8);
        // save terminal state before continuing
        savedSettings = Stty.run("-g").trim();
        Runtime.getRuntime().addShutdownHook(new Thread(this::stop, "terminal-restore")); //
        enter();
        started = true;
    }

    public synchronized void stop() {
        started = false;
        suspended = false;
        if (!active) {
            return;
        }

        active = false;
        try {
            leave();
        } catch (IOException | RuntimeException ignored) {
        }
    }

    public TerminalSize size() throws IOException {
        return Stty.size();
    }

    public int read(byte[] buffer, int offset) throws IOException {
        if (suspended || offset >= buffer.length) {
            return 0;
        }

        return input.read(buffer, offset, buffer.length - offset);
    }

    public synchronized void write(String text) throws IOException {
        output.write(text);
    }

    public synchronized void flush() throws IOException {
        output.flush();
    }

    public boolean takeRedrawRequest() {
        return redrawRequest.getAndSet(false);
    }

    private void enter() throws IOException {
        Stty.run("raw", "-echo");
        active = true;
        output.write(ENTER_SCREEN);
        output.flush();
        redrawRequest.set(true);
    }

    private void leave() throws IOException {
        try {
            output.write(LEAVE_SCREEN);
            output.flush();
        } finally {
            Stty.run(savedSettings);
        }
    }
}

