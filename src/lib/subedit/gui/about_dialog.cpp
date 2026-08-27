#include <subedit/core/version.hpp>
#include <subedit/gui/about_dialog.hpp>

#include <QDialogButtonBox>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>

namespace subedit::gui {

namespace {

/// The three things an « about » is for: what this is, which one, and under
/// what terms.
///
/// The version is derived from `core::versionString` rather than written here:
/// the number lives in `CMakeLists.txt` and is copied in exactly one place, the
/// `--version` block of the manual, which `manual-check` holds to the binary.
[[nodiscard]] QString said() {
    return QStringList{
        QStringLiteral("subedit %1")
            .arg(QString::fromStdString(std::string{core::versionString()})),
        QStringLiteral("A subtitle editor."),
        QStringLiteral("Licensed under the GPL, version 3 or later."),
    }
        .join(QStringLiteral("\n\n"));
}

} // namespace

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent), m_text(new QLabel{said(), this}) {
    setWindowTitle(QStringLiteral("About subedit"));

    m_text->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Close, this};
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(m_text);
    layout->addWidget(buttons);
}

QString AboutDialog::text() const {
    return m_text->text();
}

} // namespace subedit::gui
