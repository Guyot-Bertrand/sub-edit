#pragma once

#include <QToolButton>

#include <span>

class QFrame;
class QListWidget;
class QString;

namespace subedit::core {
struct Diagnostic;
} // namespace subedit::core

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
/// It hides itself when there is nothing to report: a button that said
/// « 0 diagnostics » would say there is something to read.
class DiagnosticsButton final : public QToolButton {
    Q_OBJECT

public:
    explicit DiagnosticsButton(QWidget* parent = nullptr);

    /// Replaces what the button reports, and shows or hides it accordingly.
    /// Anything already open is closed: it would be showing the previous list.
    void setDiagnostics(std::span<const core::Diagnostic> diagnostics);

    [[nodiscard]] int count() const;

    /// The text of one line, for a test to read what a user would.
    [[nodiscard]] QString lineAt(int row) const;

    /// The floating frame the list opens in, for a test to look at.
    [[nodiscard]] QWidget* popup() const;

    /// Opens the list above the button — what a click does.
    void openList();

private:
    QFrame* m_popup;
    QListWidget* m_lines;
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
