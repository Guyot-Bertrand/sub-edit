#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/version.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/invocation.hpp>
#include <subedit/gui/settings_path.hpp>

#include <QString>
#include <QStringList>

#include <expected>
#include <ostream>
#include <string>
#include <utility>

namespace subedit::gui {

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
    errors << "subedit-gui: " << path << ": " << core::reasonOf(read.error()) << "\n";
    return {};
}

core::Settings readUserSettings(const core::FileSystem& files,
                                const std::filesystem::path& path,
                                std::ostream& errors) {
    const core::SettingsRead read = core::readSettings(files, path);

    // Le fichier entier d'abord : ce qui suit ne peut pas exister s'il n'a pas
    // été lu, et l'ordre des lignes suit celui des causes.
    if (read.unreadable.has_value()) {
        errors << "subedit-gui: " << path.string() << ": " << core::reasonOf(read.unreadable->kind)
               << "\n";
    }

    for (const core::SettingsDiagnostic& said : read.diagnostics) {
        // La valeur fautive est citée : sans elle, « cannot be read » laisse
        // l'utilisateur chercher dans son fichier ce qui n'allait pas. Entre
        // guillemets droits, comme tout ce que le binaire écrit — les
        // chevrons sont de la typographie française, et la sortie est en
        // anglais.
        errors << "subedit-gui: " << path.string() << ": " << said.key << ": \"" << said.value
               << "\" " << core::unreadableSetting() << "\n";
    }

    return read.settings;
}

void writeUserSettings(core::FileSystem& files,
                       const std::filesystem::path& path,
                       const core::Settings& settings,
                       std::ostream& errors) {
    if (!core::writeSettings(files, path, settings))
        errors << "subedit-gui: " << path.string() << ": " << core::settingsNotWritten() << "\n";
}

core::Settings readUserSettings(const core::FileSystem& files, std::ostream& errors) {
    return readUserSettings(files, userSettingsPath(), errors);
}

void writeUserSettings(core::FileSystem& files,
                       const core::Settings& settings,
                       std::ostream& errors) {
    writeUserSettings(files, userSettingsPath(), settings, errors);
}

} // namespace subedit::gui
