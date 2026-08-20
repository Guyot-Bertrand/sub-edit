// L'entrée de subedit-gui.
//
// **Du câblage, et rien d'autre.** Tout ce qui se teste vit dans
// `subedit::gui` ; `check-architecture.sh` refuse ici une classe ou un
// algorithme, et c'est ce qui rend la fenêtre éprouvable sans écran.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/version.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>

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

        const subedit::core::RealFileSystem files;
        subedit::core::Project project;

        if (arguments.size() > 1) {
            auto opened = subedit::gui::openProject(files, arguments.at(1).toStdString());
            if (opened)
                project = std::move(*opened);
            else
                std::cerr << "subedit-gui: " << arguments.at(1).toStdString()
                          << " : rien à ouvrir\n";
        }

        subedit::gui::MainWindow window{std::move(project)};
        window.show();

        return QApplication::exec();
    } catch (const std::exception& error) {
        std::cerr << "subedit-gui: " << error.what() << "\n";
        return 2;
    }
}
