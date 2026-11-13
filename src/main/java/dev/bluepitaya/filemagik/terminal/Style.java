package dev.bluepitaya.filemagik.terminal;

public final class Style {

    public static final Style DEFAULT = new Style(Color.DEFAULT, false, false);

    private final Color fg;
    private final boolean bold;
    private final boolean reverse;

    private Style(Color fg, boolean bold, boolean reverse) {
        this.fg = fg;
        this.bold = bold;
        this.reverse = reverse;
    }

    public Style fg(Color color) {
        return new Style(color, bold, reverse);
    }

    public Style bold() {
        return new Style(fg, true, reverse);
    }

    public Style reverse() {
        return new Style(fg, bold, true);
    }

    String toSgr() {
        StringBuilder sb = new StringBuilder("\u001b[0");
        if (bold) {
            sb.append(";1");
        }
        if (reverse) {
            sb.append(";7");
        }
        if (fg != Color.DEFAULT) {
            sb.append(';').append(fg.fgCode());
        }
        return sb.append('m').toString();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof Style other)) {
            return false;
        }
        return fg == other.fg && bold == other.bold && reverse == other.reverse;
    }

    @Override
    public int hashCode() {
        int result = fg.hashCode();
        result = 31 * result + (bold ? 1 : 0);
        result = 31 * result + (reverse ? 1 : 0);
        return result;
    }
}
