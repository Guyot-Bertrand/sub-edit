#include <subedit/core/version.hpp>
#include <subedit/gui/invocation.hpp>

#include <QString>
#include <QStringList>

#include <ostream>

namespace subedit::gui {

bool reportVersion(const QStringList& arguments, std::ostream& out) {
    if (!arguments.contains(QStringLiteral("--version")))
        return false;

    out << "subedit " << core::versionString() << "\n";
    return true;
}

} // namespace subedit::gui
