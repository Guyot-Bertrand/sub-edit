// The entry point of subedit-gui.
//
// **Wiring, and nothing else.** Everything testable lives in `subedit::gui`;
// `check-architecture.sh` refuses a class or an algorithm here, and that is
// what makes the window testable without a screen.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/gui/invocation.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/player_factory.hpp>
#include <subedit/gui/qt_prompts.hpp>

#include <QApplication>
#include <QStringList>

#include <exception>
#include <iostream>
#include <utility>

int main(int argc, char** argv) {
    try {
        // Before the `QApplication`, which is the only moment Qt reads the
        // platform — and what makes the film appear in the window rather than
        // nowhere on a Wayland session.
        subedit::gui::preferEmbeddablePlatform();

        const QApplication application{argc, argv};

        const QStringList arguments = QApplication::arguments();
        if (subedit::gui::reportVersion(arguments, std::cout))
            return 0;

        subedit::core::RealFileSystem files;
        subedit::core::OpenedFile opened =
            subedit::gui::openFromArguments(files, arguments, std::cerr);

        subedit::gui::QtPrompts prompts;

        // The player of ADR 0020, made where the window can hand over the
        // surface it draws into — libmpv reads that only while it initialises.
        subedit::gui::MainWindow window{files,
                                        std::move(opened),
                                        prompts,
                                        subedit::gui::mpvPlayers(),
                                        subedit::gui::declaredFrameRates(files)};
        window.show();

        return QApplication::exec();
    } catch (const std::exception& error) {
        std::cerr << "subedit-gui: " << error.what() << "\n";
        return 2;
    }
}
