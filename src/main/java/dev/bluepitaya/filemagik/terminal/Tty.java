package dev.bluepitaya.filemagik.terminal;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

final class Tty {

    private static final String PATH = "/dev/tty";

    public static File file() {
        return new File(PATH);
    }

    public static InputStream openInput() throws IOException {
        return new FileInputStream(PATH);
    }

    public static OutputStream openOutput() throws IOException {
        return new FileOutputStream(PATH);
    }
}
