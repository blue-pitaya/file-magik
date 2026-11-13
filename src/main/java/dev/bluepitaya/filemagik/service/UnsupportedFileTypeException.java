package dev.bluepitaya.filemagik.service;

import java.nio.file.Path;

public final class UnsupportedFileTypeException extends Exception {

    public UnsupportedFileTypeException(Path file) {
        super("No program to open " + file.getFileName());
    }
}
