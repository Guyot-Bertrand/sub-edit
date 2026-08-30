#include <subedit/gui/manual_path.hpp>

#include <QCoreApplication>
#include <QString>

#include <filesystem>

namespace subedit::gui {

std::filesystem::path installedManualPath() {
    // `applicationDirPath()` demande une `QCoreApplication` construite, ce qui
    // est le cas partout où cette fonction sert : `main` l'appelle après avoir
    // construit la sienne.
    const QString binaries = QCoreApplication::applicationDirPath();

    // `lexically_normal` pour que le `..` disparaisse du chemin plutôt que
    // d'être porté jusque dans les messages d'erreur.
    return (std::filesystem::path{binaries.toStdString()} / ".." / "share" / "subedit" / "manual")
        .lexically_normal();
}

} // namespace subedit::gui
