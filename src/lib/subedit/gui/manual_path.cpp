#include <subedit/gui/manual_path.hpp>

#include <QCoreApplication>
#include <QString>

#include <filesystem>

namespace subedit::gui {

std::filesystem::path installedManualPath() {
    // `applicationDirPath()` asks for a `QCoreApplication` already built,
    // which is the case everywhere this function serves: `main` calls it after
    // building its own.
    const QString binaries = QCoreApplication::applicationDirPath();

    // `lexically_normal` so that the `..` leaves the path rather than being
    // carried all the way into the error messages.
    return (std::filesystem::path{binaries.toStdString()} / ".." / "share" / "subedit" / "manual")
        .lexically_normal();
}

} // namespace subedit::gui
