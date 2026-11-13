package dev.bluepitaya.filemagik;

import java.nio.file.Files;
import java.nio.file.Path;

record AppConfig(Path startDirectory) {

    static AppConfig fromArgs(String[] args) {
        if (args.length == 1) {
            Path path = Path.of(args[0]);
            if (!Files.isDirectory(path)) {
                throw new RuntimeException(path + " is not a directory");
            }
            return new AppConfig(path.toAbsolutePath());
        }
        return new AppConfig(Path.of(System.getProperty("user.dir")).toAbsolutePath());
    }
}
