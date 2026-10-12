#include <subedit/gui/hearing_impaired_dialog.hpp>
#include <subedit/gui/messages.hpp>

#include <QFormLayout>
#include <QLabel>
#include <QString>

#include <cstddef>

namespace subedit::gui {

HearingImpairedDialog::HearingImpairedDialog(OperationScope scope, QWidget* parent)
    : OperationDialog(scope, parent) {
    setWindowTitle(translate("Remove hearing-impaired mentions"));

    fields()->addRow(
        new QLabel{translate("Mentions in square brackets are removed; parentheses are kept.\n"
                             "A subtitle left with nothing is taken away."),
                   this});
    finish();
}

} // namespace subedit::gui
