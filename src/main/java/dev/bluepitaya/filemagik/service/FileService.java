package dev.bluepitaya.filemagik.service;

import java.io.IOException;
import java.nio.file.DirectoryIteratorException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

public final class FileService {

    private static final Comparator<FileEntry> ORDER = Comparator
            .comparingInt((FileEntry entry) -> entry.isDirectory() ? 0 : 1)
            .thenComparing(FileService::compareByName);

    public DirectoryListing listing(Path dir) throws IOException {
        Path directory = dir.toAbsolutePath().normalize();
        List<FileEntry> entries = new ArrayList<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(directory)) {
            for (Path path : stream) {
                String name = path.getFileName().toString();
                if (name.startsWith(".")) {
                    continue;
                }
                entries.add(new FileEntry(path, name, typeOf(path)));
            }
        } catch (DirectoryIteratorException e) {
            throw e.getCause();
        }

        entries.sort(ORDER);
        return new DirectoryListing(directory, entries);
    }

    private static FileEntry.Type typeOf(Path path) {
        BasicFileAttributes own = readAttributes(path, LinkOption.NOFOLLOW_LINKS);
        if (own == null) {
            return FileEntry.Type.OTHER;
        }
        if (!own.isSymbolicLink()) {
            if (own.isDirectory()) {
                return FileEntry.Type.DIRECTORY;
            }
            return own.isRegularFile() ? FileEntry.Type.FILE : FileEntry.Type.OTHER;
        }

        BasicFileAttributes target = readAttributes(path);
        if (target == null) {
            return FileEntry.Type.BROKEN_SYMLINK;
        }
        return target.isDirectory() ? FileEntry.Type.SYMLINK_DIRECTORY : FileEntry.Type.SYMLINK_FILE;
    }

    private static BasicFileAttributes readAttributes(Path path, LinkOption... options) {
        try {
            return Files.readAttributes(path, BasicFileAttributes.class, options);
        } catch (IOException e) {
            return null;
        }
    }

    private static int compareByName(FileEntry a, FileEntry b) {
        int result = compareNatural(a.name(), b.name());
        return result != 0 ? result : a.name().compareTo(b.name());
    }

    private static int compareNatural(String a, String b) {
        int i = 0;
        int j = 0;
        while (i < a.length() && j < b.length()) {
            char first = a.charAt(i);
            char second = b.charAt(j);
            if (isDigit(first) && isDigit(second)) {
                int startA = i;
                int startB = j;
                while (i < a.length() && isDigit(a.charAt(i))) {
                    i++;
                }
                while (j < b.length() && isDigit(b.charAt(j))) {
                    j++;
                }
                int result = compareNumbers(a.substring(startA, i), b.substring(startB, j));
                if (result != 0) {
                    return result;
                }
                continue;
            }

            int result = Character.compare(Character.toLowerCase(first), Character.toLowerCase(second));
            if (result != 0) {
                return result;
            }
            i++;
            j++;
        }
        return Integer.compare(a.length() - i, b.length() - j);
    }

    private static int compareNumbers(String a, String b) {
        String first = stripLeadingZeros(a);
        String second = stripLeadingZeros(b);
        if (first.length() != second.length()) {
            return Integer.compare(first.length(), second.length());
        }
        return first.compareTo(second);
    }

    private static String stripLeadingZeros(String digits) {
        int index = 0;
        while (index < digits.length() - 1 && digits.charAt(index) == '0') {
            index++;
        }
        return digits.substring(index);
    }

    private static boolean isDigit(char c) {
        return c >= '0' && c <= '9';
    }
}
