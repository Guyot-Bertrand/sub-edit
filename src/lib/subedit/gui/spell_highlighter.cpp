#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/spell_ranges.hpp>
#include <subedit/gui/spell_highlighter.hpp>

#include <QByteArray>
#include <QColor>
#include <QString>
#include <QTextCharFormat>
#include <QTextDocument>

#include <cstddef>
#include <string_view>
#include <utility>

namespace subedit::gui {

namespace {

/// The UTF-16 length of the first `bytes` bytes of a UTF-8 `text`: the unit a
/// text edit counts in.
int utf16Length(const QByteArray& text, std::size_t bytes) {
    return static_cast<int>(
        QString::fromUtf8(text.constData(), static_cast<qsizetype>(bytes)).size());
}

} // namespace

SpellHighlighter::SpellHighlighter(QTextDocument* document,
                                   std::shared_ptr<const core::SpellChecker> checker)
    : QSyntaxHighlighter(document), m_checker(std::move(checker)) {}

void SpellHighlighter::highlightBlock(const QString& text) {
    if (m_checker == nullptr || text.isEmpty())
        return;

    QTextCharFormat misspelt;
    misspelt.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    misspelt.setUnderlineColor(Qt::red);

    // The rule is the core's, in bytes of UTF-8; the document counts UTF-16.
    const QByteArray utf8 = text.toUtf8();
    const std::string_view view{utf8.constData(), static_cast<std::size_t>(utf8.size())};
    for (const core::SpellRange& range : core::misspelledSpellRanges(*m_checker, view)) {
        const int start = utf16Length(utf8, range.offset);
        const int end = utf16Length(utf8, range.offset + range.length);
        setFormat(start, end - start, misspelt);
    }
}

} // namespace subedit::gui
