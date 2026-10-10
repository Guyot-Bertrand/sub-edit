#include <subedit/gui/hearing_impaired_dialog.hpp>

#include <QFormLayout>
#include <QLabel>
#include <QString>

#include <cstddef>

namespace subedit::gui {

HearingImpairedDialog::HearingImpairedDialog(std::size_t targetCount, QWidget* parent)
    : OperationDialog(targetCount, parent) {
    setWindowTitle(QStringLiteral("Remove hearing-impaired mentions"));

    fields()->addRow(
        new QLabel{QStringLiteral("Mentions in square brackets are removed; parentheses are kept.\n"
                                  "A subtitle left with nothing is taken away."),
                   this});
    finish();
}

} // namespace subedit::gui
