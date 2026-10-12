#include <subedit/core/i18n/language.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/gui/interface_language.hpp>
#include <subedit/platform/locations.hpp>

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QString>
#include <QTranslator>

#include <filesystem>
#include <memory>
#include <optional>
#include <utility>

namespace subedit::gui {

InterfaceLanguage::InterfaceLanguage()
    : InterfaceLanguage(core::processEnvironment(),
                        platform::installedLocalePath(),
                        QLibraryInfo::path(QLibraryInfo::TranslationsPath)) {}

InterfaceLanguage::InterfaceLanguage(const core::EnvironmentLookup& environment,
                                     const std::filesystem::path& localeDirectory,
                                     const QString& qtDirectory) {
    std::optional<core::InstalledLanguage> found =
        core::installedLanguage(core::languageCandidates(environment), localeDirectory);
    if (!found)
        return;

    m_language = found->name;
    core::installCatalogue(std::move(found->catalogue));

    // `qtbase_fr_FR` falls back on `qtbase_fr` by itself: Qt drops a suffix at a time.
    auto qt = std::make_unique<QTranslator>();
    if (!qt->load(QStringLiteral("qtbase_") + QString::fromStdString(found->name), qtDirectory))
        return;
    QCoreApplication::installTranslator(qt.get());
    m_qt = std::move(qt);
}

InterfaceLanguage::~InterfaceLanguage() {
    if (m_qt)
        QCoreApplication::removeTranslator(m_qt.get());
    if (m_language)
        core::installCatalogue(nullptr);
}

} // namespace subedit::gui
