#include <subedit/gui/split_project_dialog.hpp>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>

#include <algorithm>
#include <cstddef>

namespace subedit::gui {

namespace {

/// The first subtitle that may begin a tail, numbered from one.
constexpr int kSmallestNumber = 2;

} // namespace

SplitProjectDialog::SplitProjectDialog(std::size_t count, std::size_t initial, QWidget* parent)
    : QDialog(parent), m_subtitle(new QSpinBox{this}) {
    setWindowTitle(QStringLiteral("Split project"));

    const int largest = static_cast<int>(count);
    m_subtitle->setRange(kSmallestNumber, std::max(kSmallestNumber, largest));
    m_subtitle->setValue(std::clamp(static_cast<int>(initial), kSmallestNumber, largest));

    // One subtitle number is one row of the table, counted from zero.
    connect(m_subtitle, &QSpinBox::valueChanged, this, [this](int number) {
        emit rowChosen(number - 1);
    });

    auto* fields = new QFormLayout;
    fields->addRow(QStringLiteral("Split at subtitle"), m_subtitle);

    auto* buttons = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addWidget(new QLabel{QStringLiteral("The subtitles from this one on move to a new "
                                               "project, opened in a tab of its own."),
                                this});
    stack->addLayout(fields);
    stack->addWidget(buttons);
}

std::size_t SplitProjectDialog::firstOfTail() const {
    // The box starts at two, so the subtraction cannot underflow.
    return static_cast<std::size_t>(m_subtitle->value()) - 1;
}

} // namespace subedit::gui
