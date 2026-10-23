package dev.bluepitaya.filemagik;

import java.nio.file.Path;

final class AppConfig {

    public Path startDirectory() {
        return Path.of(System.getProperty("user.dir")).toAbsolutePath();
    }
}
