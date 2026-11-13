package dev.bluepitaya.filemagik;

import dev.bluepitaya.filemagik.explorer.ExplorerHandler;
import dev.bluepitaya.filemagik.explorer.ExplorerView;
import dev.bluepitaya.filemagik.preview.PreviewHandler;
import dev.bluepitaya.filemagik.preview.PreviewView;
import dev.bluepitaya.filemagik.service.FileService;
import dev.bluepitaya.filemagik.terminal.FrameBuffer;
import dev.bluepitaya.filemagik.terminal.Terminal;
import dev.bluepitaya.filemagik.ui.EventLoop;

import java.io.IOException;

final class App {
    public static void main(String[] args) throws IOException {
        AppConfig config = AppConfig.fromArgs(args);
        Terminal terminal = new Terminal();
        EventLoop eventLoop = new EventLoop();
        FileService fileService = new FileService();
        PreviewHandler previewHandler = new PreviewHandler(fileService);
        ExplorerHandler explorerHandler = new ExplorerHandler(fileService, previewHandler::show);
        FrameBuffer frameBuffer = new FrameBuffer(terminal);
        PreviewView previewView = new PreviewView(previewHandler);
        ExplorerView explorerView = new ExplorerView(explorerHandler, previewView);
        AppRunner appRunner = new AppRunner(terminal, eventLoop, explorerHandler, frameBuffer, explorerView);

        appRunner.run(config.startDirectory());
    }
}
