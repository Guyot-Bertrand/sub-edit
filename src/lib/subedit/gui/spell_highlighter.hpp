#pragma once

// The underline of the unknown words in a text editor — issue #525,
// `GUI-SPELL-04`: Gaupol's inline spell check (`spell_check.inline`).

#include <QSyntaxHighlighter>

#include <memory>

class QString;
class QTextDocument;

namespace subedit::core {
class SpellChecker;
} // namespace subedit::core

namespace subedit::gui {

/// Underlines, in a wavy red, the words `checker` refuses, and follows the
/// typing: a block is looked at again whenever it changes.
///
/// **It keeps the checker alive**, through the shared pointer it is given, so
/// that the window may replace its own when the settings change without
/// asking whether an editor is still open on the old one.
class SpellHighlighter final : public QSyntaxHighlighter {
    // No `Q_OBJECT`: it declares neither signal nor slot.

public:
    /// `document` owns this; a null `checker` underlines nothing.
    SpellHighlighter(QTextDocument* document, std::shared_ptr<const core::SpellChecker> checker);

protected:
    void highlightBlock(const QString& text) override;

private:
    std::shared_ptr<const core::SpellChecker> m_checker;
};

} // namespace subedit::gui
