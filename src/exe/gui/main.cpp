// The entry point of subedit-gui.
//
// **Wiring, and nothing else.** Everything testable lives in `subedit::gui`;
// `check-architecture.sh` refuses a class or an algorithm here, and that is
// what makes the window testable without a screen.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/version.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>
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
        if (arguments.contains(QStringLiteral("--version"))) {
            std::cout << "subedit " << subedit::core::versionString() << "\n";
            return 0;
        }

        subedit::core::RealFileSystem files;
        subedit::gui::OpenedFile opened;

        if (arguments.size() > 1) {
            auto read = subedit::gui::openProject(files, arguments.at(1).toStdString());
            if (read)
                opened = std::move(*read);
            else
                std::cerr << "subedit-gui: " << arguments.at(1).toStdString()
                          << ": nothing to open\n";
        }

        subedit::gui::QtPrompts prompts;
        subedit::gui::MainWindow window{files, std::move(opened), prompts};
        window.show();

        return QApplication::exec();
    } catch (const std::exception& error) {
        std::cerr << "subedit-gui: " << error.what() << "\n";
        return 2;
    }
}
