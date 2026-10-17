#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/text/enchant_spell_provider.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/version.hpp>
#include <subedit/core/wording/formats.hpp>
#include <subedit/core/wording/settings.hpp>
#include <subedit/gui/invocation.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/manual_path.hpp>
#include <subedit/gui/settings_path.hpp>
#include <subedit/platform/locations.hpp>

#include <QString>
#include <QStringList>

#include <expected>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::gui {

namespace {

/// `said` behind the program's name, as every line this program writes on its errors begins.
[[nodiscard]] std::string prefixed(std::string_view said) {
    // TRANSLATORS: the start of every line the window writes on its error stream.
    return core::substitute(core::translate("subedit-gui: %1"), {said});
}

/// « path: reason », as the command line says it.
[[nodiscard]] std::string located(std::string_view path, std::string_view reason) {
    // TRANSLATORS: %1 is a file's path, %2 what went wrong with it.
    return core::substitute(core::translate("%1: %2"), {path, reason});
}

} // namespace

void reportFatal(std::ostream& errors, std::string_view what) {
    errors << prefixed(what) << "\n";
}

bool reportVersion(const QStringList& arguments, std::ostream& out) {
    if (!arguments.contains(QStringLiteral("--version")))
        return false;

    out << "subedit " << core::versionString() << "\n";
    return true;
}

core::OpenedFile openFromArguments(const core::FileSystem& files,
                                   const QStringList& arguments,
                                   std::ostream& errors) {
    if (arguments.size() <= 1)
        return {};

    const std::string path = arguments.at(1).toStdString();
    std::expected<core::OpenedFile, core::OpenError> read = core::openProject(files, path);
    if (read)
        return std::move(*read);

    // **The cause, named.** It used to be « nothing to open » whatever had
    // happened — a file that is absent, one the system refuses, one that is not
    // UTF-8 and one that is a Word document all got the same sentence, and the
    // sentence was true of the last only. The window now has what the command
    // line always had, and says it the same way.
    errors << prefixed(located(path, core::reasonOf(read.error()))) << "\n";
    return {};
}

core::Settings readUserSettings(const core::FileSystem& files,
                                const std::filesystem::path& path,
                                std::ostream& errors) {
    const core::SettingsRead read = core::readSettings(files, path);

    // The whole file first: what follows cannot exist if it was not read, and
    // the order of the lines follows that of the causes.
    if (read.unreadable.has_value()) {
        errors << prefixed(located(path.string(), core::reasonOf(read.unreadable->kind))) << "\n";
    }

    for (const core::SettingsDiagnostic& said : read.diagnostics) {
        // The offending value is quoted: without it, "cannot be read" leaves
        // the user hunting through their file for what was wrong. In straight
        // quotes, like everything the binary writes — guillemets are French
        // typography, and the output is English.
        // TRANSLATORS: %1 is a key of the settings file, %2 its value, %3 why it was refused.
        errors << prefixed(
                      located(path.string(),
                              core::substitute(core::translate("%1: \"%2\" %3"),
                                               {said.key, said.value, core::unreadableSetting()})))
               << "\n";
    }

    return read.settings;
}

void writeUserSettings(core::FileSystem& files,
                       const std::filesystem::path& path,
                       const core::Settings& settings,
                       std::ostream& errors) {
    if (!core::writeSettings(files, path, settings))
        errors << prefixed(located(path.string(), core::settingsNotWritten())) << "\n";
}

core::Settings readUserSettings(const core::FileSystem& files, std::ostream& errors) {
    return readUserSettings(files, userSettingsPath(), errors);
}

void writeUserSettings(core::FileSystem& files,
                       const core::Settings& settings,
                       std::ostream& errors) {
    writeUserSettings(files, userSettingsPath(), settings, errors);
}

void configureFromEnvironment(MainWindow& window,
                              const core::FileSystem& files,
                              const std::optional<std::string>& language,
                              std::ostream& errors) {
    window.applySettings(readUserSettings(files, errors));
    window.setManualPath(installedManualPath(), language);
    window.setLocaleDirectory(platform::installedLocalePath());
    window.setPatternCatalogue(core::readPatternCatalogue(
        files, platform::installedPatternsPath(), platform::resolvedUserPatternsPath()));
    window.setSpellChecking(std::make_shared<const core::EnchantSpellProvider>(),
                            userSettingsPath().parent_path());
}

} // namespace subedit::gui
