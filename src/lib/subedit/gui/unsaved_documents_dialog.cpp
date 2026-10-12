#include <subedit/core/model/document.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/target.hpp>
#include <subedit/gui/unsaved_documents_dialog.hpp>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include <algorithm>
#include <cstddef>

namespace subedit::gui {

namespace {

/// « Translation — film.en.srt », and what is wrong with the file when
/// something is.
[[nodiscard]] QString labelOf(const ModifiedDocument& modified) {
    const QString label = QStringLiteral("%1 — %2").arg(documentName(modified.document),
                                                        QString::fromStdString(modified.name));
    // TRANSLATORS: %1 is « Main — film.srt », a document and its file.
    return modified.missing ? substitute(translate("%1 (file is gone from the disk)"), label)
                            : label;
}

} // namespace

UnsavedDocumentsDialog::UnsavedDocumentsDialog(std::span<const ModifiedDocument> documents,
                                               QWidget* parent)
    : QDialog(parent),
      m_save(new QPushButton{translate("&Save"), this}),
      m_discard(new QPushButton{translate("Close &Without Saving"), this}),
      m_cancel(new QPushButton{translate("Cancel"), this}) {
    setWindowTitle(translate("Unsaved changes"));

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(new QLabel{translate("These documents have changes that were never written.\n"
                                           "Tick the ones to save."),
                                 this});

    for (const ModifiedDocument& modified : documents) {
        auto* box = new QCheckBox{labelOf(modified), this};
        box->setChecked(true);
        connect(box, &QCheckBox::toggled, this, [this] { refreshSave(); });
        layout->addWidget(box);
        m_boxes.append(box);
        m_documents.push_back(modified.document);
    }

    m_save->setDefault(true);
    m_cancel->setAutoDefault(false);
    m_discard->setAutoDefault(false);

    auto* buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(m_discard);
    buttons->addWidget(m_cancel);
    buttons->addWidget(m_save);
    layout->addLayout(buttons);

    connect(m_save, &QPushButton::clicked, this, [this] {
        m_choice = UnsavedChoice::Save;
        m_chosen.clear();
        for (qsizetype index = 0; index < m_boxes.size(); ++index) {
            if (m_boxes.at(index)->isChecked())
                m_chosen.push_back(m_documents.at(static_cast<std::size_t>(index)));
        }
        accept();
    });
    connect(m_discard, &QPushButton::clicked, this, [this] {
        m_choice = UnsavedChoice::Discard;
        m_chosen.clear();
        accept();
    });
    connect(m_cancel, &QPushButton::clicked, this, &QDialog::reject);
}

std::vector<core::Document> UnsavedDocumentsDialog::toSave() const {
    return m_chosen;
}

void UnsavedDocumentsDialog::refreshSave() {
    m_save->setEnabled(std::ranges::any_of(m_boxes, &QCheckBox::isChecked));
}

} // namespace subedit::gui
