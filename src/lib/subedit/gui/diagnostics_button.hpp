#pragma once

#include <QStringList>
#include <QToolButton>

#include <span>
#include <vector>

namespace subedit::core {
struct Anomaly;
struct Diagnostic;
} // namespace subedit::core

class QFrame;
class QListWidget;
class QString;

namespace subedit::gui {

/// What a reading ran into, one click away from the status bar.
///
/// **The first place in the project where diagnostics reach a user** other
/// than through `-vvv`. A reading recovers at best effort — ADR 0008 — and
/// keeping quiet about what it had to decide would make that a silence rather
/// than a policy.
///
/// **A button and not a bar** — issue #605. A diagnostic is rare, belongs to one
/// project and is read once, at opening; a strip under the table, folded or
/// not, took a row of the window for it, and unfolded it took the table's. The
/// button lives in the status bar, among the other things the window says about
/// the document, and the list opens in a floating frame above it: no row of the
/// layout is spent, and the frame goes away at a click elsewhere or on `Esc`.
///
/// **It also lists what is wrong with the subtitles themselves** — an overlap, an
/// end before its start, a start before the one above it. The table tints those
/// rows, but a tint has to be scrolled to, and one is easily missed in a file of
/// two thousand lines; here they are all together, and a click on one goes to its
/// row.
///
/// It hides itself when there is nothing to report: a button that said
/// « 0 diagnostics » would say there is something to read.
class DiagnosticsButton final : public QToolButton {
    Q_OBJECT

public:
    explicit DiagnosticsButton(QWidget* parent = nullptr);

    /// Replaces what the button reports, and shows or hides it accordingly.
    /// Anything already open is closed: it would be showing the previous list.
    void setDiagnostics(std::span<const core::Diagnostic> diagnostics);

    /// Replaces the anomalies of the project the button lists after what the
    /// reading ran into. **Does not close the list**: an edit that repairs one
    /// takes it out of a list the user may be reading, and a list that vanished
    /// at every correction could not be worked through.
    void setAnomalies(std::span<const core::Anomaly> anomalies);

    [[nodiscard]] int count() const;

    /// The text of one line, for a test to read what a user would.
    [[nodiscard]] QString lineAt(int row) const;

    /// The floating frame the list opens in, for a test to look at.
    [[nodiscard]] QWidget* popup() const;

    /// Opens the list above the button — what a click does.
    void openList();

    /// Chooses the line at `row` of the list, as a click does: an anomaly sends
    /// the window to its subtitle, a reading diagnostic goes nowhere.
    void chooseLine(int row);

signals:
    /// The zero-based row of the subtitle an anomaly is about.
    void rowChosen(int row);

private:
    void rebuild();

    QFrame* m_popup;
    QListWidget* m_lines;
    /// What the list says, as lines — the core types stay out of this header,
    /// which `moc` has to parse.
    QStringList m_diagnosticLines;
    QStringList m_anomalyLines;
    /// The subtitle row of each anomaly line, in the same order.
    std::vector<int> m_anomalyRows;
};

/// One diagnostic, as the list writes it: where, what, and what was done.
///
/// ```
/// line 5: a SubRip block without its number, recovered
/// ```
///
/// The detail comes from the file and is therefore **quoted and bounded**:
/// unquoted, a line ending in a comma would read as part of the sentence, and
/// unbounded, one absurd line would push the list off the screen. Neither is
/// ours to trust.
[[nodiscard]] QString lineOf(const core::Diagnostic& diagnostic);

} // namespace subedit::gui
