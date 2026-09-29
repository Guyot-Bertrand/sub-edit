#include <subedit/core/text/pattern_paths.hpp>
#include <subedit/gui/patterns_path.hpp>

#include <QByteArray>
#include <QCoreApplication>
#include <QString>

namespace subedit::gui {

std::filesystem::path installedPatternsPath() {
    const QString binaries = QCoreApplication::applicationDirPath();
    return core::shippedPatternsPath(std::filesystem::path{binaries.toStdString()});
}

std::filesystem::path resolvedUserPatternsPath() {
    const QByteArray xdg = qgetenv("XDG_DATA_HOME");
    const QByteArray home = qgetenv("HOME");
    return core::userPatternsPath(
        std::string_view{xdg.constData(), static_cast<std::size_t>(xdg.size())},
        std::string_view{home.constData(), static_cast<std::size_t>(home.size())});
}

} // namespace subedit::gui
