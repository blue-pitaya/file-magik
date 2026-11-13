package dev.bluepitaya.filemagik.service;

import java.io.IOException;
import java.nio.file.Path;
import java.util.List;
import java.util.Locale;

import dev.bluepitaya.filemagik.terminal.Terminal;

public final class FileOpener {

    private final Terminal terminal;

    public FileOpener(Terminal terminal) {
        this.terminal = terminal;
    }

    public void open(Path file) throws IOException, UnsupportedFileTypeException {
        switch (getExtension(file)) {
            case "mkv", "opus", "m4a" -> terminal.runInForeground(List.of("mpv", "--", file.toString()));
            default -> throw new UnsupportedFileTypeException(file);
        }
    }

    private static String getExtension(Path file) {
        String name = file.getFileName().toString();
        int dot = name.lastIndexOf('.');

        return dot <= 0 ? "" : name.substring(dot + 1).toLowerCase(Locale.ROOT);
    }
}
