package dev.bluepitaya.filemagik.terminal;

public enum Color {
    DEFAULT(39),
    RED(31),
    GREEN(32),
    YELLOW(33),
    BLUE(34),
    CYAN(36),
    BRIGHT_BLACK(90);

    private final int fgCode;

    Color(int fgCode) {
        this.fgCode = fgCode;
    }

    int fgCode() {
        return fgCode;
    }
}
