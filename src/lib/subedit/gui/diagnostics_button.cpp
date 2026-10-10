#include <subedit/core/analysis/anomaly.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/gui/diagnostics_button.hpp>

#include <QAbstractItemView>
#include <QFrame>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QString>
#include <QStyle>
#include <QVBoxLayout>

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace subedit::gui {

namespace {

/// The list: wide enough for a line and its excerpt, and never taller than this.
constexpr int kListWidth = 460;
constexpr int kListHeight = 240;
constexpr int kMostRows = 10;

/// Where an anomaly's line keeps the row of its subtitle; a reading diagnostic
/// has none.
constexpr int kRowRole = Qt::UserRole;

} // namespace

QStringList summaryLinesOf(std::span<const core::Diagnostic> diagnostics) {
    struct Group {
        core::DiagnosticKind kind;
        core::Severity severity;
        int count;
    };

    // In the order each pair first appears: the reading met them in that order.
    std::vector<Group> groups;
    for (const core::Diagnostic& diagnostic : diagnostics) {
        const auto same = [&diagnostic](const Group& group) {
            return group.kind == diagnostic.kind && group.severity == diagnostic.severity;
        };
        if (auto found = std::ranges::find_if(groups, same); found != groups.end())
            ++found->count;
        else
            groups.push_back(
                Group{.kind = diagnostic.kind, .severity = diagnostic.severity, .count = 1});
    }

    QStringList lines;
    for (const Group& group : groups) {
        lines.push_back(QStringLiteral("%1, %2: %3")
                            .arg(QString::fromUtf8(core::nameOf(group.kind)),
                                 QString::fromUtf8(core::nameOf(group.severity)))
                            .arg(group.count));
    }
    return lines;
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
    setToolTip(QStringLiteral("What the reading ran into, and what is wrong with the "
                              "subtitles — click for the list"));

    m_popup->setFrameShape(QFrame::StyledPanel);
    auto* stack = new QVBoxLayout{m_popup};
    stack->setContentsMargins(2, 2, 2, 2);
    stack->addWidget(m_lines);
    m_lines->setMinimumWidth(kListWidth);
    m_lines->setMaximumHeight(kListHeight);
    m_lines->setSelectionMode(QAbstractItemView::NoSelection);
    m_lines->setTextElideMode(Qt::ElideRight);

    connect(this, &QToolButton::clicked, this, &DiagnosticsButton::openList);
    connect(m_lines, &QListWidget::itemClicked, this, [this](const QListWidgetItem* item) {
        chooseLine(m_lines->row(item));
    });

    setVisible(false);
}

void DiagnosticsButton::setDiagnostics(std::span<const core::Diagnostic> diagnostics) {
    m_popup->hide();
    m_diagnosticLines = summaryLinesOf(diagnostics);
    rebuild();
}

void DiagnosticsButton::setAnomalies(std::span<const core::Anomaly> anomalies) {
    QStringList lines;
    std::vector<int> rows;
    for (const core::AnomalyCount& count : core::countAnomalies(anomalies)) {
        lines.push_back(QString::fromStdString(core::summaryOf(count.kind, count.count)));
        rows.push_back(static_cast<int>(count.first.value()));
    }

    // Asked after every operation, and almost always for the same answer: a list
    // rebuilt for nothing would lose the scroll position of the one being read.
    if (lines == m_anomalyLines && rows == m_anomalyRows)
        return;

    m_anomalyLines = std::move(lines);
    m_anomalyRows = std::move(rows);
    rebuild();
}

void DiagnosticsButton::rebuild() {
    m_lines->clear();
    m_lines->addItems(m_diagnosticLines);

    for (qsizetype position = 0; position < m_anomalyLines.size(); ++position) {
        auto* item = new QListWidgetItem{m_anomalyLines.at(position)};
        item->setData(kRowRole, m_anomalyRows.at(static_cast<std::size_t>(position)));
        m_lines->addItem(item);
    }

    const int total = m_lines->count();
    setText(total == 1 ? QStringLiteral("1 diagnostic")
                       : QStringLiteral("%1 diagnostics").arg(total));

    // A list with nothing left to show goes with its button.
    if (total == 0)
        m_popup->hide();
    setVisible(total != 0);
}

void DiagnosticsButton::chooseLine(int row) {
    const QListWidgetItem* item = m_lines->item(row);
    if (item == nullptr || !item->data(kRowRole).isValid())
        return;

    emit rowChosen(item->data(kRowRole).toInt());
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
