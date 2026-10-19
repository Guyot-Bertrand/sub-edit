#include <subedit/core/config/settings.hpp>
#include <subedit/core/i18n/language.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/gui/interface_language.hpp>
#include <subedit/gui/settings_path.hpp>
#include <subedit/platform/locations.hpp>

#include <QCoreApplication>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QString>
#include <QTranslator>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace subedit::gui {

InterfaceLanguage::InterfaceLanguage() : InterfaceLanguage(std::nullopt) {}

InterfaceLanguage::InterfaceLanguage(const std::optional<std::string>& preferred)
    : InterfaceLanguage(preferred,
                        core::processEnvironment(),
                        platform::installedLocalePath(),
                        QLibraryInfo::path(QLibraryInfo::TranslationsPath)) {}

InterfaceLanguage::InterfaceLanguage(const core::EnvironmentLookup& environment,
                                     const std::filesystem::path& localeDirectory,
                                     const QString& qtDirectory)
    : InterfaceLanguage(std::nullopt, environment, localeDirectory, qtDirectory) {}

InterfaceLanguage::InterfaceLanguage(const std::optional<std::string>& preferred,
                                     const core::EnvironmentLookup& environment,
                                     const std::filesystem::path& localeDirectory,
                                     const QString& qtDirectory) {
    std::optional<core::InstalledLanguage> found =
        core::chosenLanguage(preferred, environment, localeDirectory);
    if (!found)
        return;

    m_language = found->name;
    core::installCatalogue(std::move(found->catalogue));

    // The whole window mirrors under a right-to-left language: menus, dialogs, the table's
    // columns. Text that is not language — the timeline, the video controls — sets its own.
    if (QLocale(QString::fromStdString(found->name)).textDirection() == Qt::RightToLeft) {
        QGuiApplication::setLayoutDirection(Qt::RightToLeft);
        m_mirrored = true;
    }

    // `qtbase_fr_FR` falls back on `qtbase_fr` by itself: Qt drops a suffix at a time.
    auto qt = std::make_unique<QTranslator>();
    if (!qt->load(QStringLiteral("qtbase_") + QString::fromStdString(found->name), qtDirectory))
        return;
    QCoreApplication::installTranslator(qt.get());
    m_qt = std::move(qt);
}

InterfaceLanguage::~InterfaceLanguage() {
    if (m_mirrored)
        QGuiApplication::setLayoutDirection(Qt::LeftToRight);
    if (m_qt)
        QCoreApplication::removeTranslator(m_qt.get());
    if (m_language)
        core::installCatalogue(nullptr);
}

std::optional<std::string> settingsLanguage(const core::FileSystem& files,
                                            const std::filesystem::path& path) {
    return core::readSettings(files, path).settings.interfaceLanguage;
}

std::optional<std::string> settingsLanguage(const core::FileSystem& files) {
    return settingsLanguage(files, userSettingsPath());
}

} // namespace subedit::gui
