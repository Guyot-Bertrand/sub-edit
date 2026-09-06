#include <subedit/core/config/insert_placement.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/insert_dialog.hpp>

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>

#include <cstddef>

namespace subedit::gui {

namespace {

/// How many rows one may ask for at once.
///
/// Gaupol's bounds, taken as they are: one at least — inserting zero rows is
/// not an operation — and a ceiling high enough never to have to be argued
/// over.
constexpr int kSmallestCount = 1;
constexpr int kLargestCount = 99999;

/// The button of one side, named by the core.
///
/// One function for two calls, and not two `new`s in the initialiser list:
/// written there they run past the line, and a line broken inside an
/// initialiser list becomes a line coverage counts without ever reaching — it
/// is the exception clean-up code that files itself there.
[[nodiscard]] QRadioButton* buttonFor(core::InsertPlacement placement, QWidget* parent) {
    return new QRadioButton{QString::fromUtf8(core::nameOf(placement)), parent};
}

} // namespace

InsertDialog::InsertDialog(bool hasSubtitles, core::InsertPlacement placement, QWidget* parent)
    : QDialog(parent),
      m_count(new QSpinBox{this}),
      m_above(buttonFor(core::InsertPlacement::Above, this)),
      m_below(buttonFor(core::InsertPlacement::Below, this)) {
    setWindowTitle(QStringLiteral("Insert subtitles"));

    m_count->setRange(kSmallestCount, kLargestCount);
    m_count->setValue(kSmallestCount);

    // Grouped on purpose: two radio buttons of one parent already are, but the
    // group says the intent where the layout only implies it — and it would
    // survive a field added between the two.
    auto* side = new QButtonGroup{this};
    side->addButton(m_above);
    side->addButton(m_below);

    setPlacement(placement);

    // Out rather than hidden in an empty document: there is no selection to
    // place it against, so no side to choose, and the insertion happens at the
    // beginning.
    m_above->setEnabled(hasSubtitles);
    m_below->setEnabled(hasSubtitles);

    auto* fields = new QFormLayout;
    fields->addRow(QStringLiteral("How many"), m_count);
    fields->addRow(QStringLiteral("Where"), m_above);
    fields->addRow(QString{}, m_below);

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addLayout(fields);
    stack->addWidget(buttons);
}

std::size_t InsertDialog::count() const {
    // The field does not go below one, so the conversion cannot answer zero:
    // it is the bound that guarantees it, and not a check here.
    return static_cast<std::size_t>(m_count->value());
}

core::InsertPlacement InsertDialog::placement() const {
    return m_above->isChecked() ? core::InsertPlacement::Above : core::InsertPlacement::Below;
}

void InsertDialog::setPlacement(core::InsertPlacement placement) {
    m_above->setChecked(placement == core::InsertPlacement::Above);
    m_below->setChecked(placement == core::InsertPlacement::Below);
}

} // namespace subedit::gui
