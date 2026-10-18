#include <subedit/core/libraries.hpp>
#include <subedit/core/version.hpp>
#include <subedit/core/wording/version.hpp>
#include <subedit/gui/about_dialog.hpp>
#include <subedit/gui/libraries.hpp>
#include <subedit/gui/messages.hpp>

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <cstddef>
#include <string>
#include <vector>

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
        translate("A subtitle editor."),
        translate("Licensed under the GPL, version 3 or later."),
    }
        .join(QStringLiteral("\n\n"));
}

/// A cell of the table: a name or a version, never translated.
[[nodiscard]] QTableWidgetItem* cell(const std::string& text) {
    return new QTableWidgetItem{QString::fromStdString(text)};
}

} // namespace

AboutDialog::AboutDialog(const std::vector<std::string>& languages, QWidget* parent)
    : QDialog(parent),
      m_text(new QLabel{said(), this}),
      m_libraries(new QTableWidget{this}),
      m_languages(new QLabel{QString::fromStdString(core::languagesLine(languages)), this}) {
    setWindowTitle(translate("About subedit"));

    m_text->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_languages->setTextInteractionFlags(Qt::TextSelectableByMouse);

    const std::vector<core::Library> linked = windowLibraries();
    m_libraries->setColumnCount(3);
    m_libraries->setRowCount(static_cast<int>(linked.size()));
    m_libraries->setHorizontalHeaderLabels(QStringList{
        // TRANSLATORS: the heading of the column of the libraries' names, in « About subedit ».
        translate("Library"),
        // TRANSLATORS: the heading of the version of a library the program runs against.
        translate("Loaded"),
        // TRANSLATORS: the heading of the version of a library the program was compiled with.
        translate("Built with"),
    });
    m_libraries->verticalHeader()->setVisible(false);
    m_libraries->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_libraries->setSelectionMode(QAbstractItemView::NoSelection);
    m_libraries->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    for (std::size_t at = 0; at < linked.size(); ++at) {
        const int row = static_cast<int>(at);
        m_libraries->setItem(row, 0, cell(linked[at].name));
        m_libraries->setItem(row, 1, cell(linked[at].loaded));
        m_libraries->setItem(row, 2, cell(linked[at].built));
    }

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Close, this};
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(m_text);
    layout->addWidget(m_libraries);
    layout->addWidget(m_languages);
    layout->addWidget(buttons);
}

QString AboutDialog::text() const {
    return m_text->text();
}

QString AboutDialog::languages() const {
    return m_languages->text();
}

} // namespace subedit::gui
