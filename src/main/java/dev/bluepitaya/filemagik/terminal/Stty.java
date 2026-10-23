package dev.bluepitaya.filemagik.terminal;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import dev.bluepitaya.filemagik.terminal.Terminal.TerminalSize;

final class Stty {

    private static final TerminalSize FALLBACK_SIZE = new TerminalSize(80, 24);

    public static TerminalSize size() throws IOException {
        String[] parts = run("size").trim().split("\\s+");
        if (parts.length != 2) {
            return FALLBACK_SIZE;
        }

        try {
            int rows = Integer.parseInt(parts[0]);
            int columns = Integer.parseInt(parts[1]);
            if (rows <= 0 || columns <= 0) {
                return FALLBACK_SIZE;
            }
            return new TerminalSize(columns, rows);
        } catch (NumberFormatException e) {
            return FALLBACK_SIZE;
        }
    }

    public static String run(String... args) throws IOException {
        List<String> command = new ArrayList<>();
        command.add("stty");
        command.addAll(List.of(args));
        Process process = new ProcessBuilder(command)
                .redirectInput(Tty.file())
                .redirectErrorStream(true)
                .start();
        String output = new String(process.getInputStream().readAllBytes(), StandardCharsets.UTF_8);
        int exitCode = waitFor(process);
        if (exitCode != 0) {
            throw new IOException("stty " + String.join(" ", args) + " failed: " + output.trim());
        }
        return output;
    }

    private static int waitFor(Process process) throws IOException {
        try {
            return process.waitFor();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new InterruptedIOException("stty was interrupted");
        }
    }
}
