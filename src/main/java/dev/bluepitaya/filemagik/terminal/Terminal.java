package dev.bluepitaya.filemagik.terminal;

import java.io.File;
import java.io.IOException;
import java.io.InterruptedIOException;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

import org.jline.terminal.Attributes;
import org.jline.terminal.Size;
import org.jline.terminal.Terminal.Signal;
import org.jline.terminal.Terminal.SignalHandler;
import org.jline.terminal.TerminalBuilder;
import org.jline.utils.NonBlockingReader;

public final class Terminal {

    public record TerminalSize(int width, int height) {
    }

    public static final int EOF = NonBlockingReader.EOF;
    public static final int TIMEOUT = NonBlockingReader.READ_EXPIRED;

    private static final String ENTER_SCREEN = "\u001b[?1049h\u001b[?25l";
    private static final String LEAVE_SCREEN = "\u001b[0m\u001b[?25h\u001b[?1049l";
    private static final File TTY = new File("/dev/tty");

    private final AtomicBoolean redrawRequest = new AtomicBoolean();
    private final ReentrantLock readLock = new ReentrantLock(true);
    private final Condition resumed = readLock.newCondition();
    private boolean suspended;

    private org.jline.terminal.Terminal jline;
    private Attributes savedAttributes;
    private boolean active;
    private boolean closed;

    public synchronized void start() throws IOException {
        jline = TerminalBuilder.builder()
                .system(true)
                .dumb(false)
                .build();
        savedAttributes = jline.getAttributes();
        Runtime.getRuntime().addShutdownHook(new Thread(this::stop, "terminal-restore"));
        enter();
    }

    public synchronized void stop() {
        if (closed) {
            return;
        }
        closed = true;
        try {
            leave();
            jline.close();
        } catch (IOException | RuntimeException ignored) {
        }
    }

    public TerminalSize size() {
        Size size = jline.getSize();
        return new TerminalSize(size.getColumns(), size.getRows());
    }

    public void onResize(Runnable onResize) {
        jline.handle(Signal.WINCH, signal -> onResize.run());
    }

    public int read(long timeoutMillis) throws IOException {
        readLock.lock();
        try {
            while (suspended) {
                resumed.await();
            }
            return jline.reader().read(timeoutMillis);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new InterruptedIOException("interrupted while terminal was suspended");
        } finally {
            readLock.unlock();
        }
    }

    public int runInForeground(List<String> command) throws IOException {
        setSuspended(true); // waits for an in-flight read(), so the program gets every key
        // Ctrl+C in the program would also send SIGINT to us. A no-op handler (unlike SIG_IGN)
        // is reset to the default on exec, so the program still receives Ctrl+C normally.
        SignalHandler previous = jline.handle(Signal.INT, signal -> {
        });
        try {
            leave();
            Process process = new ProcessBuilder(command)
                    .redirectInput(TTY)
                    .redirectOutput(TTY)
                    .redirectError(TTY)
                    .start();
            return process.waitFor();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new InterruptedIOException(command.getFirst() + " was interrupted");
        } finally {
            jline.handle(Signal.INT, previous);
            try {
                enter();
            } finally {
                setSuspended(false);
            }
        }
    }

    public synchronized void write(String text) {
        jline.writer().write(text);
    }

    public synchronized void flush() {
        jline.writer().flush();
    }

    public boolean takeRedrawRequest() {
        return redrawRequest.getAndSet(false);
    }

    private void setSuspended(boolean value) {
        readLock.lock();
        try {
            suspended = value;
            resumed.signalAll();
        } finally {
            readLock.unlock();
        }
    }

    private synchronized void enter() {
        if (closed || active) {
            return;
        }
        jline.enterRawMode();
        jline.writer().write(ENTER_SCREEN);
        jline.writer().flush();
        active = true;
        redrawRequest.set(true);
    }

    private synchronized void leave() {
        if (!active) {
            return;
        }
        active = false;
        try {
            jline.writer().write(LEAVE_SCREEN);
            jline.writer().flush();
        } finally {
            jline.setAttributes(savedAttributes);
        }
    }
}
