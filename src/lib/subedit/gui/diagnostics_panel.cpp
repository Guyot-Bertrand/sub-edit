#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/diagnostics_panel.hpp>

#include <QListWidget>
#include <QString>
#include <QToolButton>
#include <QVBoxLayout>

#include <cstddef>
#include <span>
#include <string>

namespace subedit::gui {

namespace {

/// How much of a detail is worth reading before it stops being context.
constexpr int kLongestDetail = 80;

[[nodiscard]] QString boundedOf(const std::string& detail) {
    const QString text = QString::fromStdString(detail);
    return text.size() <= kLongestDetail ? text : text.left(kLongestDetail) + QStringLiteral("…");
}

} // namespace

QString lineOf(const core::Diagnostic& diagnostic) {
    QString line = QStringLiteral("line %1: %2")
                       .arg(diagnostic.line)
                       .arg(QString::fromUtf8(core::nameOf(diagnostic.kind)));

    if (!diagnostic.detail.empty())
        line += QStringLiteral(" (\"%1\")").arg(boundedOf(diagnostic.detail));

    return line + QStringLiteral(", ") + QString::fromUtf8(core::nameOf(diagnostic.severity));
}

DiagnosticsPanel::DiagnosticsPanel(QWidget* parent)
    : QWidget(parent), m_toggle(new QToolButton{this}), m_lines(new QListWidget{this}) {
    m_toggle->setCheckable(true);
    m_toggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toggle->setArrowType(Qt::RightArrow);

    // Repliée au départ : ce qu'une lecture a rattrapé mérite d'être
    // consultable, pas de s'imposer entre l'utilisateur et sa table.
    m_lines->setVisible(false);
    connect(m_toggle, &QToolButton::toggled, this, [this](bool open) {
        m_lines->setVisible(open);
        m_toggle->setArrowType(open ? Qt::DownArrow : Qt::RightArrow);
    });

    auto* stack = new QVBoxLayout{this};
    stack->setContentsMargins(0, 0, 0, 0);
    stack->addWidget(m_toggle);
    stack->addWidget(m_lines);

    setVisible(false);
}

void DiagnosticsPanel::setDiagnostics(std::span<const core::Diagnostic> diagnostics) {
    m_lines->clear();
    for (const core::Diagnostic& diagnostic : diagnostics)
        m_lines->addItem(lineOf(diagnostic));

    m_toggle->setText(diagnostics.size() == 1
                          ? QStringLiteral("1 diagnostic while reading")
                          : QStringLiteral("%1 diagnostics while reading").arg(diagnostics.size()));

    setVisible(!diagnostics.empty());
}

int DiagnosticsPanel::count() const {
    return m_lines->count();
}

QString DiagnosticsPanel::lineAt(int row) const {
    const QListWidgetItem* item = m_lines->item(row);
    return item == nullptr ? QString{} : item->text();
}

} // namespace subedit::gui
