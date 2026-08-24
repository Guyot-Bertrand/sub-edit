#include <subedit/core/format/read_error.hpp>
#include <subedit/core/version.hpp>
#include <subedit/gui/invocation.hpp>
#include <subedit/gui/opening.hpp>

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

OpenedFile openFromArguments(const core::FileSystem& files,
                             const QStringList& arguments,
                             std::ostream& errors) {
    if (arguments.size() <= 1)
        return {};

    const std::string path = arguments.at(1).toStdString();
    std::expected<OpenedFile, core::ReadError> read = openProject(files, path);
    if (read)
        return std::move(*read);

    // **One message for the four ways of failing**, which is a decision and not
    // a shortcut: the window does not distinguish what the file system refused
    // from what no format recognised, and the manual says so. The command line
    // is where a reader is told which of the four it met.
    errors << "subedit-gui: " << path << ": nothing to open\n";
    return {};
}

} // namespace subedit::gui
