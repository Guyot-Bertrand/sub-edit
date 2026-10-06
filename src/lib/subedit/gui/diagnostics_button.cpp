#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/gui/diagnostics_button.hpp>

#include <QAbstractItemView>
#include <QFrame>
#include <QListWidget>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QString>
#include <QStyle>
#include <QVBoxLayout>

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>

namespace subedit::gui {

namespace {

/// How much of a detail is worth reading before it stops being context.
constexpr int kLongestDetail = 80;

/// The list: wide enough for a line and its excerpt, and never taller than this.
constexpr int kListWidth = 460;
constexpr int kListHeight = 240;
constexpr int kMostRows = 10;

[[nodiscard]] QString boundedOf(const std::string& detail) {
    const QString text = QString::fromStdString(detail);
    return text.size() <= kLongestDetail ? text : text.left(kLongestDetail) + QStringLiteral("…");
}

} // namespace

QString lineOf(const core::Diagnostic& diagnostic) {
    // A diagnostic about the whole file has no line to name, and "line 0"
    // would name a place that is not there — see `kWholeFile`.
    QString line = diagnostic.line == core::kWholeFile
                       ? QString::fromUtf8(core::nameOf(diagnostic.kind))
                       : QStringLiteral("line %1: %2")
                             .arg(diagnostic.line)
                             .arg(QString::fromUtf8(core::nameOf(diagnostic.kind)));

    if (!diagnostic.detail.empty())
        line += QStringLiteral(" (\"%1\")").arg(boundedOf(diagnostic.detail));

    return line + QStringLiteral(", ") + QString::fromUtf8(core::nameOf(diagnostic.severity));
}

DiagnosticsButton::DiagnosticsButton(QWidget* parent)
    : QToolButton(parent),
      // A top-level frame of the `Popup` kind: it takes the focus while it is open,
      // goes at a click elsewhere or on `Esc`, and is no row of the layout.
      m_popup(new QFrame{this, Qt::Popup}),
      m_lines(new QListWidget{m_popup}) {
    setAutoRaise(true);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
    setToolTip(QStringLiteral("What the reading ran into — click for the list"));

    m_popup->setFrameShape(QFrame::StyledPanel);
    auto* stack = new QVBoxLayout{m_popup};
    stack->setContentsMargins(2, 2, 2, 2);
    stack->addWidget(m_lines);
    m_lines->setMinimumWidth(kListWidth);
    m_lines->setMaximumHeight(kListHeight);
    m_lines->setSelectionMode(QAbstractItemView::NoSelection);
    m_lines->setTextElideMode(Qt::ElideRight);

    connect(this, &QToolButton::clicked, this, &DiagnosticsButton::openList);

    setVisible(false);
}

void DiagnosticsButton::setDiagnostics(std::span<const core::Diagnostic> diagnostics) {
    m_popup->hide();
    m_lines->clear();
    for (const core::Diagnostic& diagnostic : diagnostics)
        m_lines->addItem(lineOf(diagnostic));

    setText(diagnostics.size() == 1 ? QStringLiteral("1 diagnostic")
                                    : QStringLiteral("%1 diagnostics").arg(diagnostics.size()));

    setVisible(!diagnostics.empty());
}

void DiagnosticsButton::openList() {
    // Tall enough for the lines, short of the bound: a list of three lines is not
    // a window of nine.
    const int rows = std::min(m_lines->count(), kMostRows);
    const int rowHeight = m_lines->sizeHintForRow(0) > 0 ? m_lines->sizeHintForRow(0) : 20;
    m_lines->setFixedHeight(
        std::min(kListHeight, (rows * rowHeight) + (2 * m_lines->frameWidth()) + 4));
    m_popup->adjustSize();

    // Above the button, its left edge on the button's — and kept on the screen,
    // which a button at the right of a status bar would otherwise push it off.
    QPoint at = mapToGlobal(QPoint{0, 0}) - QPoint{0, m_popup->height()};
    if (const QScreen* screen = window()->screen(); screen != nullptr) {
        const QRect available = screen->availableGeometry();
        at.setX(std::clamp(at.x(), available.left(), available.right() - m_popup->width()));
        at.setY(std::max(at.y(), available.top()));
    }
    m_popup->move(at);
    m_popup->show();
}

QWidget* DiagnosticsButton::popup() const {
    return m_popup;
}

int DiagnosticsButton::count() const {
    return m_lines->count();
}

QString DiagnosticsButton::lineAt(int row) const {
    const QListWidgetItem* item = m_lines->item(row);
    return item == nullptr ? QString{} : item->text();
}

} // namespace subedit::gui
