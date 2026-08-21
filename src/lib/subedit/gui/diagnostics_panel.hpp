#pragma once

#include <QWidget>

#include <span>

class QListWidget;
class QToolButton;
class QString;

namespace subedit::core {
struct Diagnostic;
} // namespace subedit::core

namespace subedit::gui {

/// What a reading ran into, folded away until someone wants it.
///
/// **The first place in the project where diagnostics reach a user** other
/// than through `-vvv`. A reading recovers at best effort — ADR 0008 — and
/// keeping quiet about what it had to decide would make that a silence rather
/// than a policy.
///
/// It hides itself when there is nothing to report: an empty panel would say
/// there is something to read.
class DiagnosticsPanel final : public QWidget {
    Q_OBJECT

public:
    explicit DiagnosticsPanel(QWidget* parent = nullptr);

    /// Replaces what the panel shows, and shows or hides it accordingly.
    void setDiagnostics(std::span<const core::Diagnostic> diagnostics);

    [[nodiscard]] int count() const;

    /// The text of one line, for a test to read what a user would.
    [[nodiscard]] QString lineAt(int row) const;

private:
    QToolButton* m_toggle;
    QListWidget* m_lines;
};

/// One diagnostic, as the panel writes it: where, what, and what was done.
///
/// ```
/// line 5: a SubRip block without its number, recovered
/// ```
///
/// The detail comes from the file and is therefore **quoted and bounded**:
/// unquoted, a line ending in a comma would read as part of the sentence, and
/// unbounded, one absurd line would push the panel off the screen. Neither is
/// ours to trust.
[[nodiscard]] QString lineOf(const core::Diagnostic& diagnostic);

} // namespace subedit::gui
