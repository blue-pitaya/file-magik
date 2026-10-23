package dev.bluepitaya.filemagik.ui;

import java.io.UncheckedIOException;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.function.Consumer;

public final class EventLoop {

    private static final Runnable WAKE_UP = () -> {
    };

    private final BlockingQueue<Runnable> queue = new LinkedBlockingQueue<>();

    private volatile Runnable afterEach = () -> {
    };
    private volatile Consumer<Exception> errorHandler = Throwable::printStackTrace;
    private volatile boolean stopped = false;

    public void invokeLater(Runnable task) {
        queue.add(task);
    }

    public void afterEach(Runnable afterEach) {
        this.afterEach = afterEach;
    }

    public void errorHandler(Consumer<Exception> errorHandler) {
        this.errorHandler = errorHandler;
    }

    public void run() {
        try {
            while (!stopped) {
                runSafely(queue.take());
                drain();
                if (!stopped) {
                    runSafely(afterEach);
                }
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }

    public void stop() {
        stopped = true;
        queue.add(WAKE_UP);
    }

    private void drain() {
        while (!stopped) {
            Runnable task = queue.poll();
            if (task == null) {
                return;
            }
            runSafely(task);
        }
    }

    private void runSafely(Runnable task) {
        try {
            task.run();
        } catch (UncheckedIOException e) {
            errorHandler.accept(e.getCause());
        } catch (RuntimeException e) {
            errorHandler.accept(e);
        }
    }
}
