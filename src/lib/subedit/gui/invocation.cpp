#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/version.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/invocation.hpp>

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

} // namespace subedit::gui
