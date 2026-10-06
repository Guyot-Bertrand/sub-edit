// The entry point of subedit-gui.
//
// **Wiring, and nothing else.** Everything testable lives in `subedit::gui`;
// `check-architecture.sh` refuses a class or an algorithm here, and that is
// what makes the window testable without a screen.

#include <subedit/core/config/settings.hpp>
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
        const QApplication application{argc, argv};

        const QStringList arguments = QApplication::arguments();
        if (subedit::gui::reportVersion(arguments, std::cout))
            return 0;

        subedit::core::RealFileSystem files;
        subedit::core::OpenedFile opened =
            subedit::gui::openFromArguments(files, arguments, std::cerr);

        subedit::gui::QtPrompts prompts;

        // The player of ADR 0020, drawn by the window itself — ADR 0041.
        subedit::gui::MainWindow window{files,
                                        std::move(opened),
                                        prompts,
                                        subedit::gui::mpvPlayers(),
                                        subedit::gui::declaredFrameRates(files)};
        // The settings, the manual and the pattern catalogue — ADR 0022's own
        // rule, all three resolved here, with the real executable and the
        // real environment, and nowhere else.
        subedit::gui::configureFromEnvironment(window, files, std::cerr);
        window.show();

        const int code = QApplication::exec();
        subedit::gui::writeUserSettings(files, window.settings(), std::cerr);
        return code;
    } catch (const std::exception& error) {
        std::cerr << "subedit-gui: " << error.what() << "\n";
        return 2;
    }
}
