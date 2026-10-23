package dev.bluepitaya.filemagik;

import java.io.IOException;

import dev.bluepitaya.filemagik.terminal.Terminal;
import dev.bluepitaya.filemagik.ui.EventLoop;

final class App {
    public static void main(String[] args) throws IOException {
        AppConfig config = new AppConfig();
        Terminal terminal = new Terminal();
        EventLoop eventLoop = new EventLoop();
        AppScreen appScreen = new AppScreen(terminal, eventLoop);

        appScreen.start();
    }
}
