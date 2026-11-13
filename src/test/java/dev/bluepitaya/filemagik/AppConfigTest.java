package dev.bluepitaya.filemagik;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

class AppConfigTest {

    @TempDir
    Path tempDir;

    @Test
    void noArgsUsesWorkingDirectory() {
        AppConfig config = AppConfig.fromArgs(new String[0]);

        Path expected = Path.of(System.getProperty("user.dir")).toAbsolutePath();
        assertEquals(expected, config.startDirectory());
    }

    @Test
    void directoryArgIsUsedAsStartDirectory() {
        AppConfig config = AppConfig.fromArgs(new String[]{tempDir.toString()});

        assertEquals(tempDir.toAbsolutePath(), config.startDirectory());
    }

    @Test
    void regularFileArgIsRejected() throws IOException {
        Path file = Files.createFile(tempDir.resolve("file.txt"));

        assertThrows(RuntimeException.class, () -> AppConfig.fromArgs(new String[]{file.toString()}));
    }

    @Test
    void missingPathArgIsRejected() {
        Path missing = tempDir.resolve("does-not-exist");

        assertThrows(RuntimeException.class, () -> AppConfig.fromArgs(new String[]{missing.toString()}));
    }
}
